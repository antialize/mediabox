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
#ifndef __canvas_hh__
#define __canvas_hh__
#include <stdint.h>
#include "input.hh"
#include <utility>

template <typename T> struct ARect {
	T l,t,r,b;
	ARect(T w, T x, T y, T z): l(w),t(x),r(y),b(z) {};
	T w() {return r-l;}
	T h() {return b-t;}
	void moveTo(T x,T y) {
		r = w() + x; b = h() + y; l = x; t = y;
	}
	std::pair<T,T> dim() {return std::pair<T,T>(w(),h());}
	bool operator == (const ARect<T> & o) {return l==o.l && t==o.t && r==o.r && b==o.b;}
	bool operator != (const ARect<T> & o) {return l!=o.l || t!=o.t || r!=o.r || b!=o.b;}
};

struct Color {
	uint8_t r,g,b,a;
	Color(uint8_t x,uint8_t y,uint8_t z,uint8_t _=0xFF): r(x),g(y),b(z),a(_) {};
	bool operator ==(const Color & o) {return r==o.r&&g==o.g&&b==o.g&&a==o.a;}
};

typedef ARect<float> Rect;

class Element {
public:
	virtual ~Element() {};
	virtual void remove() = 0;
	virtual uint32_t zindex() = 0;
	virtual void setzindex(uint32_t zindex) = 0;
	virtual void move(float x,float y) = 0;
	virtual void resize(Rect & r) = 0;
	virtual Rect & rect() = 0;
};

class Image: public virtual Element {
public:
	virtual void change(const char * image) = 0;
};

class Fill: public virtual Element {
public:
	virtual void setColor(const Color & c) = 0;
	virtual void setRadius(float r) = 0;
	virtual void setGradient(const Color & ul, const Color & ur, const Color & br, const Color & bl) = 0;
};

class Label: public virtual Element {
public:
	virtual void setValue(const char * value) = 0;
	virtual void setFontSize(float size) = 0;
	virtual void setColor(const Color & c) = 0;
	virtual void setFont(const char * font) = 0;
	virtual void setMaxWidth(float w=0.0) = 0;
	virtual void center() = 0;
};

class Text: public virtual Element {
public:
	virtual void setValue(const char * value) = 0;
	virtual void setLines(uint32_t lines) = 0;
	virtual void setColor(const Color & c) = 0;
	virtual void setFont(const char * font) = 0;
	virtual void setFirstLine(int firstLine) = 0;
	virtual int getFirstLine() = 0;
};

class Card {
public:
	virtual ~Card() {};
	virtual Image * addImage(const char * image, uint32_t zindex, Rect r, bool keepAspect=true) = 0;
	virtual Fill * addFill(const Color & c, uint32_t zindex, Rect r) = 0;
	virtual Label * addLabel(const char * value, uint32_t zindex, float x, float y, float size) = 0;
	virtual Text * addText(const char * value, uint32_t zindex, Rect r, uint32_t lines) = 0;
};

class Stack: public InputListener {
public:
	virtual ~Stack() {};
	virtual void lockLayout() = 0;
	virtual void unlockLayout() = 0;
	virtual void pushCard(Card * c) = 0;
	virtual void popCard() = 0;
	virtual Card * constructCard() = 0;
};

Stack * constructSDLStack();

#endif // __canvas_hh__
