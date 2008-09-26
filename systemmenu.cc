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
	
	vector<pair<float,float> > l;
	
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
				marker->move(l[part].first, l[part].second);
				stack->unlockLayout();
				return true;
			case right:
			case down:
				++part;
				if((size_t)part > parts.size()) part = parts.size();
				stack->lockLayout();
				marker->move(l[part].first, l[part].second);
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
		
		int x;
		int y;
		for(x=2; ; ++x) {
			for(y=x-1; y <= x; ++y)
				if(x*y >= parts.size() + 1) goto good;
		} 
	good:
		double dx = (0.8-0.3)/x;
		double dy = (0.8-0.3)/x;

		for(int j=0; j < y; ++j) 
			for(int i=0; i < x; ++i) 
				l.push_back( make_pair(i*dx+0.25, j*dy+0.25) );

			
		for(size_t i=0; i < parts.size(); ++i) {
			partsMenu->addImage( parts[i]->image() , 2 , Rect(l[i].first+0.015,l[i].second+0.015,l[i].first+dx-0.03,l[i].second+dy-0.015-0.06) );
			Label * la = partsMenu->addLabel( parts[i]->name() , 2 , l[i].first, l[i].second + dy - 0.07, 0.06);
			la->center();
			la->setMaxWidth(dx);
		}
		size_t i = parts.size();
		partsMenu->addImage( "exit.png" , 2 , Rect(l[i].first+0.015,l[i].second+0.015,l[i].first+dx-0.03,l[i].second+dy-0.015-0.06) );
		Label * la = partsMenu->addLabel( "Shutdown" , 2 , l[i].first, l[i].second + dy - 0.07, 0.06);
		la->center();
		la->setMaxWidth(dx);
			
		marker = partsMenu->addFill( Color(255,0,0), 1 ,
									 Rect(l[0].first, l[0].second, l[0].first + dx, l[0].second + dy ) );
		
		marker->setGradient( 
			Color(190,0,0,255),
			Color(170,0,0,230),
			Color(170,0,0,230),
			Color(150,0,0,210) );
		marker->setRadius(0.02);
		marker->move(0.25,0.245+0.07*part);

		
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
