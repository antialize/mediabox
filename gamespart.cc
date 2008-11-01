#include "sizemagic.hh"
#include "gamespart.hh"
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

class GamesPart: public virtual Part {
public:
	class Hook: public BrowserHook {
	public:
		InputStack * input;
		virtual bool include(const std::string & path) {
			string::size_type x = path.rfind('.',extremum<string::size_type>::max());
			if(x == extremum<string::size_type>::max()) return false;
			string ext = path.substr(x+1);
			return ext == "zip" || ext == "smc";
		}
		const char * name() {return "games";}
		virtual void execute(const std::string & path) {
			int p = fork();
			if(p == -1) DIE("fork");
			if(p == 0) {
				const char * argv[] = {"zsnes", "-m", path.c_str(), NULL};
				execvp("zsnes", (char **)argv);
				exit(2);
			}
		};
		const char * defaultDir() {return cfg()("games_root","/home/");}
	};
	
	Hook hook;
	Browser * browser;
	InputStack * input;
	
	GamesPart(InputStack * i, Stack * s, DB * d) {
		input = hook.input = i;
		browser = constructSTDBrowser(s,&hook,d);
	}

	virtual void push() {
		browser->stack()->pushCard( browser->card() );
		input->pushListener( browser->listener() );
	}

	virtual void pop() {
		browser->stack()->popCard();
		input->popListener();
	}

	virtual const char * name() {return "Games";}
	virtual const char * image() {return "games.png";}
};

Part * createGamesPart(Stack * s, InputStack * i, DB * d) {
	return new GamesPart(i,s,d);
}

