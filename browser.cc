#include "sizemagic.hh"
#include "browser.hh"
#include <vector>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>

class STDBrowser: public Browser, public InputListener {
public:
	struct DirEnt {
		std::string name;
		Label * label;
		bool dir;
		Image * icon;
		DirEnt(std::string n, Label * l, bool d): name(n), label(l), dir(d), icon(NULL) {};
		void free() {
			if(label) {delete label; label=NULL;}
			if(icon) {delete icon; icon=NULL; }
		}
	};

	struct DirEntSort {
		bool operator() (const DirEnt & a, const DirEnt & b) {return a.name < b.name;}
	};

	Stack * _stack;
	BrowserHook * _hook;
	Card * _card;
	InputStack * i;
	Fill * thumbFill;
	Fill * filesFill;
	Fill * mark;
	std::vector< DirEnt > dirlist; 
	int idx;
	int visible;
	int top;
	int oldTop;
	float fs;
	std::string dir;
	DB * db;

	void update() {
		if(idx >= (int)dirlist.size()) idx=dirlist.size()-1;
		if(idx < 0) idx=0;
		if(idx <= top) top = idx-1;
		if(idx >= visible+top-1) top=idx-visible+2;
		if(top + visible > (int)dirlist.size()) top = dirlist.size() - visible;
		if(top < 0) top=0;
		for(int i=oldTop; i < top; ++i) dirlist[i].free();
		float s=0.06;
		float sp=fs;
		mark->move(0.06,0.06+sp*(idx-top));
		for(int i=top; i < std::min((int)dirlist.size(),top+visible); ++i) {
			if(!dirlist[i].icon) dirlist[i].icon = _card->addImage(dirlist[i].dir?(char*)"folder.png":(char*)"media.png",2,Rect(0.055,s,0.06+0.045,s+0.045),true);
			else dirlist[i].icon->move(0.055,s);
			if(!dirlist[i].label) {
				dirlist[i].label = _card->addLabel(dirlist[i].name.c_str(),2,0.1,s,fs);
				dirlist[i].label->setMaxWidth(0.56-0.1);
			} else dirlist[i].label->move(0.1, s);
			s += sp;
		}
		for(int i=top+visible; i < std::min((int)dirlist.size(),oldTop+visible); ++i) dirlist[i].free();
		oldTop=top;
	}

	void changeDir(std::string dir) {
		if(dir.empty() || dir=="/") dir="/";
		else if(dir[dir.size()-1] == '/') 
			dir.resize(dir.size()-1);

		if(!this->dir.empty()) {
			const char * k[] = {_hook->name().c_str(), this->dir.c_str(), NULL};
			char x[10];
			sprintf(x,"%d",idx);
			const char * v[] = {x, NULL};
			db->update("browser_directory_index",k,v);
		}
		const char * k[] = {_hook->name().c_str(), dir.c_str(), NULL};
		char x[10];
		char * v[] = {x, NULL};
		if(db->fetch("browser_directory_index",k,v)) {
			idx = atoi(x);
		} else 
			idx = 0;

		const char * k2[] = {_hook->name().c_str(), NULL};
		const char * v2[] = {dir.c_str(), NULL};
		db->update("browser_directory",k2,v2);

		this->dir = dir;
		for(size_t i =0; i < dirlist.size(); ++i) dirlist[i].free();
		dirlist.clear();
		DIR * d = opendir(dir.c_str());
		if(d) {
			while(struct dirent * e = readdir(d)) {
				struct stat s;
				if(e->d_name[0] == '.') continue;
				std::string path = dir + "/" + e->d_name;
				if( stat(path.c_str(), &s) == 0 && (S_ISDIR(s.st_mode) || _hook->include(path)))
					dirlist.push_back( DirEnt(e->d_name, NULL, S_ISDIR(s.st_mode)?true:false) );
			}
			closedir(d);
			std::sort(dirlist.begin(), dirlist.end(), DirEntSort());
		}
		update();
	}

	STDBrowser(Stack * s, BrowserHook * h, DB * d): _stack(s), _hook(h), oldTop(0), db(d) {
		_card = s->constructCard();
		const char * k1[]={"browser",NULL};
		const char * k2[]={"browser","directory",NULL};
		const char * v1[]={"directory",NULL};
		const char * v2[]={"index",NULL};
		db->describeTabel("browser_directory",k1,v1);
		db->describeTabel("browser_directory_index",k2,v2);

		top=0;
		fs=0.04;
		idx=0;
		thumbFill = _card->addFill(Color(100,100,100,100),0,Rect(0.58, 0.06, 0.94, 0.94));
		thumbFill->setGradient(
			Color(20,20,255,200) , Color(20,20,255,100),
			Color(20,20,255,50) , Color(20,20,255,100) );
		thumbFill->setRadius(0.02);
		
		mark = _card->addFill(Color(0,0,255),1,Rect(0.06, 0.06, 0.56, 0.07+fs));
		mark->setRadius(0.02);
		mark->setGradient(
			Color(255,0,0,255) , Color(255,0,0,100),
			Color(100,0,0,255) , Color(100,0,0,255) );
		
		filesFill = _card->addFill(Color(50,50,255,100),0,Rect(0.06, 0.06, 0.56, 0.94));
		filesFill->setGradient(
			Color(20,20,255,200) , Color(20,20,255,100),
			Color(20,20,255,50) , Color(20,20,255,100) );
		filesFill->setRadius(0.02);
		visible=(int)((1.0-0.12)/fs);
		
		db->describeTabel("browser_directory",k1,v1);
		
		const char * k3[] = {_hook->name().c_str(), NULL};
		char buff[1024];
		char * v3[] = {buff, NULL};
		if(db->fetch("browser_directory",k3,v3))
			changeDir(buff);
		else
			changeDir(_hook->defaultDir());
	};

	Card * card() {return _card;}
	Stack * stack() {return _stack;}
	BrowserHook * hook() {return _hook;}
	InputListener * listener() {return this;}

	bool onSpecialKey(int key) {
		switch(key) {
		case escape:
			i->terminate();
			return true;
		case up:
			_stack->lockLayout();
			--idx;
			update();
			_stack->unlockLayout();
			return true;
		case down:
			_stack->lockLayout();
			++idx;
			update();
			_stack->unlockLayout();
			return true;
		case left:
		{
			std::string::size_type x = dir.rfind("/", extremum<std::string::size_type>::max() );
			if(x == extremum<std::string::size_type>::max()) return false;
				std::string d=dir;
			if(x==0) d="/";
			else d.resize(x);
			_stack->lockLayout();
			changeDir(d);
			_stack->unlockLayout();
			return true;
		}
		case enter:
		case right:
		{
			if(idx < 0 || idx >= (int)dirlist.size()) return true;
			std::string path=dir;
			if(path[path.length()-1] != '/') 
				path.push_back('/');
			path.append(dirlist[idx].name);
			if(dirlist[idx].dir) {
				_stack->lockLayout();
				changeDir(path);
				_stack->unlockLayout();
			} else
				_hook->execute(path);
			return true;
		}
		case pageup:
			_stack->lockLayout();
			idx-=visible-3;
			update();
			_stack->unlockLayout();
			return true;
		case pagedown:
			_stack->lockLayout();
			idx+=visible-3;
			update();
			_stack->unlockLayout();
			return true;
		default:
			return false;
		}
	}

	bool onKey(int unicode) {
		switch(unicode) {
		case 'q':
			i->terminate();
			return true;
		default:
			return false;
		}
	}

	void run(InputStack * _) {
		i=_;
		i->pushListener(listener());
		stack()->pushCard(card());
		i->mainLoop();
	}

	~STDBrowser() {
		const char * k[] = {_hook->name().c_str(), dir.c_str(), NULL};
		char x[10];
		char * v[] = {x, NULL};
		if(db->fetch("browser_directory_index",k,v)) {
			idx = atoi(x);
		} else 
			idx = 0;
	}
};

Browser * constructSTDBrowser(Stack * stack, BrowserHook * hook, DB * db) {
	return new STDBrowser(stack,hook, db);
};
