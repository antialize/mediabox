// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet 
/*
 * Mediabox
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

