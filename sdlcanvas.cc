#include "canvas.hh"
#include <stdio.h>
#include <vector>
#include <list>
#include <utility>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "threading.hh"
#include <queue>
#include <map>
#include <string>
#include <utility>
#include <ext/hash_map>
#include "util.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.hh"


#define DIE(format, args...) {fprintf(stderr, format, ##args); exit(1);}
#define SDLERR(text) DIE("SDL failure at %s line %d: %s\n",__FILE__,__LINE__,SDL_GetError()) 

class SDLElement;
class SDLImage;
class SDLLabel;
class SDLStack;
class SDLFill;
class SDLCard;
class SDLText;

struct IRect: public ARect<uint32_t> {
	typedef uint32_t i_t;
	IRect(i_t w,i_t x,i_t y,i_t z): ARect<uint32_t>(w,x,y,z) {};
	inline operator SDL_Rect() {
		SDL_Rect _ = {l,t,w(),h()};
		return _;
	}

};

class SDLElement: public virtual Element {
public:
	SDLStack * stack;
	SDLCard * card;
	uint32_t zidx;
	Rect _rect;
	IRect irect;
	SDLElement(SDLStack * s, SDLCard * c, Rect r, uint32_t z);
	void invalidate();
	void remove();
	uint32_t zindex();
	void setzindex(uint32_t);
	void move(float,float);
	void resize(Rect &);
	virtual void rescale();
	virtual void render(IRect &r) = 0;
	Rect & rect() {return _rect;}
	~SDLElement() {remove();}
};

class SDLImage;
class Scaled;

struct FColor {
	float r;
	float g;
	float b;
	float a;
	FColor(const Color & c): r(c.r),g(c.g),b(c.b),a(c.a) {};
	FColor(float x,float y,float z,float _): r(x),g(y),b(z),a(_) {};
	FColor operator *(float s) {return FColor(r*s,g*s,b*s,a*s);}
	FColor operator +(const FColor & o) {return FColor(r+o.r,g+o.g,b+o.b,a+o.a);}
	Color operator() () {return Color((uint8_t)r,(uint8_t)g,(uint8_t)b,(uint8_t)a);}
	void operator=(const Color & c) {r=c.r;g=c.g;b=c.b;a=c.a;}
};


template <typename T> struct deathrow_reader {
	inplace_data<T> & operator() (T * t) {return t->deathrow_inplace;}
};

template <typename T> struct users_reader {
	inplace_data<T> & operator() (T * t) {return t->users_inplace;}
};

struct ScaledKey {
	const uint32_t w;
	const uint32_t h;
	const bool ar;
	ScaledKey(uint32_t a, uint32_t b, bool c): w(a), h(b), ar(c) {};
	inline bool operator ==(const ScaledKey & o) const {return w==o.w && h==o.h && ar==o.ar;}
	inline bool operator !=(const ScaledKey & o) const {return w!=o.w || h!=o.h || ar!=o.ar;}
};

namespace __gnu_cxx {
	template <> 
	struct hash<ScaledKey> {
		inline size_t operator() (const ScaledKey & x) const {return x.w * 101719 + x.h * 79633 + x.ar?42:123;}
	};
}



struct streq: public std::binary_function<char *, char *, bool> {
	bool operator() (const char * a, const char * b) const {return strcmp(a,b) == 0;}
};
	
	
class Unscaled {
public:
	typedef __gnu_cxx::hash_map<ScaledKey, Scaled*> usermap_t;
	struct cmp;
	struct LoaderThread;

	static inplace_list<Unscaled, deathrow_reader<Unscaled> > deathrow;
	static __gnu_cxx::hash_map<const char *, Unscaled *, __gnu_cxx::hash<const char *>, streq> all;
	static std::priority_queue<Unscaled *, std::vector<Unscaled*> , cmp> loadQueue;
	static Mutex lqMutex;
	static Mutex allMutex;
	static Mutex drMutex;
	static Cond lqCond;
	static LoaderThread loader;
	static bool die;

	SDL_Surface * surface;
	char * path;
	inplace_data<Unscaled> deathrow_inplace;
	usermap_t users;
	Mutex mutex;
	bool inQueue;
	
	void removeUser(Scaled * s);
	void addUser(Scaled * s);
	Unscaled(const char * p);
	~Unscaled();
};


class Scaled {
public:
	struct cmp;
	struct ScalerThread;

	static Mutex sqMutex;
	static Cond sqCond;
	static Mutex drMutex;
	static std::priority_queue<Scaled *, std::vector<Scaled*> , cmp> scaleQueue;
	static inplace_list<Scaled, deathrow_reader<Scaled> > deathrow;
	static ScalerThread scaler;
	static bool die;

	bool ownSurface;
	bool keepAspect;
	bool inQueue;
	uint32_t w;
	uint32_t h;
	inplace_data<Scaled> users_inplace;
	inplace_data<Scaled> deathrow_inplace;
	inplace_list<SDLImage, users_reader<SDLImage> > users;
	SDL_Surface * surface;
	Unscaled * unscaled;
	volatile int state;
	Mutex mutex;

	ScaledKey key() {return ScaledKey(w,h,keepAspect);}

	void removeUser(SDLImage * i);
	void addUser(SDLImage * i);
	void invalidate();
	Scaled(Unscaled *u, uint32_t w, uint32_t h, bool ka);
	~Scaled();
};

class SDLImage: public Image, public SDLElement {
public:
	inplace_data<SDLImage> users_inplace;
	Scaled * scaled;
	bool keepAspect;
	char * path;

	virtual void rescale();
	static void start();
	static void stop();

	SDLImage(SDLStack * s, SDLCard * c, Rect r, uint32_t z, const char * path,bool keepaspect=true);
	void render(IRect& r);
	void change(const char *);
	void assosiate();
	void disassosiate();
	~SDLImage();
};

class SDLFill: public Fill, public virtual SDLElement {
public:
	Color ul,ur,br,bl;
	SDL_Surface * s;
	float r;
	bool dirty;

	SDLFill(SDLStack * s, SDLCard * c, Rect re, uint32_t z, const Color c_) 
		:SDLElement(s,c,re,z),ul(c_),ur(c_),br(c_),bl(c_),s(NULL),r(0.0) {update();}
	void update();
	void setColor(const Color & c_);
	void setGradient(const Color & ul_, const Color & ur_, const Color & br_, const Color & bl_);
	void setRadius(float _);
	void render(IRect &r);
	void resize(Rect & r) {SDLElement::resize(r); dirty=true;}
	void rescale() {SDLElement::rescale(); dirty=true;}
};

std::map< std::pair< std::string, int>, TTF_Font *> fontCache;
TTF_Font * lookupFont(int fontHeight, std::string fontName) {
	if( fontCache.count( make_pair(fontName, fontHeight) ) == 0) {
		const char * places[] = {
			"/usr/share/fonts/","/usr/share/fonts/truetype/",
			"fonts/","",NULL};
		for(const char ** place=places; *place; ++place) {
			char path[2024];
			sprintf(path,"%s%s",*place,fontName.c_str());
			TTF_Font * f = TTF_OpenFont(path,fontHeight);
			if(f) {
				printf("%s\n",path);
				fontCache[make_pair(fontName, fontHeight)] = f;
				break;
			}
		}
	}
	return fontCache[make_pair(fontName, fontHeight)];
}

class SDLLabel: public Label, public virtual SDLElement {
public:
	uint32_t fontHeight;
	const char * fontName;
	float size;
	float mw;
	TTF_Font * font;
	SDL_Surface * renderedText;
	Color color;
	bool c;
	std::pair< double, double > loc;
	void center() {c = true;}
	std::string text;
	SDLLabel(SDLStack * s, SDLCard * c, uint32_t z, float x, float y, const char * v, float si);
	void reload();
	void render(IRect &r);
	void setValue(const char * val);
	void setFontSize(float size);
	void setColor(const Color & c);
	void setFont(const char * font);
	void setMaxWidth(float mw);
	virtual void rescale();
};

class SDLText: public Text, public virtual SDLElement {
public: 
	const char * fontName;
	uint32_t lc;
	TTF_Font * font;
	Color color;
	int firstLine;
	std::vector<SDL_Surface *> lines;
	std::string text;
	SDLText(SDLStack * s, SDLCard * c, Rect r, uint32_t z, const char * text, uint32_t ll);
	void render(IRect& r);
	void reload();
	void setValue(const char * val);
	void setLines(uint32_t lines);
	void setColor(const Color & c);
	void setFont(const char * font);
	void setFirstLine(int l);
	int getFirstLine() {return firstLine;}
	//void assosiate();
	//void disassosiate();
	virtual void rescale();
	~SDLText();
};

class SDLCard: public Card {
private:
	SDLStack * stack;
	std::vector<std::list<SDLElement*> > elements;
	void addElement(SDLElement *e);
public:
	SDLCard(SDLStack * s);
	void invalidate();
	Image * addImage(const char * path, uint32_t zindex, Rect r, bool keepAspcet=true);
	Fill * addFill(const Color & color, uint32_t zindex, Rect rect);
	Label * addLabel(const char * value, uint32_t zindex, float x, float y, float size);
	Text * addText(const char * value, uint32_t zindex, Rect r, uint32_t lines);	
	void remove(SDLElement* elm);
	void render(IRect rect);
	void rescale();
};

class SDLStack: public Stack {
public:
	SDL_Surface *screen;
	std::vector<SDLCard *> stack;
	std::vector<IRect> ir1;
	std::vector<IRect> ir2;
	std::vector<IRect> * oi;
	std::vector<IRect> * ni;
	uint32_t lc;

	void update() {
		if(lc) return;
		uint32_t l=32000,t=32000,r=0,b=0;
		for(std::vector<IRect>::iterator i=ir1.begin(); i != ir1.end(); ++i) {
			l=std::min(i->l,l);
			t=std::min(i->t,t);
			r=std::max(i->r,r);
			b=std::max(i->b,b);
		}
		for(std::vector<IRect>::iterator i=ir2.begin(); i != ir2.end(); ++i) {
			l=std::min(i->l,l);
			t=std::min(i->t,t);
			r=std::max(i->r,r);
			b=std::max(i->b,b);
		}
		if(l >= r || t >= b) return;
		for(std::vector<SDLCard *>::iterator i=stack.begin(); i != stack.end(); ++i)
			(*i)->render(IRect(l,t,r,b));
		SDL_Flip(screen);
		oi->clear();
		std::swap(ni,oi);
	};
	
	bool onUpdate() {
		update();
		return true;
	}

	void invalidate(IRect  r) {
		ni->push_back(r);
		if(lc) return;
		SDL_UserEvent e;
		e.code = 42;
		e.type = SDL_USEREVENT;
		SDL_PushEvent((SDL_Event*)&e);
	}

	bool onResize(int w, int h) {
		screen = SDL_SetVideoMode(w, h, 32,
								  SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
		lockLayout();
		invalidate(IRect(0,0,w,h));
		for(std::vector<SDLCard *>::iterator i=stack.begin(); i != stack.end(); ++i)
			(*i)->rescale();
		unlockLayout();
		return true;
	}

	bool onExpose() {
		invalidate(IRect(0,0,screen->w,screen->h));
		update();
		return true;
	}

	SDLStack(): lc(0) {
		oi = &ir1;
		ni = &ir2;
		SDL_Init(SDL_INIT_VIDEO);
		atexit(SDL_Quit);
		
		screen = SDL_SetVideoMode(720, 576, 32,
								  SDL_SWSURFACE | (cfg()("fullscreen",true)?SDL_FULLSCREEN | SDL_NOFRAME : SDL_RESIZABLE) );
		if(cfg()("fullscreen",true)) SDL_ShowCursor(SDL_DISABLE);
		SDLImage::start();
	}

	void lockLayout() {++lc;}
	void unlockLayout() {--lc; if(lc==0)update();}
	void pushCard(Card * c) {
		stack.push_back(static_cast<SDLCard*>(c));
		static_cast<SDLCard*>(c)->rescale();
		static_cast<SDLCard*>(c)->invalidate();
		update();
	}
	void popCard() {
		stack.back()->invalidate();
		stack.pop_back();
		update();
	}
	Card * constructCard() {return new SDLCard(this);}
	IRect irect(Rect r) {
		return IRect(
			(uint32_t)(r.l*(float)screen->w),
			(uint32_t)(r.t*(float)screen->h),
			(uint32_t)(r.r*(float)screen->w),
			(uint32_t)(r.b*(float)screen->h));
	}
};

using namespace std;
//==================================> SDLElement <===================================
SDLElement::SDLElement(SDLStack * s, SDLCard * c, Rect r, uint32_t z)
	: stack(s), card(c), zidx(z), _rect(r), irect(s->irect(r)) {
//invalidate();
}
void SDLElement::invalidate() {stack->invalidate(irect);}
void SDLElement::remove() {
	invalidate();
	card->remove(this);
}

uint32_t SDLElement::zindex() {return zidx;}
void SDLElement::setzindex(uint32_t) {}


void SDLElement::resize(Rect & r) {
	if(_rect == r) return;
	_rect = r;
	IRect i = stack->irect(r);
	if(i == irect) return;
	invalidate();
	irect = i;
	invalidate();
	stack->update();
}
void SDLElement::move(float x,float y) { 
	if(_rect.l == x && 	_rect.t == y) return;
	_rect.b += y - _rect.t;
	_rect.r += x - _rect.l;
	_rect.l = x;
	_rect.t = y;
	IRect i = stack->irect(_rect);
	if(i == irect) return;
	invalidate();
	irect = i;
	invalidate();
	stack->update();
}


void SDLElement::rescale() {irect = stack->irect(_rect);}

//================================> Unscaled <=======================================
inplace_list<Unscaled, deathrow_reader<Unscaled> > Unscaled::deathrow;
__gnu_cxx::hash_map<const char *, Unscaled *, __gnu_cxx::hash<const char *>, streq> Unscaled::all;
Mutex Unscaled::lqMutex;
Cond Unscaled::lqCond;
Mutex Unscaled::allMutex;
Mutex Unscaled::drMutex;
bool Unscaled::die;

struct Unscaled::cmp : public std::binary_function<Unscaled *,Unscaled *,bool> {
	bool operator() (Unscaled * a, Unscaled * b)  {return false;}
};

struct Scaled::cmp : public std::binary_function<Scaled *,Scaled *,bool> {
	bool operator() (Scaled * a, Scaled * b)  {return a->state < b->state;}
};


std::priority_queue<Unscaled *, std::vector<Unscaled*> , Unscaled::cmp> Unscaled::loadQueue;

void Unscaled::removeUser(Scaled * s) {
	mutex.lock();
	users.erase(s->key());
	if(!users.empty()) {
		mutex.unlock();
		return;
	}
	
	drMutex.lock();
	deathrow.push_back(this);
	mutex.unlock();
	
	while(deathrow.size() > 20) {
		Unscaled * u = deathrow.front();
		deathrow.pop_front();
		
		u->mutex.lock();
		if(u->inQueue) {
			u->inQueue = false;
			u->mutex.unlock();
		} else {
			u->mutex.unlock();
			delete u;
		}
	}
	drMutex.unlock();
}

void Unscaled::addUser(Scaled * s) {
	mutex.lock();
	if(users.empty()) {
		drMutex.lock();
		deathrow.remove(this);
		drMutex.unlock();
	}
	users[s->key()] = s;
	mutex.unlock();
}

Unscaled::Unscaled(const char * p): surface(NULL) {
	path = (char*)malloc(strlen(p)+1);
	strcpy(path,p);
	allMutex.lock();
	all[path] = this;
	allMutex.unlock();
	lqMutex.lock();
	inQueue=true;
	loadQueue.push(this);
	lqCond.signal();
	lqMutex.unlock();
}

Unscaled::~Unscaled() {
	allMutex.lock();
	all.erase(path);
	allMutex.unlock();
	
	mutex.lock();
	if(surface) SDL_FreeSurface(surface);
	surface = NULL;
	free(path);
	mutex.unlock();
}

#define STR(x) #x
#define XSTR(x) STR(x)

struct Unscaled::LoaderThread: public Thread<LoaderThread> {
	void run() {
		while(!die) {
			lqMutex.lock();
			while(loadQueue.empty()) {
				if(die) {lqMutex.unlock();return;}
				lqCond.wait(lqMutex);
			}
			Unscaled * i = loadQueue.top();
			loadQueue.pop();
			i->mutex.lock();
			if(!i->inQueue) { //Marked for deletion
				i->mutex.unlock();
				delete i;
				lqMutex.unlock();
				continue;
			}
			i->inQueue = false;
			i->mutex.unlock();
			lqMutex.unlock();

			char path[2048];
			strcpy(path,i->path);
			if(path[0] != '/') {
				int x = open(path,O_RDONLY);
				if(x == -1) 
					sprintf(path, "%s/%s", cfg()("data_path", XSTR(INSTALL_PREFIX) "/share/mediabox/") , i->path);
				else 
					close(x);
			}
			printf("load %s\n",path);
			SDL_Surface * s = IMG_Load(path);
			if(!s) {
				continue;
			}
			SDL_Surface * s2 = SDL_DisplayFormatAlpha(s);
			SDL_FreeSurface(s);
			i->mutex.lock();
			i->surface = s2;
			Scaled::sqMutex.lock();
			for(usermap_t::iterator x=i->users.begin(); x != i->users.end(); ++x) {
			  printf("%dx%d vs %dx%d\n",x->second->w,x->second->h,s2->w,s2->h);
			  if((int)x->second->w == (int)s2->w && (int)x->second->h == (int)s2->h) {
					x->second->mutex.lock();
					x->second->ownSurface = false;
					x->second->surface = i->surface;
					x->second->state = 3;
					x->second->invalidate();
					x->second->mutex.unlock();
				} else {
				  x->second->state = 1;
				  x->second->inQueue = true;
				  Scaled::scaleQueue.push(x->second);
				}
			}
			i->mutex.unlock();
			Scaled::sqCond.signal();
			Scaled::sqMutex.unlock();			
		}
	}
}; 
Unscaled::LoaderThread Unscaled::loader;


//================================> Scaled <=======================================
Mutex Scaled::sqMutex;
Cond Scaled::sqCond;
bool Scaled::die;
inplace_list<Scaled, deathrow_reader<Scaled> > Scaled::deathrow;

std::priority_queue<Scaled *, std::vector<Scaled*> , Scaled::cmp> Scaled::scaleQueue;

void Scaled::removeUser(SDLImage * i) {
	mutex.lock();
	users.remove(i);
	if(!users.empty()) {
		mutex.unlock();
		return;
	}
	mutex.unlock();
	deathrow.push_back(this);
	while(deathrow.size() > 20) {
		Scaled * s = deathrow.front();
		deathrow.pop_front();
		delete s;
	}
}

void Scaled::invalidate() {
	for(inplace_list<SDLImage, users_reader<SDLImage> >::iterator x= users.begin();
		x != users.end(); ++x)
		(*x)->invalidate();
}

void Scaled::addUser(SDLImage * i) {
	if(users.empty()) deathrow.remove(this);
	users.push_back(i);
}

Scaled::~Scaled() {
	if(surface && ownSurface) SDL_FreeSurface(surface);
	if(unscaled) unscaled->removeUser(this);
}

Scaled::Scaled(Unscaled * u, uint32_t a, uint32_t b, bool ka)
	: keepAspect(ka), w(a), h(b), surface(NULL), unscaled(u), state(0) {
	u->mutex.lock();
	u->users[key()] = this;
	if(u->surface) {
		sqMutex.lock();
		state = 1;
		inQueue = true;
		scaleQueue.push(this);
		sqCond.signal();
		sqMutex.unlock();
	}
	u->mutex.unlock();
};

struct Scaled::ScalerThread: public Thread<ScalerThread> {
	template <int bpp> inline void bilinearishInterpolationScale(SDL_Surface * dst,SDL_Surface * src) {
		SDL_LockSurface(src);
		SDL_LockSurface(dst);
		uint32_t dw=dst->w;
		uint32_t dh=dst->h;
		uint8_t * dp=(uint8_t*)dst->pixels;
		uint8_t * sp=(uint8_t*)src->pixels;
		float w=dst->w;
		float h=dst->h;
		for(uint32_t dy=0; dy < dh; ++dy) {
			float sy_a=(float)(dy*src->h)/h;
			float sy_b=(float)((dy+1)*src->h)/h;
			uint8_t * dline = dp+dy*dst->pitch;
			for(uint32_t dx=0; dx < dw; ++dx) {
				float sx_a=(float)(dx*src->w)/w;
				float sx_b=(float)((dx+1)*src->w)/w;
				
				float area=0;
				FColor c(0,0,0,0);
				uint32_t y=(uint32_t)sy_a;
				while((float)y < sy_b) {
					float y_a=std::max(sy_a,(float)y);
					float y_b=std::min(sy_b,(float)y+1);
					uint32_t x=(uint32_t)sx_a;
					while((float)x < sx_b) {
						float x_a=std::max(sx_a,(float)x);
						float x_b=std::min(sx_b,(float)x+1);
						float aa=(y_b-y_a)*(x_b-x_a);
						uint8_t r,g,b,a;
						SDL_GetRGBA( *(uint32_t*)(&(sp[y*src->pitch + x*bpp])),  src->format, &r,&g,&b,&a );
						c = c + FColor(r,g,b,a)*aa;
						area += aa;
						x++;
					}
					y++;
				}
				c = c *(1.0/area);
				uint32_t co = SDL_MapRGBA(src->format, (uint8_t)c.r, (uint8_t)c.g, (uint8_t)c.b, (uint8_t)c.a);
				for(int i=0; i < bpp; ++i)
					dline[dx*bpp+i] = ((uint8_t*)&co)[i]; //+4-bpp];
			}
		}
		SDL_UnlockSurface(dst);
		SDL_UnlockSurface(src);
	}
		
	
	template <int bpp> inline void nearestNeighborScale(SDL_Surface * dst,SDL_Surface * src) {
		SDL_LockSurface(src);
		SDL_LockSurface(dst);
		uint32_t dw=dst->w;
		uint32_t dh=dst->h;
		uint32_t sw=src->w;
		uint32_t sh=src->h;
		uint8_t * sp=(uint8_t*)src->pixels;
		uint8_t * dp=(uint8_t*)dst->pixels;
		for(uint32_t dy=0; dy < dh; ++dy) {
			uint32_t sy=dy*sh/dh;
			uint8_t * dline = dp+dy*dst->pitch;
			uint8_t * sline = sp+sy*src->pitch;
			for(uint32_t dx=0; dx < dw; ++dx) {
				uint32_t sx=dx*sw/dw;
				for(uint8_t i=0; i < bpp; ++i) 
					dline[dx*bpp+i] = sline[sx*bpp+i];
			}
		}
		SDL_UnlockSurface(dst);
		SDL_UnlockSurface(src);
	}
	
	void run() {
		while(!die) {
			sqMutex.lock();
			while(scaleQueue.empty()) {
				if(die) {sqMutex.unlock(); return;}
				sqCond.wait(sqMutex);
			}
			Scaled * i = scaleQueue.top();
			scaleQueue.pop();
			i->mutex.lock();
			if(!i->inQueue) {
				i->mutex.unlock();
				delete i;
				sqMutex.unlock();
				continue;
			}
			i->inQueue = false;

			sqMutex.unlock();
			//printf("scaling %16X @ %dx%d\n",i->unscaled,i->w,i->h);
			SDL_Surface * s = i->unscaled->surface;
			SDL_PixelFormat * f = s->format;
			uint32_t w = i->w;
			uint32_t h = i->h;
			if(i->keepAspect) {
				if(w*s->h < h*s->w) h = (s->h * w) / s->w;
				else w = (s->w * h) / s->h;
			}
			i->mutex.unlock();
			SDL_Surface * d = SDL_CreateRGBSurface(
				SDL_SWSURFACE, w, h, f->BitsPerPixel, f->Rmask,f->Gmask,f->Bmask,f->Amask);
			if(!d) 
				exit(1);
			//if(i->state == 1) {
			//	switch(s->format->BytesPerPixel) {
			//	case 1: nearestNeighborScale<1>(d,s); break;
			//	case 2: nearestNeighborScale<2>(d,s); break;
			//	case 3: nearestNeighborScale<3>(d,s); break;
			//	case 4: nearestNeighborScale<4>(d,s); break;
			//	}
			//} else if(i->state ==2 ) {
			switch(s->format->BytesPerPixel) {
			case 1: bilinearishInterpolationScale<1>(d,s); break;
			case 2: bilinearishInterpolationScale<2>(d,s); break;
			case 3: bilinearishInterpolationScale<3>(d,s); break;
			case 4: bilinearishInterpolationScale<4>(d,s); break;
			}
			//}
			i->mutex.lock();
			if(i->surface && i->ownSurface)
				SDL_FreeSurface(i->surface); 
			i->surface = d;
			i->ownSurface = true;
			++i->state;
			if(i->state < 3) {
				sqMutex.lock();
				i->inQueue = true;
				scaleQueue.push(i);
				sqMutex.unlock();
			}
			i->invalidate();
			i->mutex.unlock();
		}
	}
};
Scaled::ScalerThread Scaled::scaler;

//================================> SDLImage <=======================================

void SDLImage::start() {
	Unscaled::die = false;
	Scaled::die = false;
	Unscaled::loader.start();
	Scaled::scaler.start();
}
void SDLImage::stop() {
	Unscaled::die = true;
	Scaled::die = true;
	Unscaled::lqCond.signalAll();
	Scaled::sqCond.signalAll();
	Unscaled::loader.join();
	Scaled::scaler.join();
}

void SDLImage::disassosiate() {
	if(scaled) {
		scaled->removeUser(this);
		scaled = NULL;
	}
}

void SDLImage::assosiate() {
	Unscaled * u = Unscaled::all.count(path)?Unscaled::all[path]:new Unscaled(path);
	scaled = u->users.count(ScaledKey(irect.w(),irect.h(),keepAspect))?
		u->users[ ScaledKey(irect.w(),irect.h(),keepAspect) ]:
		new Scaled(u, irect.w(), irect.h(), keepAspect);
	scaled->addUser(this);
}

void SDLImage::rescale() {
	SDLElement::rescale();
	disassosiate();
	assosiate();
}

SDLImage::SDLImage(SDLStack * s, SDLCard * c, Rect r, uint32_t z, const char * p, bool ka)
  :SDLElement(s,c,r,z),scaled(NULL),keepAspect(ka),path(NULL) {
	change(p);
}

void SDLImage::render(IRect& r) {
	if(!scaled) return;
	scaled->mutex.lock();
	if(scaled->state > 1) {
		uint32_t sx = (irect.w() - scaled->surface->w) / 2;
		uint32_t sy = (irect.h() - scaled->surface->h) / 2;
		SDL_Rect d = r;
		SDL_Rect s = {r.l - irect.l-sx,r.t - irect.t-sy,r.w(),r.h()};
		SDL_BlitSurface(scaled->surface, &s, stack->screen, &d);
	}
	scaled->mutex.unlock();
}

void SDLImage::change(const char * p) {
	invalidate();
	disassosiate();
	path = (char *)p;
	assosiate();
}

SDLImage::~SDLImage() {
	disassosiate();
}

//==================================> SDLFill <======================================

#include <math.h>


template <typename T> T sq(T x) {return x*x;}

void SDLFill::update() {
	if(ul.a == 0xFF && r == 0.0 && ul==ur && ul==br && ul==bl) {
		if(s) SDL_FreeSurface(s);
		s=NULL;
	} else {
		if(s) SDL_FreeSurface(s);
		SDL_Surface  *_= SDL_CreateRGBSurface(SDL_SWSURFACE, irect.w(), irect.h(), 1,1,1,1,1);
		s = SDL_DisplayFormatAlpha(_);
		SDL_FreeSurface(_);
		SDL_LockSurface(s);
		//uint32_t cc = SDL_MapRGBA(s->format,c.r,c.g,c.b,c.a);
		//uint32_t co;
		uint8_t * dp=(uint8_t*)s->pixels;
		uint8_t bpp = s->format->BytesPerPixel;
		IRect i=stack->irect(Rect(0,0,r/2,r/2));
		uint32_t r = i.r + i.b;

		FColor ul_(ul);
		FColor ur_(ur);
		FColor bl_(bl);
		FColor br_(br);
		float h=irect.h();
		float w=irect.w();
		for(uint32_t y=0; y < irect.h(); ++y) {
			uint8_t * dline = dp+y*s->pitch;
			FColor l_ = ul_ * ((h-y)/h) + bl_ * ((float)y/h);
			FColor r_ = ur_ * ((h-y)/h) + br_ * ((float)y/h);
			for(uint32_t x=0; x < irect.w(); ++x) { 
				FColor c = l_ * ((w-x)/w) + r_*(((float)x)/w);
				float as = 1;
				if(y < r && x < r) 
					if(sq(r-y)+sq(r-x) > sq(r)) as = 0;	
				if(y < r && x > irect.w() - r) 
					if(sq(r-y)+sq(x+r-irect.w()) > sq(r)) as =0;
				if(y > irect.h() - r && x < r) 
					if(sq(y+r-irect.h())+sq(r-x) > sq(r)) as = 0;	
				if(y > irect.h() - r && x > irect.w() - r) 
					if(sq(y+r-irect.h())+sq(x+r-irect.w()) > sq(r)) as =0;
				uint32_t co = SDL_MapRGBA(s->format,(uint16_t)c.r,(uint16_t)c.g,(uint16_t)c.b,(uint16_t)(c.a*as));
				for(int i=0; i < bpp; ++i)
					dline[x*bpp+i] = ((uint8_t*)&co)[i]; //+4-bpp];
			}
		}
		SDL_UnlockSurface(s);
	}
	dirty=false;
}

void SDLFill::setRadius(float _) {r=_; dirty=true; invalidate();}

void SDLFill::setGradient(const Color & ul_, const Color & ur_, const Color & br_, const Color & bl_) {
	ul=ul_;ur=ur_;br=br_;bl=bl_;
	dirty=true; invalidate();
}

void SDLFill::setColor(const Color & _) {
	ul=ur=br=bl=_;
	dirty=true; invalidate();
}

void SDLFill::render(IRect &r) {
	if(dirty) update();
	SDL_Rect d = r;
	if(!s) {
		uint32_t cc = SDL_MapRGB(stack->screen->format,ul.r,ul.g,ul.b);
		SDL_FillRect(stack->screen,&d,cc);
	} else {
		SDL_Rect sr = {r.l - irect.l,r.t - irect.t,r.w(),r.h()};
		SDL_BlitSurface(s, &sr, stack->screen, &d);
	}
}

//====================================> Label <======================================


SDLLabel::SDLLabel(SDLStack * stack, SDLCard * card, uint32_t z, float x, float y, const char * txt, float fs):
	SDLElement(stack,card,Rect(x,y,x,y),z), fontName("ttf-bitstream-vera/Vera.ttf"), size(fs), text(txt), loc(x,y),
	mw(1), renderedText(NULL), color(0,0,0)
{
	if(!TTF_WasInit() && TTF_Init()==-1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(1);
	}
	c = false;
	reload();
}

void SDLLabel::rescale() {
	reload();
}

void SDLLabel::reload() {
	invalidate();
	if(renderedText != NULL) {SDL_FreeSurface(renderedText); renderedText=NULL;}
	fontHeight = (int)((float)stack->screen->h * size);
	font = lookupFont(fontHeight, fontName);
	if(!font) return;
	if(text.size() == 0) return;
	
	SDL_Color co = {0,0,0};
	SDL_Surface * s = TTF_RenderUTF8_Solid(font, text.c_str(), co);
	if(s == NULL) SDLERR("TTF_RenderUTF8_Solid");
	renderedText = SDL_DisplayFormatAlpha(s);
	if(renderedText == NULL) SDLERR("SDL_DisplayFormatAlpha");
	SDL_FreeSurface(s);
	_rect.l = loc.first;
	_rect.t = loc.second;
	
	_rect.r = _rect.l + std::min((float)renderedText->w/(float)stack->screen->w,mw);
	if(c) {
		float x = mw - (_rect.r - _rect.l);  
		_rect.r += x/2;
		_rect.l += x/2;
	}
	_rect.b = _rect.t + (float)renderedText->h/(float)stack->screen->h;
	irect = stack->irect(_rect);
	invalidate();
	stack->update();
}

void SDLLabel::render(IRect &r) {
	SDL_Rect d = r;
	SDL_Rect s;
	if(c) {
		uint32_t sx = (irect.w() - renderedText->w) / 2;
		uint32_t sy = (irect.h() - renderedText->h) / 2;
		SDL_Rect s1 = {r.l - irect.l-sx,r.t - irect.t-sy,r.w(),r.h()};
		s=s1;
	} else {
		SDL_Rect s2 = {r.l-irect.l,
			 r.t-irect.t,
			 std::min(r.w(),renderedText->w-r.l+irect.l),
			 std::min(r.h(),renderedText->h-r.t+irect.t)};
		s=s2;
	}
	if(renderedText) SDL_BlitSurface(renderedText, &s, stack->screen, &d);
}

void SDLLabel::setValue(const char * t) {text=t;reload();}
void SDLLabel::setFontSize(float s) {size=s;reload();}
void SDLLabel::setColor(const Color & c) {color = c;reload();}
void SDLLabel::setFont(const char * font) {fontName = font;reload();}
void SDLLabel::setMaxWidth(float _) {mw=_;reload();}

//========================================> Sdl text <====================================

void SDLText::setValue(const char * val) {text=val; firstLine=0; reload();}
void SDLText::setLines(uint32_t lines) {lc=lines; reload();}
void SDLText::setColor(const Color & c)  {color = c;reload();}
void SDLText::setFont(const char * font) {fontName = font;reload();}
void SDLText::setFirstLine(int fl) {firstLine = fl;
	if(firstLine > (int)lines.size() - (int)lc) firstLine=(int)lines.size()-(int)lc;
	if(firstLine < 0) firstLine=0;
	invalidate();
	stack->update();
}

SDLText::SDLText(SDLStack * s, SDLCard * c, Rect r, uint32_t z, const char * txt, uint32_t ll):
	SDLElement(s,c, r,z), fontName("ttf-bitstream-vera/Vera.ttf"), text(txt), lc(ll), color(0,0,0), firstLine(0) {
}

void SDLText::reload() {
	for(uint32_t i=0; i < lines.size(); ++i) SDL_FreeSurface(lines[i]);
	lines.clear();
	if(text.size() == 0) {
		invalidate();
		stack->update();
	}
	irect = stack->irect(_rect);
	int h=irect.h() / lc;
	int t=h;
	do {
		font = lookupFont(h, fontName);
		if(!font) return;
		--h;
	} while( TTF_FontHeight(font) > t);

	uint32_t line_start=0;
	if(text[text.size()-1] != ' ') text.push_back(' ');
	while(line_start < text.size()) {
		while(text[line_start] == ' ') ++line_start;
		std::string l="";
		uint32_t oline_end;
		for(uint32_t line_end=line_start; line_end < text.size(); ++line_end) {
			std::string p = text.substr(line_start, min<uint32_t>(line_end-line_start+1,text.size()-line_start));
			string::size_type pos = min(p.find_last_not_of(' '),p.find_last_not_of('\n'));
			if(pos != string::npos) p.erase(pos + 1);
			if(text[line_end] == ' ' || text[line_end] == '.' || text[line_end] == ',' || 
			   text[line_end] == '\t' || text[line_end] == '\n' || text[line_end] == '\r' ||
			   text[line_end] == ':' || text[line_end] == ';' || text[line_end] == '-' ) 
				;
			else if(line_end > 0 && text[line_end-1] != ' ')
				p.push_back('-');
			else 
				continue;
			int w;
			if(p.size() == 0) w = 0;
			else TTF_SizeUTF8(font, p.c_str() , &w,NULL);
			//std::cout << "S: " << p << " W: " << w << std::endl;
			if(w <= irect.w()) {l=p; oline_end=line_end;}
			else break;
		}
		SDL_Color co = {0,0,0};
		if(l.empty()) break;
		//std::cout << oline_end << std::endl;
		SDL_Surface * s = TTF_RenderUTF8_Solid(font,  l.c_str(), co);
		if(s == NULL) SDLERR("TTF_RenderUTF8_Solid");
		SDL_Surface * renderedText = SDL_DisplayFormatAlpha(s);
		if(renderedText == NULL) SDLERR("SDL_DisplayFormatAlpha");
		SDL_FreeSurface(s);
		lines.push_back(renderedText);
		line_start=oline_end+1;
	}
	if(firstLine > (int)lines.size() - (int)lc) firstLine=(int)lines.size()-(int)lc;
	if(firstLine < 0) firstLine=0;
	invalidate();
	stack->update();
}

void SDLText::render(IRect &r) {
	SDL_Rect sr = {r.l - irect.l,r.t - irect.t,r.w(),r.h()};
	int fl=firstLine;
	uint32_t tt=irect.t;
	for(uint32_t i=fl; i < fl+lc && i < lines.size(); ++i) {
		if( tt <= r.b && tt+lines[i]->h >= r.t) {
			//std::cout << i << std::endl;
			int top=max<uint32_t>(sr.y,tt);
			int bot=min<uint32_t>(tt+lines[i]->h, sr.y+sr.h);
			SDL_Rect ssr = {sr.x, top-tt, sr.w, bot-top};
			SDL_Rect d = {r.l, top, ssr.w, ssr.h};
			SDL_BlitSurface(lines[i], &ssr, stack->screen, &d);
		
		}
		tt += lines[i]->h;
	}
}

void SDLText::rescale() {
	reload();
}

SDLText::~SDLText() {
	for(uint32_t i=0; i < lines.size(); ++i) SDL_FreeSurface(lines[i]);
	lines.clear();
}

//===================================================================================

void SDLCard::render(IRect rect) {
	for(std::vector<std::list<SDLElement*> >::iterator i=elements.begin();
		i != elements.end(); ++i)
		for(std::list<SDLElement*>::iterator j=i->begin(); j != i->end(); ++j) {
			IRect rr(std::max((*j)->irect.l,rect.l),
					 std::max((*j)->irect.t,rect.t),
					 std::min((*j)->irect.r,rect.r),
					 std::min((*j)->irect.b,rect.b));
			if(rr.l >= rr.r || rr.t >= rr.b) continue;
			(*j)->render(rr);
		}
}

void SDLCard::rescale() {
	for(std::vector<std::list<SDLElement*> >::iterator i=elements.begin();
		i != elements.end(); ++i)
		for(std::list<SDLElement*>::iterator j=i->begin(); j != i->end(); ++j) 
			(*j)->rescale();
}

SDLCard::SDLCard(SDLStack * s): stack(s) {}

void SDLCard::addElement(SDLElement * e) {
	uint32_t zindex = e->zindex();
	if(elements.size() < zindex+1) elements.resize(zindex+5);
	elements[zindex].push_back(e);
	e->invalidate();
}

Image * SDLCard::addImage(const char * path, uint32_t zindex, Rect r, bool keepAspect) {
	SDLImage * f = new SDLImage(stack,this,r,zindex,path,keepAspect);
	addElement(f);
	return f;
}

Fill * SDLCard::addFill(const Color & c, uint32_t zindex, Rect rect) {
	SDLFill * f = new SDLFill(stack,this,rect,zindex,c);
	addElement(f);
	return f;
}

Label * SDLCard::addLabel(const char * value, uint32_t zindex, float x, float y, float size) {
	SDLLabel * l = new SDLLabel(stack,this,zindex, x,y, value ,size);
	addElement(l);
	return l;
}

Text * SDLCard::addText(const char * value, uint32_t zindex, Rect r, uint32_t lines) {
	SDLText * t = new SDLText(stack,this,r, zindex,  value, lines);
	addElement(t);
	return t;
}

void SDLCard::invalidate() {
	for(std::vector<std::list<SDLElement*> >::iterator i=elements.begin();
		i != elements.end(); ++i)
		for(std::list<SDLElement*>::iterator j=i->begin(); j != i->end(); ++j) 
			(*j)->invalidate();
}

void SDLCard::remove(SDLElement* elm) {
	elements[elm->zindex()].remove(elm);
}

Stack * constructSDLStack() {
	return new SDLStack();;
}

