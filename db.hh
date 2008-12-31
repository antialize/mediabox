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
#ifndef __db_hh__
#define __db_hh__
#include <exception>

class DB {
public:
	virtual void describeTabel(const char * name, const char ** keys, const char ** values) = 0;
	virtual void update(const char * tbl, const char ** keys, const char ** values) = 0;
	virtual bool fetch(const char * tbl, const char ** keys, char ** values) = 0;
	virtual ~DB() {};
};

DB * createMemoryDB();
#ifdef __HAVE_MYSQL__
DB * createMysqlDB();
#endif
#endif //__db_hh__
