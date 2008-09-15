#include <SDL/SDL.h>
#include "input.hh"
#include <list>
#include <map>

class SDLInputStack: public InputStack {
public:
	typedef std::list<InputListener*> ll;
	ll ls;
	std::map<SDLKey,int> specialMap;
	bool dead;
	SDLInputStack(): dead(false) {
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
		SDL_EnableUNICODE(true);
		specialMap[SDLK_ESCAPE] = InputListener::escape;
		specialMap[SDLK_LEFT] = InputListener::left;
		specialMap[SDLK_RIGHT] = InputListener::right;
		specialMap[SDLK_UP] = InputListener::up;
		specialMap[SDLK_DOWN] = InputListener::down;
		specialMap[SDLK_PAGEUP] = InputListener::pageup;
		specialMap[SDLK_PAGEDOWN] = InputListener::pagedown;
		specialMap[SDLK_RETURN] = InputListener::enter;
	}
	
	void pushListener(InputListener * l) {ls.push_front(l);}
	InputListener * popListener() {InputListener * l=ls.front();ls.pop_front();return l;}
	void removeListener(InputListener * l) {ls.remove(l);}

	bool handleEvent(SDL_Event & event) {
		switch(event.type) {
		case SDL_QUIT:
			dead=true;
			break;
		case SDL_KEYDOWN:
			if(specialMap.count(event.key.keysym.sym)) {
				int special = specialMap[event.key.keysym.sym];
				for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
					if( (*i)->onSpecialKey(special) ) return !dead;
			}
			for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
				if( (*i)->onKey(event.key.keysym.unicode) ) break;
			break;
		case SDL_VIDEORESIZE:
			for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
				if( (*i)->onResize( event.resize.w, event.resize.h ) ) break;
			break;
		case SDL_VIDEOEXPOSE:
			for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
				if( (*i)->onExpose() ) break;
			break;
		case SDL_USEREVENT:
			if(event.user.code == 42) {
				for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
					if( (*i)->onUpdate() ) break;
			} else {
				for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
					if( (*i)->onUser(event.user.code, event.user.data1) ) break;
			}
		}
		return !dead;
	}

	void triggerUser(int code, void * data) {
		SDL_Event event;
		event.type = SDL_USEREVENT;
		event.user.code = code;
		event.user.data1 = data;
		SDL_PushEvent(&event);
	}
	
	void triggerSpecialKey(int key) {
		for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
			if( (*i)->onSpecialKey(key) ) break;
	}
	
	void triggerKey(int key) {
		for(ll::iterator i=ls.begin(); i != ls.end(); ++i)
			if( (*i)->onKey(key) ) break;
	}
	
	bool wait() {
		SDL_Event event;
		SDL_WaitEvent(&event); if(!handleEvent(event)) return false;
		while(SDL_PollEvent(&event)) if(!handleEvent(event)) return false;
		return true;
	}
	
	bool poll() {
		SDL_Event event;
		while(SDL_PollEvent(&event)) if(!handleEvent(event)) return false;
		return true;
	}

	void mainLoop() {
		while(wait());
	}
	void terminate() {
		SDL_Event e;
		e.type = SDL_QUIT;
		SDL_PushEvent(&e);
	}
};

InputStack * createSDLInputStack() {
	return new SDLInputStack();
}
