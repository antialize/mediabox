// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet 
/*
 * Mediabox
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
#ifndef __player_hh__
#define __player_hh__
#include "input.hh"

class PlayerHook {
public:
	virtual void onFinish() {};
	virtual ~PlayerHook() {};
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
