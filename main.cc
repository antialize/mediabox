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
