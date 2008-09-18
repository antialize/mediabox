#ifndef __player_hh__
#define __player_hh__
#include "input.hh"

class PlayerHook {
public:
	virtual void onFinish() {};
};

class Player: public InputListener {
public:
	virtual ~Player() {};
	virtual void setVolume(float volume) = 0;
	virtual void incVolume() = 0;	
	virtual void decVolume() = 0;

	virtual void setPause(bool pause) = 0; 
	virtual float getLength() = 0;
	virtual float getPos() = 0;
	virtual void setPos(float p) = 0;
	virtual void setMute(bool bute) = 0;
	virtual void stop() = 0;
	virtual void play(const char * path) = 0;
	virtual void wait() = 0;
	virtual bool onSpecialKey(int i) = 0;
	virtual bool running() = 0;
	virtual void setHook(PlayerHook * hook) = 0;
};

Player * constructMPlayer(PlayerHook * hook=0);
#endif //__player_hh__
