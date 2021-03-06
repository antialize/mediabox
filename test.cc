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
