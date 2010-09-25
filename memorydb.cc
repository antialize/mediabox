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
#include "db.hh"
#include <map>
#include <vector>
#include <string>
#include <string.h>
using namespace std;

class MemoryDB: public DB {
public:
	map<string,map<vector<string>,vector<string> > > db;
	void describeTabel(const char * name, const char ** keys, const char ** values) {
		if(!db.count(string(name))) db[string(name)]=map<vector<string>,vector<string> >();
	}
	void update(const char * tbl, const char ** keys, const char ** values) {
		vector<string> k;
		for(;*keys; keys++) k.push_back(string(*keys));
		vector<string> v;
		for(;*values; values++) v.push_back(string(*values));
		db[tbl][k] = v;
	}
	bool fetch(const char * tbl, const char ** keys, char ** values) {
		vector<string> k;
		for(;*keys; keys++) k.push_back(string(*keys));
		if(!db[tbl].count(k)) return false;
		vector<string> & v = db[tbl][k];
		for(vector<string>::iterator i=v.begin(); 
			i != v.end(); ++i, ++values) {
			strcpy(*values,i->c_str());
		}
		return true;
	}

};


DB * createMemoryDB() {return new MemoryDB();}

