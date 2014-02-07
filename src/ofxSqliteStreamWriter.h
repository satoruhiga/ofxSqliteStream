#pragma once

#include "ofMain.h"

#include "ofxSqliteStreamConstants.h"
#include "ofxSqliteStreamSession.h"

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

class Writer : public Session
{
public:
	
	typedef ofPtr<Writer> Ptr;
	
	Writer() {}
	virtual ~Writer() { close(); }
	
	bool open(const string& path, bool overwrite = false, bool use_journal = false)
	{
		close();
		
		if (ofFile::doesFileExist(path))
		{
			if (overwrite)
			{
				ofFile::removeFile(path);
			}
			else
			{
				ofLogError("Writer") << "file already exists: " << path;
				return false;
			}
		}
		
		sqlite3_open(ofToDataPath(path).c_str(), &db);
		migrateDB(use_journal);
		
		const char *sql = "insert into Message(time, data) VALUES(?, ?)";
		sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
		
		return true;
	}
	
	void close()
	{
		if (db == NULL) return;
		
		exec("pragma synchronous = ON");
		exec("create index time_index on Message(time)");
		
		sqlite3_finalize(stmt);
		stmt = NULL;
		
		Session::close();
	}
	
	bool addMessage(TimeStamp t, const string &data)
	{
		if (db == NULL)
		{
			ofLogError("Writer") << "open session first";
			return false;
		}
		
		sqlite3_bind_double(stmt, 1, t);
		sqlite3_bind_blob(stmt, 2, data.data(), data.size(), SQLITE_TRANSIENT);
		
		if (SQLITE_DONE != sqlite3_step(stmt))
		{
			ofLogError("Writer") << sqlite3_errmsg(db);
		}
		
		sqlite3_clear_bindings(stmt);
		sqlite3_reset(stmt);
		
		return true;
	}
	
	void clear()
	{
		try
		{
			exec("drop table if exists Message");
		}
		catch (exception &e)
		{
			ofLogError("Writer") << e.what();
			throw;
		}
	}
	
protected:
	
	sqlite3_stmt *stmt;
	
	void migrateDB(bool use_journal)
	{
		clear();
		
		try
		{
			if (!use_journal)
			{
				exec("pragma journal_mode = MEMORY");
				exec("pragma synchronous = OFF");
			}
			
			exec("create table if not exists Message(time double, data blob)");
		}
		catch (exception &e)
		{
			ofLogError("Writer") << e.what();
			throw;
		}
	}
};

OFX_SQLITE_STREAM_END_NAMESPACE