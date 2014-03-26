#pragma once

#include "ofMain.h"

#include "ofxMSATimer.h"

#include "ofxSqliteStreamConstants.h"
#include "ofxSqliteStreamReader.h"

#include <Poco/Net/DatagramSocket.h>

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

struct PlayerDelegate
{
	typedef ofPtr<PlayerDelegate> Ref;
	virtual ~PlayerDelegate() {}
	virtual void onDataReceived(float t, const string& data) = 0;
};

class Player : protected ofThread
{
public:
	
	Player() : fps(120), is_playing(false), loop(false), playback_ratio(1)
	{
		setPosition(0);
	}
	
	~Player() { close(); }
	
	bool open(const string& path)
	{
		close();
		
		lock();
		{
			session = Reader::Ptr(new Reader);
			if (!session->open(path))
			{
				session.reset();
				unlock();
				return false;
			}
			
			setPosition(0);
		}
		unlock();
		
		return true;
	}
	
	void close()
	{
		stop();
	}
	
	void start()
	{
		is_playing = true;
		startThread();
	}
	
	void stop()
	{
		waitForThread();
		is_playing = false;
	}
	
	void setPosition(int sec)
	{
		current_tick = sec;
		last_tick = sec;
	}
	
	TimeStamp getPosition() const { return current_tick; }
	TimeStamp getDuration()
	{
		TimeStamp result = 0;
		
		lock();
		{
			if (session)
				result = session->getLastTimeStamp();
		}
		unlock();
		
		return result;
	}
	
	//

	bool isOpend() const { return session; }
	bool isPlaying() const { return is_playing; }

	float getPlaybackFps() const { return playback_fps; }
	int getMessagePerFrame() const { return message_per_frame; }
	
	void setLoopState(bool yn) { loop = yn; }
	bool getLoopState() const { return loop; }
	
	void setPlaybackRatio(float v) { playback_ratio = v; if (playback_ratio < 0) playback_ratio = 0; }
	float getPlaybackRatio() const { return playback_ratio; }
	
	void setDelegate(PlayerDelegate::Ref delegate)
	{
		this->delegate = delegate;
	}
	
protected:
	
	bool loop;
	bool is_playing;
	Reader::Ptr session;
	
	ofxMSATimer timer;
	TimeStamp current_tick;
	TimeStamp last_tick;
	
	float fps;
	float playback_fps;
	int message_per_frame;
	
	float playback_ratio;
	
	PlayerDelegate::Ref delegate;
	
	void threadedFunction()
	{
		while (isThreadRunning())
		{
			lock();
			
			TimeStamp inv_fps = 1. / fps;
			
			if (!session)
			{
				sleep(inv_fps * 1000);
				unlock();
				continue;
			}
			
			TimeStamp t = timer.getAppTimeSeconds();
			vector<pair<TimeStamp, string> > records;
			
			session->getMessages(last_tick, current_tick, records);
			
			if (delegate)
			{
				for (int i = 0; i < records.size(); i++)
				{
					TimeStamp t = records[i].first;
					const string &data = records[i].second;
					delegate->onDataReceived(t, data);
				}
			}
			
			message_per_frame = records.size();
			
			TimeStamp tt = timer.getAppTimeSeconds();
			TimeStamp delta = tt - t;
			
			TimeStamp sleep_time = (inv_fps - delta);
			if (sleep_time > 0) sleep(sleep_time * 1000);
			
			TimeStamp spend = timer.getSecondsSinceLastCall() * playback_ratio;
			
			playback_fps = 1. / spend;
			
			// break if finished
			if (current_tick >= session->getLastTimeStamp())
			{
				if (loop)
				{
					setPosition(0);
				}
				else
				{
					break;
				}
			}

			last_tick = current_tick;
			current_tick += spend;

			unlock();
		}
		
		is_playing = false;
	}
};

OFX_SQLITE_STREAM_END_NAMESPACE
