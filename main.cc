#include "sizemagic.hh"
#include "browser.hh"
#include "canvas.hh"
#include "db.hh"
#include "input.hh"
#include "player.hh"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "config.hh"

struct tolower {
	int operator() (int i) {return std::tolower(i);}
};

class VideoBrowserHook: public BrowserHook {
public:
	InputStack * input;
	virtual void onExit(Browser * b) {
	};
	virtual bool include(const std::string & path) {
		std::string::size_type x = path.rfind('.',extremum<std::string::size_type>::max());
		if(x == extremum<std::string::size_type>::max()) return false;
		std::string ext = path.substr(x+1);
		std::transform(ext.begin(),ext.end(),ext.begin(),tolower);
		return ext == "mpg" || ext == "avi" || ext == "xvid" || ext == "mpeg" || ext == "rm" || ext == "mkv"
			|| ext == "wmv";
	}

	const std::string name() {return "video";}
	virtual void execute(const std::string & path) {
	  int mplayer = fork();
	  if(mplayer == 0) {
	    const char * argv[] = {"mplayer","-fs","-zoom",path.c_str(),NULL};
	    execvp("mplayer", (char **)argv);
	    exit(2);
	  }
	}
	const std::string defaultDir() {return cfg()("video_root","/home/");}
};

int main(int argc, char ** argv) {
	Stack * stack = constructSDLStack();
	stack->lockLayout();
	VideoBrowserHook hook;
	InputStack * input = createSDLInputStack();
	hook.input=input;
	#ifdef __HAVE_CWIID__
	InputHandler * wiimote = createWiimoteInputHandler(input);
	#endif //__HAVE_CWIID__
	
	input->pushListener(stack);
	Card * c = stack->constructCard();
	c->addImage(cfg()("background","stOrmblue-scaled.jpg"),0,Rect(0,0,1,1),false);
	stack->pushCard(c);

	DB * db = NULL;
	#ifdef __HAVE_MYSQL__
	try {
		db = createMysqlDB();
	} catch(bool x) {}
	#endif
	if(db == NULL) db = createMemoryDB();
	Browser * b = constructSTDBrowser(stack,&hook,db);
	stack->unlockLayout();
	b->run(input);

	delete b;

    #ifdef __HAVE_CWIID__
	delete wiimote;
	#endif //__HAVE_CWIID__

	delete input;
	delete stack;
	delete db;
}
