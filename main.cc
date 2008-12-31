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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include "canvas.hh"
#include "db.hh"
#include "input.hh"
#include "systemmenu.hh"
#include "videopart.hh"
#include "musicpart.hh"
#include "gamespart.hh"

int main(int argc, char ** argv) {
	Stack * stack = constructSDLStack();
	stack->lockLayout();
	InputStack * input = createSDLInputStack();

	input->pushListener(stack);

	DB * db = NULL;
	#ifdef __HAVE_MYSQL__
	try {
		db = createMysqlDB();
	} catch(bool x) {}
	#endif
	if(db == NULL) db = createMemoryDB();
	
	SystemMenu * menu = createSystemMenu(stack, input, db);
	Part * videoPart = createVideoPart(stack, input, db);
	Part * musicPart = createMusicPart(stack, input, db);
	Part * gamesPart = createGamesPart(stack, input, db);
	menu->addPart( videoPart );
	menu->addPart( musicPart );
	menu->addPart( gamesPart );
	menu->run();
	delete menu;
	delete videoPart;
	delete musicPart;
	delete input;
	delete stack;
	delete db;
}
