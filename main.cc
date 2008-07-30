#include "sizemagic.hh"
#include "player.hh"
#include "canvas.hh"
#include <unistd.h>
#include <stdio.h>
#include "input.hh"
#include "browser.hh"
#include "db.hh"

/*
class Test: public InputListener {
public:
	Stack * stack;
	Card * card;
	Fill * fill;
	Image * image;
	Label * label;
	InputStack * input;
	Test() {
		stack = constructSDLStack();
		stack->lockLayout();
		card = stack->constructCard();
		stack->pushCard(card);
		fill = card->addFill(Color(255,0,0),0,Rect(0.0,0.0,1.0,1.0));
		image = card->addImage("Chiyo.jpg",1,Rect(0.2,0.2,0.8,0.8));
		label = card->addLabel("Hello world",2,0.0,0.3,0.1);
		stack->unlockLayout();
		input = createSDLInputStack();
		input->pushListener(stack);
		input->pushListener(this);
	}
	
	bool onSpecialKey(int key) {
		switch(key) {
		case escape:
			input->terminate();
			return true;
		default:
			return false;
		}
	}

	bool onKey(int unicode) {
		switch(unicode) {
		case 'q':
			input->terminate();
			return true;
		default:
			return false;
		}
	}
	
	void run() {
		float _=0.0;
		float delta=0.0005;
		while(input->poll()) {
			_+=delta;
			if(_ > 1-label->rect().w()) {
				_ = 1-label->rect().w();
				delta = -delta;
			} else if(_ < 0.0) {
				_ = 0;
				delta = -delta;
			}
			label->move(_,0.3);
		}
	}
};
*/

#include <iostream>
#include <cctype>

struct tolower {
	int operator() (int i) {return std::tolower(i);}
};

class VideoBrowserHook: public BrowserHook {
public:
	virtual void onExit(Browser * b) {
	};
	virtual bool include(const std::string & path) {
		std::string::size_type x = path.rfind('.',extremum<std::string::size_type>::max());
		if(x == extremum<std::string::size_type>::max()) return false;
		std::string ext = path.substr(x+1);
		std::transform(ext.begin(),ext.end(),ext.begin(),tolower);
		return ext == "mpg" || ext == "avi" || ext == "xvid" || ext == "mpeg" || ext == "rm" || ext == "mkv"
			|| ext == "wmv";
	}

	const std::string name() {return "video";}
	virtual void execute(const std::string & path) {
		Player * p = constructMPlayer();
		p->play(path.c_str());
		p->wait();
		delete p;
	}
	const std::string defaultDir() {return "/home/";}
};

int main(int argc, char ** argv) {
	Stack * stack = constructSDLStack();
	stack->lockLayout();
	VideoBrowserHook hook;
	InputStack * input = createSDLInputStack();
	input->pushListener(stack);
	Card * c = stack->constructCard();
	c->addImage("stOrmblue-1.1-0.jpg",0,Rect(0,0,1,1),false);
	stack->pushCard(c);
	DB * db = createMysqlDB();
	//DB * db = createMemoryDB();
	Browser * b = constructSTDBrowser(stack,&hook,db);
	stack->unlockLayout();
	b->run(input);
	delete b;
	delete input;
	delete stack;
	delete db;
}
