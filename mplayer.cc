#include "player.hh"
#include "threading.hh"

#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>

#define DIE(msg) {perror(msg); exit(1);}

class MPlayer: public Player {
private:
	pid_t mplayer;
	int in;
	int out;
	Mutex m;
	volatile float length;
	volatile float pos;
	Cond length_cond;
	Cond pos_cond;
	pthread_t rt;
	template <typename T> T get(char * cmd,Cond & cond,volatile T & elm) {
		T tmp;
		m.lock();
		write(in,cmd,strlen(cmd));
		cond.wait(m);
		tmp = elm;
		m.unlock();
		return tmp;
	}
	void set(char * cmd,...) {
		char buff[128];
		va_list ap;
		va_start(ap, cmd);
		int len = vsnprintf(buff,128,cmd,ap);
		write(in,buff,len);
		va_end(ap);
	}

	template <typename T> bool readVar(char * line, char *scan, T & res, Cond & cond) {
		char * i = strchr(scan,'=');
		if(strncmp(line,scan,i-scan+1)) return false;
		m.lock();
		sscanf(line,scan,&res);
		cond.signalAll();
		m.unlock();
		return true;
	}
	
	void realReadThread() {
		char buf[1024];
		ssize_t a=0;
		ssize_t s=0;
		ssize_t e=0;
		while(true) {
			ssize_t j = read(out,buf+s,1024-s);
			if(j == 0) break;
			e+=j;
			for(; s < e; ++s) {
				if(buf[s] != '\n') continue;
				buf[s] = '\0';

				if(readVar(buf+a,"ANS_LENGTH=%f",length,length_cond));
				else if(readVar(buf+a,"ANS_TIME_POSITION=%f",pos,pos_cond));
				else {
					printf("MPlayer: %s\n",buf+a);
				}
				
				a = s+1;
			}
			for(s=0; s < e-a; ++s) buf[s]=buf[a+s];
			e-=a;
			a=0;
		}
	}
	static void * readThread(void * _self) {
		static_cast<MPlayer*>(_self)->realReadThread();
		return NULL;
	}

public:
	MPlayer() {mplayer=in=out=-1;}
	~MPlayer() {stop();}
	void setVolume(float volume) {set("volume %f 1\n",volume);}
	void setPause(bool pause) {set("pause %d\n",pause?1:0);}
	float getLength() {return get<float>("get_time_length\n",length_cond,length);}
	float getPos() {return get<float>("get_time_pos\n",pos_cond,pos);}
	void setPos(float p) {set("seek %f 2\n",p);}
	void setMute(bool mute) {set("mute %d\n",mute?1:0);}
	void stop() {
		if(mplayer == -1) return;
		set("quit\n");
		close(in);
		close(out);
		waitpid(mplayer,NULL,0);
		mplayer = -1;
	}
	void wait() {
		if(mplayer == -1) return;
		close(in);
		close(out);
		waitpid(mplayer,NULL,0);
		mplayer = -1;
	}

	void play(const char * file) {
		int inpipe[2];
		int outpipe[2];
		if(pipe(inpipe) == -1) DIE("pipe");
		if(pipe(outpipe) == -1) DIE("pipe");
		mplayer = fork();
		if(mplayer == -1) DIE("fork");
		if(mplayer == 0) {
			char * argv[] = {"mplayer","-quiet","-slave","-fs","-zoom",(char *)file,NULL};
			dup2(inpipe[0],0);
			dup2(outpipe[1],1);
			execvp("mplayer", argv);
			exit(2);
		}
		in = inpipe[1];
		out = outpipe[0];
		pthread_create(&rt,NULL,readThread,this);
	}
};

Player * constructMPlayer() {
	return new MPlayer();
}
