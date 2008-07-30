#include "db.hh"
#include <map>
#include <vector>
#include <string>
using namespace std;

class MemoryDB: public DB {
public:
	map<string,map<vector<string>,vector<string> > > db;
	void describeTabel(const char * name, const char ** keys, const char ** values) {
		if(!db.count(string(name))) db[string(name)]=map<vector<string>,vector<string> >();
	}
	void update(const char * tbl, const char ** keys, const char ** values) {
		vector<string> k;
		for(;*keys; keys++) k.push_back(string(*keys));
		vector<string> v;
		for(;*values; values++) v.push_back(string(*values));
		/*printf("storing (");
		for(vector<string>::iterator i=v.begin(); i != v.end(); ++i)
			printf("%s, ",i->c_str());
		printf(") at (");
		for(vector<string>::iterator i=k.begin(); i != k.end(); ++i)
			printf("%s, ",i->c_str());
			printf(")\n");*/
		db[tbl][k] = v;
	}
	bool fetch(const char * tbl, const char ** keys, char ** values) {
		vector<string> k;
		for(;*keys; keys++) k.push_back(string(*keys));
		if(!db[tbl].count(k)) return false;
		vector<string> & v = db[tbl][k];
		for(vector<string>::iterator i=v.begin(); 
			i != v.end(); ++i, ++values) {
			strcpy(*values,i->c_str());
		}
		return true;
	}

};


DB * createMemoryDB() {return new MemoryDB();}

