#pragma once

#include "ofMain.h"
#include "ofxSqliteStreamConstants.h"

#include "sqlite3.h"

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

class Session
{
public:
	
	Session() : db(NULL) {}
	virtual ~Session() { close(); }
	
	virtual void close()
	{
		if (db == NULL) return;
		
		sqlite3_close(db);
		db = NULL;
	}
	
	bool isOpend() const { return db; }
	
protected:
	
	sqlite3* db;
	
	void exec(const char* sql)
	{
		if (db == NULL) return;
		
		char *err = NULL;
		sqlite3_exec(db, sql, 0, 0, &err);
		
		if (err != NULL)
		{
			ofLogError("SessionWriter") << err;
			sqlite3_free(err);
		}
	}
};

OFX_SQLITE_STREAM_END_NAMESPACE