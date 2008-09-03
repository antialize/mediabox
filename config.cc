#include "config.hh"
#include <map>
#include <string>
#include <stdio.h>

using namespace std;


struct scmp {
	bool operator() (const char * a, const char * b) const {
		return strcmp(a,b) < 0;
	}
};

class RealConfig: public Config {	
private:
	char * data;
	typedef map<const char *, const char *, scmp> m_t;
	m_t values;
public:
	RealConfig() {								
		data = NULL;
		char buff[1024];
		sprintf(buff,"%s/.mediabox",getenv("HOME"));
		FILE * f = fopen(buff,"rb");
		if(!f) return;
		fseek(f,0,SEEK_END);
		size_t s = ftell(f);
		fseek(f,0,SEEK_SET);
		data = (char *)malloc(s+1);
		fread(data,s,1,f);
		fclose(f);
		data[s] = '\0';
		for(size_t i=0; i < s;++i) {
			while(data[i] == '\t' || data[i] == ' ') ++i;
			char * name = data + i;
			while(data[i] != '#' && data[i] != '=' &&
				  data[i] != '\n' && data[i] != ' ' &&
				  data[i] != '\0' && data[i] != '\t') ++i;
			while(data[i] == '\t' || data[i] == ' ') {data[i] = '\0'; ++i;}
			if(data[i] == '=') {
				data[i] = '\0';
				++i;
				int nbl=i;
				while(data[i] == '\t' || data[i] == ' ') {data[i] = '\0'; ++i;}
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
