#ifndef __player_hh__
#define __player_hh__

class Player {
public:
	virtual ~Player() {};
	virtual void setVolume(float volume) = 0;
	virtual void setPause(bool pause) = 0; 
	virtual float getLength() = 0;
	virtual float getPos() = 0;
	virtual void setPos(float p) = 0;
	virtual void setMute(bool bute) = 0;
	virtual void stop() = 0;
	virtual void play(char * path) = 0;
	virtual void wait() = 0;
};

Player * constructMPlayer();
#endif //__player_hh__
