#ifndef __threading_hh__
#define  __threading_hh__
#include <pthread.h>

template <class X> class Thread {
private:
	static void * run(void * self) {
		static_cast<X*>(self)->run();
		return NULL;
	}
public:
	pthread_t t;
	~Thread() {}
	void start() {pthread_create(&t,NULL,run,this);}
	void join() {pthread_join(t,NULL);}
};

class Mutex {
public:
	pthread_mutex_t m;
	Mutex() {pthread_mutex_init(&m,NULL);}
	~Mutex() {pthread_mutex_destroy(&m);}
	void lock() {pthread_mutex_lock(&m);}
	void unlock() {pthread_mutex_unlock(&m);}
};

class Cond {
public:
	pthread_cond_t c;
	Cond() {pthread_cond_init(&c,NULL);}
	~Cond() {pthread_cond_destroy(&c);}
	void signal() {pthread_cond_signal(&c);}
	void signalAll() {pthread_cond_broadcast(&c);}
	void wait(Mutex & m) {pthread_cond_wait(&c,&m.m);}
};

#endif //__threading_hh__
