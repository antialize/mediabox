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
#ifndef __config_hh__
#define __config_hh__

class Config {
public:
	virtual ~Config() {};
	virtual bool operator() (const char * name, bool def) const = 0;
	virtual const char * operator() (const char * name, const char *) const = 0;
	virtual int operator() (const char * name, int def) const = 0;
	virtual float operator() (const char * name, float def) const = 0;
};

Config & cfg();

#endif //__config_hh__
