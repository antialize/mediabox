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
#include "config.hh"
#include <map>
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

struct scmp {
	bool operator() (const char * a, const char * b) const {
		return strcmp(a,b) < 0;
	}
};

class RealConfig: public Config {	
private:
	char * data;
	typedef map<const char *, const char *, scmp> m_t;
	m_t values;
public:
	RealConfig() {								
		data = NULL;
		char buff[1024];
		sprintf(buff,"%s/.mediabox",getenv("HOME"));
		FILE * f = fopen(buff,"rb");
		if(!f) return;
		fseek(f,0,SEEK_END);
		size_t s = ftell(f);
		fseek(f,0,SEEK_SET);
		data = (char *)malloc(s+1);
		fread(data,s,1,f);
		fclose(f);
		data[s] = '\0';
		for(size_t i=0; i < s;++i) {
			while(data[i] == '\t' || data[i] == ' ') ++i;
			char * name = data + i;
			while(data[i] != '#' && data[i] != '=' &&
				  data[i] != '\n' && data[i] != ' ' &&
				  data[i] != '\0' && data[i] != '\t') ++i;
			while(data[i] == '\t' || data[i] == ' ') {data[i] = '\0'; ++i;}
			if(data[i] == '=') {
				data[i] = '\0';
				++i;
				int nbl=i;
				while(data[i] == '\t' || data[i] == ' ') {data[i] = '\0'; ++i;}
				char * val=data+i;
				while(data[i] != '\n' && data[i] != '\0' 
					  && data[i] != '#') {
					if(data[i] != '\t' && data[i] != ' ') nbl=i;
					++i; 
				}
				data[nbl+1] = '\0';
				values[name] = val;
			}
			while(data[i] != '\n' && data[i] != '\0') ++i;
		}
	}

	int operator() (const char * name, int def) const {
		m_t::const_iterator i=values.find(name);
		if(i == values.end()) return def;
		return atoi(i->second);
	}

	const char * operator() (const char * name, const char * def) const {
		m_t::const_iterator i=values.find(name);
		if(i == values.end()) return def;
		return i->second;
	}

	bool operator() (const char * name, bool def) const {
		m_t::const_iterator i=values.find(name);
		if(i == values.end()) return def;
		char c = i->second[0];
		return c!='f'&&c!='F'&&c!='0';
	}

	~RealConfig() {
		free(data);
	}
};

static RealConfig * c = NULL;
Config & cfg() {
	if(!c) c = new RealConfig();
	return *c;
}
