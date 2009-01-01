/*
 * Mediabox: a light mediacenter solution
 * Copyright (C) 2009 Jakob Truelsen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
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

class VideoPart: public virtual Part {
public:
	class Hook: public BrowserHook {
	public: 
		ExtFilter f; 
		InputStack * input;
		virtual bool include(const std::string & path) {return f(path);}
		const char * name() {return "video";}
		virtual void execute(const std::string & path) {
			int p = fork();
			if(p == -1) DIE("fork");
			if(p == 0) {
				const char * argv[] = {"mplayer", "-fs", "-zoom", 
									   "-cache", cfg()("vidio.cache","51200"), 
									   "-cache-min", cfg()("video.min-cache","1"), path.c_str(), NULL};
				execvp("mplayer", (char **)argv);
				exit(2);
			}
		}
		Hook(): f( cfg()("video.extensions","mpg,avi,xvid,mpeg,rm,mkv,wmv,mp4") ) {}
		const char * defaultDir() {return cfg()("video.root","/home/");}
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
		browser->stack()->popCard();
		input->popListener();
	}

	virtual const char * name() {return "Video";}
	virtual const char * image() {return "video.png";}
};

Part * createVideoPart(Stack * s, InputStack * i, DB * d) {
	return new VideoPart(i,s,d);
}

