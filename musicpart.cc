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

using namespace std;

string trim(string x) {
	std::string::size_type pos = x.find_first_not_of(" \r\n\t");
	if (pos != std::string::npos) x.erase(0,pos);
	pos = x.find_last_not_of(" \r\n\t");
	if (pos != std::string::npos) x.erase(pos+1);
	return x;
}

class MusicPart: public Part, InputListener {
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
			int x = pls[i].rfind("/");
			strcpy(buff,pls[i].c_str() + x + 1);
			buff[pls[i].rfind(".") -x -1] = '\0';
			return buff;
		}
		virtual const char * icon(size_t i) {return "media.png";}
	};

	ListListHook llhook;
	ListHook lhook;

	void play(int n) {
		char buff[2048];
		player->stop();
		sprintf(buff, "%s/%s", cfg()("music_root","~/Music"), lhook.pls[n].c_str());
		player->play(buff);
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
		const char * k2[]={llhook.pls[listList->getIndex()].c_str(), NULL};
		const char * v2[]={lhook.pls[playing].c_str(), lhook.pls[list->getIndex()].c_str(), NULL};
		db->update("music_playlist_index", k2, v2);
	}
	
	void setLIndex(int idx,bool update) {
		list->setIndex(idx, update);
		storeFoo();
	}

	void setLLIndex(int idx,bool update) {
		listList->setIndex(idx, update);
		idx = listList->getIndex();

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
			for(int _=0; _ < lhook.size(); ++_) {
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
		void run() {while(true) {mp->input->triggerUser(1, NULL);sleep(1);}};
	};

	ProgressThread pt;
   
	class WaitThread: public Thread<WaitThread> {
	public:
		MusicPart * mp;
		WaitThread(MusicPart * m): mp(m) {}
		void run() {while(true) {mp->player->wait();}};
	};

	WaitThread wt;

	MusicPart(Stack * s, InputStack * i, DB * d)
		: stack(s), input(i), db(d), rightHasFocus(false), llhook(this), lhook(this), pt(this), wt(this) {
		card = stack->constructCard();
		createBox( Rect(0.06, 0.06, 0.94, 0.23 ) );
		listList = createListBox(&llhook, card, 0, Rect(0.06, 0.24, 0.40, 0.86 ), 15 );
		listList->setIndex(0,true);
		createBox( Rect(0.06, 0.87, 0.40, 0.94 ) );
		list = createListBox(&lhook, card, 0, Rect(0.41, 0.24, 0.94, 0.94 ), 20);

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

		player = constructMPlayer();

		const char * k[] = {NULL};
		char buff[1024];
		char * v[] = {buff, NULL};
		int idx=0;
		if(db->fetch("music_playlist", k, v))
			for(int i=0; i < llhook.size(); ++i)
				if(llhook.pls[i] == buff) idx=i;
		setLLIndex(idx, true);
		rightHasFocus=true;
		listList->setFocus(false);
		play(playing);
		pt.start();
	}

	virtual void push() {
		stack->pushCard(card);
		input->pushListener(this);
	}

	virtual void pop() {
		stack->popCard();
		input->popListener();
	}

   	bool onUser(int code, void * data) {
		if(code != 1) return false;
		stack->lockLayout();
		Rect r = progressBase->rect();
		double x = player->getLength();
		double y =  player->getPos();
		if(x != 0) x = y / x;
		Rect r2 = Rect(r.l, r.t, r.l + (r.r-r.l)*x, r.b);
		progressBar->resize(r2);					
		stack->unlockLayout();
		return true;
	}

	bool onSpecialKey(int key) {
		if(!rightHasFocus) {
			switch(key) {
			case up:
				stack->lockLayout();
				setLLIndex( listList->getIndex() - 1, false);
				stack->unlockLayout();
				return true;
			case down: 
				stack->lockLayout();
				setLLIndex( listList->getIndex() + 1, false);
				stack->unlockLayout();
				return true;
			case right:
			case enter:
				rightHasFocus=true;
				stack->lockLayout();
				listList->setFocus(false);
				list->setFocus(true);
				stack->unlockLayout();
			}
		} else {
			switch(key) {
			case up:
				stack->lockLayout();
				setLIndex( list->getIndex() - 1, false);
				stack->unlockLayout();
				return true;
			case down: 
				stack->lockLayout();
				setLIndex( list->getIndex() + 1, false);
				stack->unlockLayout();
				return true;
			case left: 
				rightHasFocus=false;
				stack->lockLayout();
				listList->setFocus(true);
				list->setFocus(false);
				stack->unlockLayout();
				return true;
			case right:
			case enter:
				play( list->getIndex() );
				return true;
			}

		}
	}

	virtual const char * name() {return "Music";}
	virtual const char * image() {return "music.jpg";}
	
	~MusicPart() {
		player->stop();
		delete player;
	}
};

Part * createMusicPart(Stack * s, InputStack * i, DB * d) {
	return new MusicPart(s,i,d);
}
