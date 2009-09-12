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
#ifndef __input_hh__
#define __input_hh__

class InputListener {
public:
	enum {
		up,
		down,
		left,
		right,
		enter,
		escape,
		pageup,
		pagedown,
		pause
	};
	virtual bool onSpecialKey(int key) {return false;}
	virtual bool onKey(int unicode) {return false;}
	virtual bool onResize(int w, int h) {return false;}
	virtual bool onExpose() {return false;}
	virtual bool onUpdate() {return false;}
	virtual bool onUser(int code, void * data) {return false;}
	virtual ~InputListener() {}
};

class InputHandler {
public:
	virtual ~InputHandler() {};
};

class InputStack {
public:
	virtual void pushListener(InputListener * l) = 0;
	virtual InputListener * popListener() = 0;
	virtual void removeListener(InputListener * l) = 0;
	virtual bool wait() = 0;
	virtual bool poll() = 0;
	virtual void mainLoop() = 0;
	virtual void terminate() = 0;
	virtual ~InputStack() {};
	
	virtual void triggerSpecialKey(int key) = 0;
	virtual void triggerKey(int unicode) = 0;
	virtual void triggerUser(int code, void * data) = 0;
};

InputStack * createSDLInputStack();

#endif //__input_hh__
