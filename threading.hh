// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet 
/*
 * Mediabox: a light mediacenter solution
 * Copyright (C) 2009 Jakob Truelsen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __threading_hh__
#define  __threading_hh__
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <time.h>

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
	bool wait(Mutex & m, double time) {
		struct timespec ts;
		double i;
		double f = modf(time, &i);  
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_sec += (long)i;
		ts.tv_nsec += (long)(f*1000000000);
		return pthread_cond_timedwait(&c, &m.m, &ts) == ETIMEDOUT;
	}
};

#endif //__threading_hh__
