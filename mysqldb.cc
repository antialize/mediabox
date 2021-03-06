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
#include <mysql.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "config.hh"
#include <string.h>
using namespace std;

class MysqlDB: public DB {
public:
	MYSQL mysql;
	map< string, pair<vector<string>,vector<string> > > dbdesc;
	
	MysqlDB() {
		if(!cfg()("mysql.use",false)) throw false;
		mysql_init(&mysql);
		if(!mysql_real_connect(&mysql,
							   cfg()("mysql.host","127.0.0.1"),
							   cfg()("mysql.user","mediabox"),
							   cfg()("mysql.password","password"),
							   cfg()("mysql.db","mediabox"),
							   cfg()("mysql.port",3306)
							   ,NULL,0)) {
			fprintf(stderr, "Failed to connect to database: Error: %s\n",
					mysql_error(&mysql));
			throw false;
		}
	}
	
	virtual ~MysqlDB() {
		mysql_close(&mysql);
	}

	void describeTabel(const char * name, const char ** keys, const char ** values) {
		if(dbdesc.count(name)) return;
		dbdesc[name] = pair<vector<string>,vector<string> >();
		ostringstream s;
		s << "CREATE TABLE `" << name << "` (";
		bool f=true;
		for(;*keys != NULL; keys++) {
			dbdesc[name].first.push_back(*keys);
			if(f) f=false;
			else s <<", ";
			s << "`" << *keys << "` TEXT NOT NULL";
		}
		for(;*values != NULL; values++) {
			dbdesc[name].second.push_back(*values);
			if(f) f=false;
			else s <<", ";
			s << "`" << *values << "` TEXT NOT NULL";
		}
		s << ")";
		mysql_query(&mysql, s.str().c_str());
		/*
		string x=;
		cout << x << endl;
		if(mysql_query(&mysql, x.c_str())) {
			fprintf(stderr, "Query failed: %s\n", mysql_error(&mysql)); 
		}
		*/
	}

	void update(const char * tbl, const char ** keys, const char ** values) {
		ostringstream s1;
		ostringstream s2;
		s1 << "DELETE FROM `" << tbl << "` WHERE 1=1";
		s2 << "INSERT INTO `" << tbl << "` SET ";
		bool first=true;
		for(vector<string>::iterator i= dbdesc[tbl].first.begin();
			i != dbdesc[tbl].first.end(); ++i, ++keys) {
			if(first) first=false;
			else {s2 << ", ";}
			s1 << " AND ";
			char buff[1024];
			mysql_real_escape_string(&mysql, buff, *keys, strlen(*keys));
			s1 << "`" << *i << "`='" << buff << "'";
			s2 << "`" << *i << "`='" << buff << "'";
		}
		for(vector<string>::iterator i= dbdesc[tbl].second.begin();
			i != dbdesc[tbl].second.end(); ++i, ++values) {
			if(first) first=false;
			else {s2 << ", ";}
			char buff[1024];
			mysql_real_escape_string(&mysql, buff, *values, strlen(*values));
			s2 << "`" << *i << "`='" << buff << "'";
		}
		if(mysql_query(&mysql, s1.str().c_str()))
			fprintf(stderr, "Query failed: %s\n%s\n", mysql_error(&mysql), s1.str().c_str()); 
		if(mysql_query(&mysql, s2.str().c_str()))
			fprintf(stderr, "Query failed: %s\n%s\n", mysql_error(&mysql), s2.str().c_str()); 
	}

	bool fetch(const char * tbl, const char ** keys, char ** values) {
		ostringstream s;
		s << "SELECT ";
		bool first=true;
		for(vector<string>::iterator i= dbdesc[tbl].second.begin();
			i != dbdesc[tbl].second.end(); ++i) {
			if(first) first=false;
			else s << ", ";
			s << "`" << *i << "`";
		}			
		s << " FROM `" << tbl << "` WHERE 1=1";
		for(vector<string>::iterator i= dbdesc[tbl].first.begin();
			i != dbdesc[tbl].first.end(); ++i, ++keys) {
			s << " AND ";
			char buff[1024];
			mysql_real_escape_string(&mysql, buff, *keys, strlen(*keys));
			s << "`" << *i << "`='" << buff << "'";
		}
		s << " LIMIT 1";
		if(mysql_query(&mysql, s.str().c_str()))
			fprintf(stderr, "Query failed: %s\n%s\n", mysql_error(&mysql), s.str().c_str()); 
		bool good=false;
		if(mysql_next_result(&mysql)) {
			MYSQL_RES * r = mysql_store_result(&mysql);
			if(r) {
				MYSQL_ROW row = mysql_fetch_row(r);
				if(row) {
					for(int i=0; *values != NULL; ++i, ++values)
						strcpy(*values, row[i]);
					good=true;
				}
			}
			mysql_free_result(r);
		}
		return good;
	}
};

DB * createMysqlDB() {
	return new MysqlDB();
}
