#include "player.hh"

#include <unistd.h>
#include <stdio.h>

int main(int argc, char ** argv) {
	Player * p = constructMPlayer(); ;
	p->play("/mnt/media/Musik/Rock/Mew/Mew - Apocalypso.mp3");
	//p->setVolume(100);
	float l = p->getLength();
	while(true) {
		printf("%f/%f\n",p->getPos(),l);
	}

//p->setVolume(40);
	p->wait();
}
