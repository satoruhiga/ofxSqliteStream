#pragma once

#include "ofMain.h"

#include "ofxSqliteStreamConstants.h"
#include "ofxSqliteStreamSession.h"

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

class Writer : public Session
{
public:
	
	typedef ofPtr<Writer> Ptr;
	typedef float TimeStamp;
	
	Writer() : is_recording(false), start_time(-1) {}
	virtual ~Writer() { close(); }
	
	bool open(const string& path, bool overwrite = false, bool use_journal = false)
	{
		close();
		
		if (overwrite)
		{
			ofFile::removeFile(path);
		}
		else if (ofFile::doesFileExist(path))
		{
			ofLogError("Writer") << "file already exists: " << path;
			return false;
		}
		
		sqlite3_open(ofToDataPath(path).c_str(), &db);
		migrateDB(use_journal);
		
		const char *sql = "insert into Message(time, data) VALUES(?, ?)";
		sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
		
		return true;
	}
	
	void close()
	{
		if (is_recording)
		{
			is_recording = false;
			start_time = -1;
		}
		
		if (db)
		{
			exec("pragma synchronous = ON");
			exec("create index time_index on Message(time)");
			
			sqlite3_finalize(stmt);
			stmt = NULL;
		}
		
		Session::close();
	}
	
	void start()
	{
		if (!db)
		{
			ofLogError("Writer") << "open session first";
			throw;
		}
		
		is_recording = true;
	}
	
	void stop()
	{
		close();
	}
	
	bool addMessage(TimeStamp t, const string &data)
	{
		if (!db)
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
	
	bool isRecording() const { return is_recording; }
	
protected:
	
	sqlite3_stmt *stmt;
	
	bool is_recording;
	TimeStamp start_time;
	
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