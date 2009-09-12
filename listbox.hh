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
#ifndef __listbox_hh__
#define __listbox_hh__
#include "canvas.hh"
#include <cstring>

class ListBoxHook {
public:
	virtual size_t size() const = 0;
	virtual const char * name(size_t i) = 0;
	virtual const char * icon(size_t i) = 0;
	virtual ~ListBoxHook() {};
};

class ListBox {
public:
	virtual int getIndex() = 0;
	virtual void setIndex(int i, bool update) = 0;
	virtual void renew(size_t i) = 0;
	virtual size_t lines() = 0;
	virtual void setFocus(bool hasFocus) = 0;
	virtual ~ListBox() {};
};

ListBox * createListBox(ListBoxHook * hook, Card * card, uint32_t zindex, Rect r, uint32_t lines);

#endif //__listbox_hh__
