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
#include "sizemagic.hh"
#include "browser.hh"
#include <vector>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "listbox.hh"
#include "config.hh"
#include <cstdio>
using namespace std;

class STDBrowser: public Browser, public InputListener, ListBoxHook {
public:
	Stack * _stack;
	BrowserHook * _hook;
	Card * _card;
	InputStack * i;
	Fill * thumbFill;
	Image * thumb;
	Text * plotBox;

	vector< pair<string, bool> > dirlist; 
	string dir;
	DB * db;
	
	ListBox * lb;

	size_t size() const {return dirlist.size();}

	string _;
	const char * name(size_t i) {
		_ = dirlist[i].first;
		int b = _.rfind("."); if(b > 0) _.resize(b);
		return _.c_str();
	}
	const char * icon(size_t i) {
		return dirlist[i].second?"folder.png":"media.png";
	}
	
	map<string, string> art_map;

	bool fileExists(const string & path) {
		int f = open(path.c_str(), O_RDONLY);
		return (f==-1)?false:(close(f),true);
	}

	string locateArt(string path) {
		map<string, string>::iterator i = art_map.find(path);
		string p=path;
		if(path == "") return "";
		if(i != art_map.end()) return i->second;
		if(p[p.size()-1] == '/') p.append("folder.");
		else {
			size_t b = p.rfind(".");
			if(b > p.rfind("/")) p.resize(b+1);
		}
		if(fileExists(p+"jpg")) return (art_map[path]=p+"jpg");
		if(fileExists(p+"png")) return (art_map[path]=p+"png");
		if(path == "/") return ""; 
		path.resize( path.rfind("/",path.size()-2)+1);
		return (art_map[path]=locateArt(path));
	}

	void setIndex(int i, bool update) {
		lb->setIndex(i, update);
		i = lb->getIndex();

		string path=dir;
		if(path[path.length()-1] != '/') path.push_back('/');
		if(i < (int)size()) {
			path.append(dirlist[i].first);
			if(dirlist[i].second) path.push_back('/');
		}
		thumb->change(locateArt(path).c_str());
		string p=path;
		if(p[p.size()-1] == '/') p.append("description");
		else {
			size_t b = p.rfind(".");
			if(b > p.rfind("/")) p.resize(b+1);
			p.append("plot");
		}
		FILE * f =  fopen(p.c_str(),"r");
		if(f) {
			char buff[1024*128];
			buff[fread(buff,1,1024*128-1,f)] = '\0';
			plotBox->setValue(buff);			
			fclose(f);
		} else {
			plotBox->setValue("");
		}
	
		if(!this->dir.empty()) {
			const char * k[] = {_hook->name(), this->dir.c_str(), NULL};
			char x[10];
			sprintf(x,"%d",i);
			const char * v[] = {x, NULL};
			db->update("browser_directory_index",k,v);
		}
	}

	void changeDir(string dir) {
		if(dir.empty() || dir=="/") dir="/";
		else if(dir[dir.size()-1] == '/') dir.resize(dir.size()-1);

		const char * k[] = {_hook->name(), dir.c_str(), NULL};
		char x[10];
		char * v[] = {x, NULL};
		size_t i = db->fetch("browser_directory_index",k,v)?atoi(x):0;

		const char * k2[] = {_hook->name(), NULL};
		const char * v2[] = {dir.c_str(), NULL};
		db->update("browser_directory",k2,v2);
		
		this->dir = dir;
		dirlist.clear();
		DIR * d = opendir(dir.c_str());
		if(d) {
			while(struct dirent * e = readdir(d)) {
				struct stat s;
				if(e->d_name[0] == '.') continue;
				string path = dir + "/" + e->d_name;
				if( stat(path.c_str(), &s) == 0 && (S_ISDIR(s.st_mode) || _hook->include(path)))
					dirlist.push_back( make_pair(e->d_name, S_ISDIR(s.st_mode)?true:false) );
			}
			closedir(d);
			sort(dirlist.begin(), dirlist.end() );
		}
		setIndex(i,true);
	}

	STDBrowser(Stack * s, BrowserHook * h, DB * d): _stack(s), _hook(h), db(d) {
		art_map[""] = "";
		_card = s->constructCard();
		const char * k1[]={"browser",NULL};
		const char * k2[]={"browser","directory",NULL};
		const char * v1[]={"directory",NULL};
		const char * v2[]={"index",NULL};
		db->describeTabel("browser_directory",k1,v1);
		db->describeTabel("browser_directory_index",k2,v2);
		float margin=cfg()("margin",(float)0.06);
		
		thumbFill = _card->addFill(Color(100,100,100,100),0,Rect(0.58, margin, 1.0-margin, 1.0-margin));
		thumbFill->setGradient(
			Color(20,20,255,200) , Color(20,20,255,100),
			Color(20,20,255,50) , Color(20,20,255,100) );
		thumbFill->setRadius(0.02);
		thumb = _card->addImage("",1, Rect(0.59, 0.01+margin, 0.99-margin, 0.55));
		plotBox = _card->addText("",1, Rect(0.59, 0.56, 1.0-margin, 0.99-margin),cfg()("plotlines",10));
		lb = createListBox(this, _card, 0, Rect(margin, margin, 0.56, 1.0 - margin), cfg()("lines",24));
						
		const char * k3[] = {_hook->name(), NULL};
		char buff[1024];
		char * v3[] = {buff, NULL};
		changeDir(db->fetch("browser_directory",k3,v3)? buff: _hook->defaultDir());
	};

	Card * card() {return _card;}
	Stack * stack() {return _stack;}
	BrowserHook * hook() {return _hook;}
	InputListener * listener() {return this;}

	bool onKey(int k) {
		switch(k) {
		case '9':
			plotBox->setFirstLine( plotBox->getFirstLine() -1 );
			return true;
		case '0':
			plotBox->setFirstLine( plotBox->getFirstLine() +1 );
			return true;
		default:
			return false;
		}
	}

	bool onSpecialKey(int key) {
		switch(key) {
		case up:
			_stack->lockLayout();
			setIndex( ((int)lb->getIndex()-1), false);
			_stack->unlockLayout();
			return true;
		case down:
			_stack->lockLayout();
			setIndex( lb->getIndex()+1, false);
			_stack->unlockLayout();
			return true;
		case left:
		{
			string::size_type x = dir.rfind("/", extremum<string::size_type>::max() );
			if(x == extremum<string::size_type>::max()) return false;
				string d=dir;
			if(x==0) d="/";
			else d.resize(x);
			_stack->lockLayout();
			changeDir(d);
			_stack->unlockLayout();
			return true;
		}
		case enter:
		case right:
		{
			size_t i = lb->getIndex();
			if(i >= size()) return true;
			string path=dir;
			if(path[path.length()-1] != '/') path.push_back('/');
			path.append(dirlist[i].first);
			if(dirlist[i].second) {
				_stack->lockLayout();
				changeDir(path);
				_stack->unlockLayout();
			} else
				_hook->execute(path);
			return true;
		}
		case pageup:
			_stack->lockLayout();
			setIndex( lb->getIndex() - lb->lines() + 3, false);
			_stack->unlockLayout();
			return true;
		case pagedown:
			_stack->lockLayout();
			setIndex( lb->getIndex() + lb->lines() - 3, false );
			_stack->unlockLayout();
			return true;
		default:
			return false;
		}
	}

	~STDBrowser() {}
};

Browser * constructSTDBrowser(Stack * stack, BrowserHook * hook, DB * db) {
	return new STDBrowser(stack,hook, db);
};
