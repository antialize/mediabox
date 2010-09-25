// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet 
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __browser_hh__
#define __browser_hh__
#include "canvas.hh"
#include "input.hh"
#include "db.hh"
#include <string>
#include <set>
#include <algorithm>

class Browser;

class BrowserHook {
public:
	virtual const char * name() = 0;
	virtual void onExit(Browser * browser) {}
	virtual bool include(const std::string & path) {return true;}
	virtual void execute(const std::string & path) {}
	virtual const char * defaultDir() = 0;
	virtual ~BrowserHook() {};
};

class ExtFilter {
public:
	struct tolower2 {
		int operator() (int i) {return std::tolower(i);}
	};
	std::set<std::string> exts;
	ExtFilter(const std::string & str) {
		size_t s=0;
		while(s < str.size()) {
			size_t n=str.find(',',s);
			if(n == std::string::npos) n = str.size();
			std::string ext = str.substr(s, n-s);
			transform(ext.begin(), ext.end(), ext.begin(), tolower2() );
			exts.insert(ext);
			s=n+1;
		} 
	}
		
	bool operator() (const std::string & path) {
		size_t x = path.rfind('.');
		if(x == std::string::npos) return false;
		std::string ext = path.substr(x+1);
		transform(ext.begin(), ext.end(), ext.begin(), tolower2() );
		return exts.count(ext);
	}
};

class Browser {
public:
	virtual Stack * stack() = 0;
	virtual Card * card() = 0;
	virtual BrowserHook * hook() = 0;
	virtual InputListener * listener() = 0;
	virtual void changeDir(std::string c) = 0;
	virtual ~Browser() {}
};

Browser * constructSTDBrowser(Stack * stack, BrowserHook * hook, DB * db);

#endif //__browser_hh__
