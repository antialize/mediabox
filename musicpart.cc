#include "musicpart.hh"
#include "listbox.hh"
#include <vector>
#include "config.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "player.hh"
#include "threading.hh"
#include <algorithm>
using namespace std;

string trim(string x) {
	std::string::size_type pos = x.find_first_not_of(" \r\n\t");
	if (pos != std::string::npos) x.erase(0,pos);
	pos = x.find_last_not_of(" \r\n\t");
	if (pos != std::string::npos) x.erase(pos+1);
	return x;
}

class MusicPart: public Part, InputListener, PlayerHook {
public:
	Stack * stack;
	InputStack * input;
	DB * db;
	Card * card;
	
	ListBox * listList;
	ListBox * list;
	
	Fill * optionsFill;
	Fill * playFill;
	bool rightHasFocus;
	int playing;
	Player * player;
	Fill * progressBase;
	Fill * progressBar;
	Label * nl;
	bool stopped;
	int cnt;
	Fill * black;
	int blackness;
	int b_s;
	int b_d;
	int b_m;

	class ListListHook: public ListBoxHook {
	public:
		MusicPart * mp;
		vector<string> pls;
		
		const char * pldir() {
			return cfg()("music_playlistDir","~/playlists");
		}
		
		ListListHook(MusicPart * m): mp(m) {
			DIR * d = opendir(pldir());
			if(d) {
				while(struct dirent * e = readdir(d)) {
					string x = e->d_name;
					int _ = x.rfind(".");
					if(_ > 0 && x.substr(_) == ".m3u")pls.push_back(x);
				}
			}
			sort(pls.begin(), pls.end());
		}

		virtual size_t size() const {return pls.size();}

		char buff[1024];
		virtual const char * name(size_t i) {
			if(i >= pls.size()) return "";
			strcpy(buff,pls[i].c_str());
			buff[pls[i].rfind(".")] = '\0';
			return buff;
		}
		virtual const char * icon(size_t i) {return "folder.png";}
	};

	class ListHook: public ListBoxHook {
	public:
		MusicPart * mp;
		vector<string> pls;
		
		void loadPlayList(const char * list) {
			pls.clear();
			FILE * f = fopen(list,"rb");
			if(f == NULL) return;
			char buff[2048];
			while(fgets(buff,2040,f)) {
				string x= trim(buff);
				if(x != "") pls.push_back(x);
			}
		}

		ListHook(MusicPart * m): mp(m) {}
		virtual size_t size() const {return pls.size();}

		char buff[1024];
		virtual const char * name(size_t i) {
			if(i>=pls.size()) return "";
			int x = pls[i].rfind("/");
			strcpy(buff,pls[i].c_str() + x + 1);
			buff[pls[i].rfind(".") -x -1] = '\0';
			return buff;
		}
		virtual const char * icon(size_t i) {
			return (mp->playing == (int)i)?"playing.png":"media.png";
		}
	};

	virtual void onFinish2() {
		if(cnt < 5) return;
		if(lhook.pls.size() == 0) return;
		if(!stopped) play( (playing+1) % lhook.pls.size() );
	}
	
	virtual void onFinish() {
		input->triggerUser(2, NULL);
	}
	
	ListListHook llhook;
	ListHook lhook;

	void play(size_t n) {
		if(n >= lhook.pls.size()) return;
		char buff[2048];
		cnt = 0;
		if(playing != (int)n); {
			size_t op = playing;
			playing = n;
			list->renew(op);
			list->renew(playing);
		}
		sprintf(buff, "%s/%s", cfg()("music_root","~/Music"), lhook.pls[n].c_str());
		player->play(buff);
		char * x = rindex(buff, '.');
		if(x != NULL) x[0] = '\0';
		x  = rindex(buff, '/');
		nl->setValue( x==NULL?buff:x+1);
	}

	Fill * createBox(const Rect & r) {
		Fill * f = card->addFill(Color(0,0,0), 0, r);
		f->setGradient(
			Color(20,20,255,200) , Color(20,20,255,100),
			Color(20,20,255,50) , Color(20,20,255,100) );
		f->setRadius(0.02);
		return f;
	}

	void storeFoo() {
		const char * k2[]={
			(listList->getIndex() >= 0 && listList->getIndex() < (int)llhook.pls.size()) ? llhook.pls[listList->getIndex()].c_str() : "", NULL};
		const char * v2[]={
			(playing >= 0 && playing < (int)lhook.pls.size())? lhook.pls[playing].c_str():"", 
			(list->getIndex() >= 0 && list->getIndex() < (int)lhook.pls.size())?lhook.pls[list->getIndex()].c_str():"", NULL};
		db->update("music_playlist_index", k2, v2);
	}
	
	void setLIndex(int idx,bool update) {
		list->setIndex(idx, update);
		storeFoo();
	}

	void setLLIndex(int idx,bool update) {
		listList->setIndex(idx, update);
		idx = listList->getIndex();
		if(idx < 0 || idx >= (int)llhook.pls.size()) return;
		const char * k[] = {NULL};
		char buff[1024];
		sprintf(buff,"%d",idx);
		const char * v[] = {buff, NULL};
		db->update("music_playlist", k, v);
		
		sprintf(buff,"%s/%s", 
				llhook.pldir(), 
				llhook.pls[idx].c_str());
		
		lhook.loadPlayList(buff);
		list->setIndex(0,true);
		
		playing = 0;
		int i=0;
		
		char buff2[1024];
		const char * k2[]={llhook.pls[idx].c_str(),NULL};
		char * v2[]={buff,buff2,NULL};
		if(db->fetch("music_playlist_index", k2, v2)) {
			for(size_t _=0; _ < lhook.size(); ++_) {
				if(lhook.pls[_] == buff) playing=_;
				if(lhook.pls[_] == buff2) i=_;
			}
		}
		setLIndex(i, true);
	}	


	class ProgressThread: public Thread<ProgressThread> {
	public:
		MusicPart * mp;
		ProgressThread(MusicPart * m): mp(m) {}
		void run() {while(!mp->stopped) {mp->input->triggerUser(1, NULL);usleep(500000);}};
	};

	ProgressThread pt;
 
	MusicPart(Stack * s, InputStack * i, DB * d)
		: stack(s), input(i), db(d), rightHasFocus(false), playing(-1), llhook(this), lhook(this), pt(this) {
		stopped = false;

		card = stack->constructCard();
		createBox( Rect(0.06, 0.06, 0.94, 0.23 ) );
		listList = createListBox(&llhook, card, 0, Rect(0.06, 0.24, 0.40, 0.86 ), 15 );
		listList->setIndex(0,true);
		createBox( Rect(0.06, 0.87, 0.40, 0.94 ) );
		list = createListBox(&lhook, card, 0, Rect(0.41, 0.24, 0.94, 0.94 ), 20);

		nl = card->addLabel("",2,0.09,0.08,0.05);
		nl->setMaxWidth(0.90);

		progressBase = card->addFill(Color(0,0,0), 0, Rect(0.08,0.15,0.92,0.21));
		progressBase->setGradient(
			Color(200,200,200,255) , Color(150,150,150,255),
			Color(150,150,150,255) , Color(100,100,100,255) );
		progressBase->setRadius(0.02);

		progressBar = card->addFill(Color(0,0,0), 0, Rect(0.08,0.15,0.92,0.21));
		progressBar->setGradient( 
			Color(255,0,0,255) , Color(255,0,0,100),
			Color(100,0,0,255) , Color(100,0,0,255) );
		progressBar->setRadius(0.02);

		const char * k1[]={NULL};
		const char * k2[]={"playlist",NULL};
		const char * v1[]={"playlist",NULL};
		const char * v2[]={"playing","index",NULL};
		db->describeTabel("music_playlist",k1,v1);
		db->describeTabel("music_playlist_index",k2,v2);

		player = constructMPlayer(this);

		const char * k[] = {NULL};
		char buff[1024];
		char * v[] = {buff, NULL};
		int idx=0;
		if(db->fetch("music_playlist", k, v))
			for(size_t i=0; i < llhook.size(); ++i)
				if(llhook.pls[i] == buff) idx=i;
		setLLIndex(idx, true);
		rightHasFocus=true;
		listList->setFocus(false);

		b_s = 30*2;
		b_d = 40*2;
		b_m = 227;
		blackness = 0;
		black = card->addFill(Color(0,0,0,0) , 10, Rect(0,0,1,1) );
	}

	virtual void push() {
		stopped = false;
		stack->pushCard(card);
		input->pushListener(this);
		play(playing);
		pt.start();
	}

	virtual void pop() {
		stopped=true;
		player->stop();
		stack->popCard();
		input->popListener();
	}

   	bool onUser(int code, void * data) {
		if(code == 2) {
			onFinish2(); return true;
		}
		if(code != 1) return false;
		++cnt;
		if(cnt > 0x00FFFFFF) cnt=0x00FFFFFF;
		stack->lockLayout();
		blackness++;
		if(blackness > b_s) {
			int co = min( ((blackness-b_s-1)*b_m) / b_d, b_m );
			int c = min( ((blackness-b_s)*b_m) / b_d, b_m );
			if( c != co ) black->setColor( Color(0,0,0,c) );
			else blackness--;
		} 

		Rect r = progressBase->rect();
		double y = player->getPos();
		double x = player->getLength();
		if(x != 0) x = y / x;
		if(x > 1) x = 1;
		Rect r2 = Rect(r.l, r.t, r.l + (r.r-r.l)*x, r.b);
		progressBar->resize(r2);
		stack->unlockLayout();
		if(!player->running()) onFinish2();
		return true;
	}

	void next() {};
	void prev() {}

	bool onKey(int key) {
		if(blackness > b_s) {
			stack->lockLayout();
			black->setColor( Color(0,0,0,0) );
			stack->unlockLayout();
		}
		blackness = 0;		
		if(key == ' ')
			prev();
		else if(key == '9') player->decVolume();
		else if(key == '0') player->incVolume();
		else return false;
		return true;
	}


	bool onSpecialKey(int key) {
		stack->lockLayout();
		if(blackness > b_s) black->setColor( Color(0,0,0,0) );
		blackness = 0;
		
		if(key == enter) {
			next();
		} else if(!rightHasFocus) {
			switch(key) {
			case up: setLLIndex( listList->getIndex() - 1, false); break;
			case down: setLLIndex( listList->getIndex() + 1, false); break;
			case pageup: setLLIndex( listList->getIndex() - listList->lines() + 3, false); break;
			case pagedown: setLLIndex( listList->getIndex() + listList->lines() - 3, false); break;
			case right:
			case enter:
				rightHasFocus=true;
				listList->setFocus(false);
				list->setFocus(true);
				break;
			default:
				stack->unlockLayout();		   
				return false;
			}
		} else {
			switch(key) {
			case up: setLIndex( list->getIndex() - 1, false); break;
			case down: setLIndex( list->getIndex() + 1, false); break;
			case pageup: setLIndex( list->getIndex() - list->lines() + 3, false); break;
			case pagedown: setLIndex( list->getIndex() + list->lines() + 3, false); break;
			case left: 
				rightHasFocus=false;
				listList->setFocus(true);
				list->setFocus(false);
				break;
			case right:
			case enter:
				play( list->getIndex() );
				break;
			default:
				stack->unlockLayout();		   
				return false;
			}
		}
		stack->unlockLayout();
		return true;
	}

	virtual const char * name() {return "Music";}
	virtual const char * image() {return "music.png";}
	
	~MusicPart() {
		stopped=true;
		player->stop();
		delete player;
	}
};

Part * createMusicPart(Stack * s, InputStack * i, DB * d) {
	return new MusicPart(s,i,d);
}
