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
#include "listbox.hh"
#include <cstring>
#include <map>
#include <algorithm>
using namespace std;

class MyListBox: public ListBox {
public:
	ListBoxHook * hook;
	Card * card;
	int z;
	Rect rect;
	uint32_t lines_;
	
	int idx;
	int top;
	size_t size;
	double fs;

	Fill * fill;
	Fill * mark;

	struct Entry {
		Image * icon;
		Label * label;
		bool mark;
		Entry(): icon(NULL), label(NULL) {};
		Entry(Image * i, Label * l): icon(i), label(l), mark(false) {};
		void remove() {
			if(icon) delete icon; 
			if(label) delete label;
		}
	};
	
	MyListBox(ListBoxHook * h, Card * c, uint32_t zi, Rect r, uint32_t l)
		: hook(h), card(c), z(zi), rect(r), lines_(l) {
		fs = (r.b - r.t - 0.01)/(double)lines_;
		top = 0;
		idx = 0;

		fill = card->addFill(Color(0,0,0,0), z, r);
		fill->setGradient(
			Color(20,20,255,200), Color(20,20,255,100),
			Color(20,20,255,50), Color(20,20,255,100) );
		fill->setRadius(0.02);

		mark = card->addFill(Color(0,0,0,0), z+1, Rect(r.l, r.t, r.r, r.t + fs + 0.01));
		mark->setRadius(0.02);
		mark->setGradient(
			Color(255,0,0,255) , Color(255,0,0,100),
			Color(100,0,0,255) , Color(100,0,0,255) );
	}

	void setFocus(bool focus) {
		if(focus)
			mark->setGradient(
				Color(255,0,0,255) , Color(255,0,0,100),
				Color(100,0,0,255) , Color(100,0,0,255) );
		else
			mark->setGradient(
				Color(255,0,0,51) , Color(255,0,0,20),
				Color(100,0,0,51) , Color(100,0,0,51) );
	}

	map<size_t, Entry> entries;
	void removeUnmarked() {
		for(map<size_t, Entry>::iterator i=entries.begin(); i != entries.end();) {
			map<size_t, Entry>::iterator next = i;
			++next;
			if(i->second.mark) i->second.mark=false;
			else {
				i->second.remove();
				entries.erase(i);
			}
			i=next;
		}
	}
	
	int getIndex() {return idx;}
	size_t lines() {return lines_;}

	void renew(size_t i) {
		if(entries.count(i) == 0) return;
		entries[i].label->setValue(hook->name(i));
		entries[i].icon->change(hook->icon(i));
	}

	void setIndex(int x, bool update) {
		idx=x;
		if(update) {size=hook->size(); removeUnmarked();}
		if(idx >= (int)size) idx=size-1;
		if(idx < 0) idx=0;
		if(idx <= top) top = idx-1;
		if(idx >= (int)lines_+top-1) top=idx-lines_+2;
		if(top + lines_ > size) top = size - lines_;
		if(top < 0) top=0;
		
		float s=rect.t;
		float sp=fs;
		mark->move(rect.l,rect.t+sp*(idx-top));
		for(int i=top; i < min((int)size, (int)(top+lines_)); ++i) {
			if(entries.count(i) == 0) {
				entries[i] = Entry(
					card->addImage(hook->icon(i), z+2, Rect(rect.l+0.005,s+0.005,rect.l+0.015+fs,s+fs+0.005),true),
					card->addLabel(hook->name(i), z+2, rect.l+0.015+fs  , s, fs)
					);
				entries[i].label->setMaxWidth(rect.w()-fs-0.015);
			} else {
				entries[i].icon->move(rect.l+0.005,s+0.005);
				entries[i].label->move(rect.l+0.015+fs, s);
			}
			entries[i].mark=true;
			s += sp;
		}
		removeUnmarked();
	}
};

ListBox * createListBox(ListBoxHook * h, Card * c, uint32_t z, Rect r, uint32_t l) {
	return new MyListBox(h,c,z,r,l);
}
