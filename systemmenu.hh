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
#ifndef __systemmenu_hh__
#define __systemmenu_hh__
#include "input.hh"
#include <string>
#include "canvas.hh"
#include "db.hh"

class Part {
public:
	virtual void push() = 0;
	virtual void pop()  = 0;
	virtual ~Part() {};
	virtual const char * name()  = 0;
	virtual const char * image() = 0;
};

class SystemMenu {
public:
	virtual void addPart(Part * p)  = 0;
	virtual ~SystemMenu() {}
	virtual void run() = 0;
};

SystemMenu * createSystemMenu(Stack *, InputStack * input, DB *);

#endif //__systemmenu_hh__
