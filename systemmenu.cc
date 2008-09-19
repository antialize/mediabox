#include "systemmenu.hh"
#include <vector>
#include "config.hh"

using namespace std;

class MySystemMenu: public SystemMenu, InputListener {
public:
	enum State {
		showing_part,
		showing_partsMenu,
		showing_killMenu,
	};


	Stack * stack;
	InputStack * input;
	DB * db;
	vector<Part*> parts;
	Fill * marker;
	void addPart(Part * p) {parts.push_back(p);}
	MySystemMenu(Stack *s, InputStack * i, DB * d): stack(s), input(i), db(d) {
		const char * k[]={NULL};
		const char * v[]={"part",NULL};
		db->describeTabel("part",k,v);
	};
	
	Card * bg;

	Card * partsMenu;

	Card * killMenu;
	State state;
	Fill * killSwitch;
	bool killSwitchOn;

	int part;
	int oldpart;

	bool onSpecialKey(int key) {
		switch(state) {
		case showing_killMenu:
			switch(key) {
			case enter:
				if(killSwitchOn) {
					input->terminate(); return true;
				}
			case escape:
				stack->lockLayout();
				stack->popCard();
				input->popListener();
				stack->unlockLayout();
				state = showing_partsMenu;
				return true;
			case left:
			case right:
			case up:
			case down:
				killSwitchOn = !killSwitchOn;
				stack->lockLayout();
				if(killSwitchOn) 
					killSwitch->move(0.32, 0.53);
				else
					killSwitch->move(0.51, 0.53);
				stack->unlockLayout();
				
			default:
				return true;
			}
		case showing_partsMenu:
			switch(key) {
			case enter:
				if((size_t)part == parts.size()) {
					stack->lockLayout();
					stack->pushCard(killMenu);
					input->pushListener(this);
					killSwitchOn = true;
					killSwitch->move(0.32, 0.53);
					stack->unlockLayout();
					state = showing_killMenu;
				} else {
					stack->lockLayout();
					stack->popCard();
					input->popListener();
					state = showing_part;
					if(part != oldpart) {
						parts[oldpart]->pop();
						parts[part]->push();
						oldpart=part;

						const char * k[]={NULL};
						const char * v[]={parts[part]->name(),NULL};
						db->update("part",k,v);
					}
					stack->unlockLayout();
				}
				return true;
			case left:
			case up:
				--part;
				if(part < 0) part =0;
				stack->lockLayout();
				marker->move(0.25,0.245+0.07*part);
				stack->unlockLayout();
				return true;
			case right:
			case down:
				++part;
				if((size_t)part > parts.size()) part = parts.size();
				stack->lockLayout();
				marker->move(0.25,0.245+0.07*part);
				stack->unlockLayout();
				return true;
			case escape:
				stack->lockLayout();
				stack->popCard();
				input->popListener();
				stack->unlockLayout();
				state = showing_part;
				return true;
			default:
				return true;
			}
		case showing_part: 
			switch(key) {
			case escape:
				stack->lockLayout();
				stack->pushCard(partsMenu);
				input->pushListener(this);
				stack->unlockLayout();
				state = showing_partsMenu;
				return true;
			default:
				return false;
			}
		default:
			return false;
		}
	}

	void run() {
		oldpart = 0;
		char buff[1234];
		const char * k[]={NULL};
		char * v[]={buff,NULL};
		if(db->fetch("part",k,v)) {
			for(size_t i=0; i < parts.size(); ++i)
				if(!strcmp(parts[i]->name(),buff)) oldpart=i;
		}
		part = oldpart;

		bg = stack->constructCard();
		bg->addImage(cfg()("background","stOrmblue-scaled.jpg"),0,Rect(0,0,1,1),false);

		partsMenu = stack->constructCard();
		Fill * f2 = partsMenu->addFill( Color(255,0,0), 0 , 
									  Rect(0.2, 0.2, 0.8, 0.8) );
		f2->setGradient( 
			Color(0,0,190,255),
			Color(0,0,170,230),
			Color(0,0,170,230),
			Color(0,0,150,210) );
		f2->setRadius(0.02);
		
		marker = partsMenu->addFill( Color(255,0,0), 1 ,
									 Rect(0.25, 0.245, 0.75, 0.245 + 0.07 ) );
		
		marker->setGradient( 
			Color(190,0,0,255),
			Color(170,0,0,230),
			Color(170,0,0,230),
			Color(150,0,0,210) );
		marker->setRadius(0.02);
		marker->move(0.25,0.245+0.07*part);
		
		for(size_t i=0; i < parts.size(); ++i)
			partsMenu->addLabel( parts[i]->name() , 2 , 0.3, 0.25 + 0.07*i, 0.06);
		partsMenu->addLabel( "Shutdown" , 2 , 0.3, 0.25 + 0.07*(parts.size()), 0.06);

		killMenu = stack->constructCard();
		Fill * f = killMenu->addFill( Color(255,0,0), 0 , 
									  Rect(0.2, 0.35, 0.8, 0.65) );
		f->setGradient( 
			Color(190,0,0,255),
			Color(170,0,0,230),
			Color(170,0,0,230),
			Color(150,0,0,210) );
		f->setRadius(0.02);
		killMenu->addLabel("Shutdown MediaBox?",1, 0.25, 0.40 , 0.06);
		killMenu->addLabel("Yes",1, 0.35, 0.53 , 0.06);
		killMenu->addLabel("No",1, 0.55, 0.53 , 0.06);
		
		killSwitch = killMenu->addFill( Color(0,0,255), 0 , 
									  Rect(0.32, 0.53, 0.455, 0.595) );
		killSwitch->setGradient( 
			Color(0,0,255,255),
			Color(0,0,230,200),
			Color(0,0,230,200),
			Color(0,0,200,190) );
		killSwitch->setRadius(0.02);
		
		stack->pushCard(bg);
		input->pushListener(this);
		stack->unlockLayout();
		
		state = showing_part;
		parts[part]->push();
		input->mainLoop();
	};
};

SystemMenu * createSystemMenu(Stack *s, InputStack * i, DB * d) {
	return new MySystemMenu(s,i,d);
};
