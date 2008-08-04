#ifndef __config_hh__
#define __config_hh__

class Config {
public:
	virtual ~Config() {};
	virtual bool operator() (const char * name, bool def) const = 0;
	virtual const char * operator() (const char * name, const char *) const = 0;
	virtual int operator() (const char * name, int def) const = 0;
};

Config & cfg();

#endif //__config_hh__
