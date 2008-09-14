#ifndef __browser_hh__
#define __browser_hh__
#include "canvas.hh"
#include "input.hh"
#include "db.hh"
#include <string>

class Browser;

class BrowserHook {
public:
	virtual const std::string name() = 0;
	virtual void onExit(Browser * browser) {}
	virtual bool include(const std::string & path) {return true;}
	virtual void execute(const std::string & path) {}
	virtual const std::string defaultDir() = 0;
	virtual ~BrowserHook() {};
};

class Browser {
public:
	virtual Stack * stack() = 0;
	virtual Card * card() = 0;
	virtual BrowserHook * hook() = 0;
	virtual InputListener * listener() = 0;
	virtual void changeDir(std::string c) = 0;
	virtual ~Browser() {}
};

Browser * constructSTDBrowser(Stack * stack, BrowserHook * hook, DB * db);

#endif //__browser_hh__
