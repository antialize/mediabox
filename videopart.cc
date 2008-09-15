#include "sizemagic.hh"
#include "videopart.hh"
#include "browser.hh"
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include "config.hh"
using namespace std;

#define DIE(msg) {perror(msg); exit(1);}

struct tolower2 {
	int operator() (int i) {return std::tolower(i);}
};

class VideoPart: public virtual Part {
public:
	class Hook: public BrowserHook {
	public: 
		InputStack * input;
		virtual bool include(const std::string & path) {
			string::size_type x = path.rfind('.',extremum<string::size_type>::max());
			if(x == extremum<string::size_type>::max()) return false;
			string ext = path.substr(x+1);
			//transform(ext.begin(),ext.end(),ext.begin(),tolower2);
			return ext == "mpg" || ext == "avi" || ext == "xvid" || ext == "mpeg" || ext == "rm" || ext == "mkv"
				|| ext == "wmv";
		}
		const char * name() {return "video";}
		virtual void execute(const std::string & path) {
			int p = fork();
			if(p == -1) DIE("fork");
			if(p == 0) {
				const char * argv[] = {"mplayer", "-fs", "-zoom", path.c_str(), NULL};
				execvp("mplayer", (char **)argv);
				exit(2);
			}
		}
		const char * defaultDir() {return cfg()("video_root","/home/");}
	};
	
	Hook hook;
	Browser * browser;
	InputStack * input;

	VideoPart(InputStack * i, Stack * s, DB * d) {
		input = hook.input = i;
		browser = constructSTDBrowser(s,&hook, d);
	}

	virtual void push() {
		browser->stack()->pushCard( browser->card() );
		input->pushListener( browser->listener() );
	}

	virtual void pop() {

	}

	virtual const char * name() {return "Video";}
	virtual const char * image() {return "video.jpg";}
};

Part * createVideoPart(Stack * s, InputStack * i, DB * d) {
	return new VideoPart(i,s,d);
}

