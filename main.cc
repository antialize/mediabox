#include "canvas.hh"
#include "db.hh"
#include "input.hh"
#include "systemmenu.hh"
#include "videopart.hh"

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
	
	SystemMenu * menu = createSystemMenu(stack,input,db);
	Part * videoPart = createVideoPart(stack,input,db);
	menu->addPart( videoPart );

	menu->run();
	delete menu;
	delete videoPart;
	delete input;
	delete stack;
	delete db;
}
