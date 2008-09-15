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
	virtual size_t lines() = 0;
	virtual void setFocus(bool hasFocus) = 0;
	virtual ~ListBox() {};
};

ListBox * createListBox(ListBoxHook * hook, Card * card, uint32_t zindex, Rect r, uint32_t lines);

#endif //__listbox_hh__
