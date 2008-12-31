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
#ifndef __browser_hh__
#define __browser_hh__
#include "canvas.hh"
#include "input.hh"
#include "db.hh"
#include <string>

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
