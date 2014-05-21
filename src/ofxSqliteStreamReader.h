#pragma once

#include "ofMain.h"
#include "ofxMSATimer.h"

#include "ofxSqliteStreamConstants.h"
#include "ofxSqliteStreamSession.h"

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

class Reader : public Session
{
public:
	
	typedef ofPtr<Reader> Ptr;
	
	Reader() : last_time_stamp(0) {}
	~Reader() { close(); }
	
	bool open(const string& path)
	{
		close();
		
		if (!ofFile::doesFileExist(path))
		{
			ofLogError("Reader") << "no such file: " << path;
			return false;
		}
		
		sqlite3_open(ofToDataPath(path).c_str(), &db);
		
		if (SQLITE_OK != sqlite3_exec(db, "select time from Message order by time desc limit 1", onSelectLastTimeStamp, this, NULL))
		{
			ofLogError("Reader") << "invalid file: " << path;
			return false;
		}
		
		const char *sql = "select time, data from Message where time >= ? and time < ?";
		if (SQLITE_OK != sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL))
		{
			ofLogError("Reader") << sqlite3_errmsg(db);
			return false;
		}

		return true;
	}
	
	void close()
	{
		last_time_stamp = 0;
		
		if (db)
		{
			sqlite3_finalize(stmt);
			stmt = NULL;
		}
		
		Session::close();
	}
	
	bool getMessages(TimeStamp from, TimeStamp to, vector<pair<TimeStamp, string> > &messages)
	{
		if (!db)
		{
			ofLogError("Reader") << "open session first";
			return false;
		}
		
		messages.clear();
		
		sqlite3_bind_double(stmt, 1, from);
		sqlite3_bind_double(stmt, 2, to);
		
		while (SQLITE_ROW == sqlite3_step(stmt))
		{
			TimeStamp t = sqlite3_column_double(stmt, 0);
			char *ptr = (char*)sqlite3_column_blob(stmt, 1);
			size_t len = sqlite3_column_bytes(stmt, 1);
			messages.push_back(std::pair<TimeStamp, string>(t, string(ptr, len)));
		}
		
		sqlite3_clear_bindings(stmt);
		sqlite3_reset(stmt);
		
		return true;
	}
	
	inline TimeStamp getLastTimeStamp() const { return last_time_stamp; }
	
protected:
	
	sqlite3_stmt *stmt;
	TimeStamp last_time_stamp;
	
	ofxMSATimer timer;
	
	static int onSelectLastTimeStamp(void *arg, int argc, char **argv, char **column)
	{
		if (argc == 0) return SQLITE_ERROR;
		if (string(column[0]) != "time") return SQLITE_ERROR;
		
		Reader *self = (Reader*)arg;
		self->last_time_stamp = ofToFloat(argv[0]);
		
		return SQLITE_OK;
	}
};

OFX_SQLITE_STREAM_END_NAMESPACE