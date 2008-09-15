#ifndef __input_hh__
#define __input_hh__

class InputListener {
public:
	enum {
		up,
		down,
		left,
		right,
		enter,
		escape,
		pageup,
		pagedown,
		pause
	};
	virtual bool onSpecialKey(int key) {return false;}
	virtual bool onKey(int unicode) {return false;}
	virtual bool onResize(int w, int h) {return false;}
	virtual bool onExpose() {return false;}
	virtual bool onUpdate() {return false;}
	virtual bool onUser(int code, void * data) {return false;}
	virtual ~InputListener() {}
};

class InputHandler {
public:
	virtual ~InputHandler() {};
};

class InputStack {
public:
	virtual void pushListener(InputListener * l) = 0;
	virtual InputListener * popListener() = 0;
	virtual void removeListener(InputListener * l) = 0;
	virtual bool wait() = 0;
	virtual bool poll() = 0;
	virtual void mainLoop() = 0;
	virtual void terminate() = 0;
	virtual ~InputStack() {};
	
	virtual void triggerSpecialKey(int key) = 0;
	virtual void triggerKey(int unicode) = 0;
	virtual void triggerUser(int code, void * data) = 0;
};

InputStack * createSDLInputStack();

#endif //__input_hh__
