#ifndef __db_hh__
#define __db_hh__

class DB {
public:
	virtual void describeTabel(const char * name, const char ** keys, const char ** values) = 0;
	virtual void update(const char * tbl, const char ** keys, const char ** values) = 0;
	virtual bool fetch(const char * tbl, const char ** keys, char ** values) = 0;
	virtual ~DB() {};
};

DB * createMemoryDB();
DB * createMysqlDB();
#endif //__db_hh__
