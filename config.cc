#include "config.hh"
#include <map>
#include <string>
#include <stdio.h>

using namespace std;

class RealConfig: public Config {	
private:
	char * data;
	typedef map<const char *, const char *> m_t;
	m_t values;
public:
	RealConfig() {								
		data = NULL;
		FILE * f = fopen("~mediabox","rb");
		if(!f) return;
		size_t s = fseek(f,0,SEEK_END);
		fseek(f,0,SEEK_SET);
		data = (char *)malloc(s+1);
		fread(data,1,s,f);
		fclose(f);
		data[s] = '\0';
		for(size_t i=0; i < s;++i) {
			while(data[i] == '\t' || data[i] == ' ') ++i;
			char * name = data + i;
			while(data[i] != '#' && data[i] != '=' &&
				  data[i] != '\n' && data[i] != ' ' &&
				  data[i] != '\0' && data[i] != '\t') ++i;
			if(data[i] == '\t' || data[i] != ' ') {
				data[i] = '\0'; 	++i;
				while(data[i] == '\t' || data[i] == ' ') ++i;
			}
			if(data[i] == '=') {
				++i;
				int nbl=i;
				while(data[i] == '\t' || data[i] == ' ') ++i;
				char * val=data+i;
				while(data[i] != '\n' && data[i] != '\0' 
					  && data[i] != '#') {
					if(data[i] != '\t' && data[i] != ' ') nbl=i;
					++i; 
				}
				data[nbl+1] = '\0';
				values[name] = val;
			}
			while(data[i] != '\n' && data[i] != '\0') ++i;
		}
	}

	int operator() (const char * name, int def) const {
		m_t::const_iterator i=values.find(name);
		if(i == values.end()) return def;
		return atoi(i->second);
	}

	const char * operator() (const char * name, const char * def) const {
		m_t::const_iterator i=values.find(name);
		if(i == values.end()) return def;
		return i->second;
	}

	bool operator() (const char * name, bool def) const {
		m_t::const_iterator i=values.find(name);
		if(i == values.end()) return def;
		char c = i->second[0];
		return c!='f'&&c!='F'&&c!='0';
	}

	~RealConfig() {
		free(data);
	}
};

static RealConfig * c = NULL;
Config & cfg() {
	if(!c) c = new RealConfig();
	return *c;
}
