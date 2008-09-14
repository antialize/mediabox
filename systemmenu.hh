#ifndef __systemmenu_hh__
#define __systemmenu_hh__
#include "input.hh"
#include <string>
#include "canvas.hh"
#include "db.hh"

class Part {
public:
	virtual void push() = 0;
	virtual void pop()  = 0;
	virtual ~Part() {};
	virtual std::string name()  = 0;
	virtual std::string image() = 0;
};

class SystemMenu {
public:
	virtual void addPart(Part * p)  = 0;
	virtual ~SystemMenu() {}
	virtual void run() = 0;
};

SystemMenu * createSystemMenu(Stack *, InputStack * input, DB *);

#endif //__systemmenu_hh__
