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
template <typename T> struct inplace_data {
	T * next;
	T * prev;
	inplace_data(): next(NULL), prev(NULL) {};
};

template <typename T> struct inplace_reader {
	inline inplace_data<T> & operator() (T * t) const {return t->inplace_data;}
};

#include <iostream>

template <typename T, 
		  class reader=inplace_reader<T>,
		  class data=inplace_data<T> >
class inplace_list {
private:
	T * first;
	T * last;
	size_t cnt;
public:
	inplace_list(): first(NULL), last(NULL), cnt(0) {};
	
	class iterator {
	private: 
		T * e;
	public:
		iterator(T * _): e(_) {};
		void operator ++() {e = reader()(e).next;};
		void operator --() {e = reader()(e).prev;};
		bool operator ==(const iterator & other) const {return e == other.e;}
		bool operator !=(const iterator & other) const {return e != other.e;}
		T * operator * () {return e;}
	};

	void remove(T*e) {
		reader r;
		if(r(e).prev == NULL && e != first) return;
		if(r(e).next) r(r(e).next).prev = r(e).prev;
		else last=r(e).prev;
		if(r(e).prev) r(r(e).prev).next= r(e).next;
		else first=r(e).next;
		r(e).next = r(e).prev = NULL;
		cnt--;
	}
	void push_back(T*e) {
		reader r;
		r(e).prev = last;
		r(e).next = NULL;
		if(last) r(last).next=e;
		else first=e;
		last=e;
		cnt++;
	}
	void push_front(T*e) {
		reader r;
		r(e).prev = NULL;
		r(e).next = first;
		if(first) r(first).prev=e;
		else last=e;
		first=e;
		cnt;
	}
	void pop_back() {
		reader r;
		T * e = r(last).prev;
		if(e) r(e).next = NULL;
		else first=NULL;
		r(last).next= r(last).prev = NULL;
		last=e;
		cnt--;
	}
	void pop_front() {
		reader r;
		T * e = r(first).next;
		if(e) r(e).prev = NULL;
		else last=NULL;
		r(first).next= r(first).prev = NULL;
		first=e;
		cnt--;
	}
	T * front() {return first;}
	T * back() {return last;}
	iterator begin() {return iterator(first);}
	iterator end() {return iterator(NULL);}
	bool empty() {return cnt==0;}
	size_t size() {return cnt;}
};

