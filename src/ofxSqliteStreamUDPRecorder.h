#pragma once

#include "ofMain.h"

#include <Poco/Net/DatagramSocket.h>

#include "ofxSqliteStreamWriter.h"

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

class UDPRecorderDelegate
{
public:
	typedef ofPtr<UDPRecorderDelegate> Ref;
	virtual ~UDPRecorderDelegate() {}
	virtual void onDataReceived(const string& data) = 0;
};

class UDPRecorder : protected ofThread
{
public:
	
	virtual ~UDPRecorder()
	{
		close();
	}
	
	void setup(int udp_in_port)
	{
		close();
		sock.bind(Poco::Net::SocketAddress("0.0.0.0", udp_in_port), true);
		sock.setReceiveTimeout(0);
		startThread(false);
	}
	
	void close()
	{
		stop();
		
		sock.close();
		waitForThread(true);
	}
	
	bool start(const string& session_filename, bool overwrite = false, bool use_journal = false)
	{
		bool started = false;
		
		this->session_filename = session_filename;
		lock();
		{
			session = Writer::Ptr(new Writer);
			if (session->open(session_filename, overwrite, use_journal))
			{
				timer.setStartTime();
				started = true;
			}
		}
		unlock();
		
		return started;
	}

	void stop()
	{
		session_filename = "";
		
		lock();
		{
			session.reset();
		}
		unlock();
	}
	
	bool isRecording() const { return session; }

	//
	
	string getSessionFilename() const
	{
		return session_filename;
	}
	
	void setDelegate(UDPRecorderDelegate::Ref delegate)
	{
		this->delegate = delegate;
	}

protected:
	
	Writer::Ptr session;
	Poco::Net::DatagramSocket sock;
	
	string session_filename;
	
	ofxMSATimer timer;

	UDPRecorderDelegate::Ref delegate;

	void threadedFunction()
	{
		while (isThreadRunning())
		{
			const size_t length = 65507;
			char buf[length];
			string data;

			int n = sock.receiveBytes(buf, length);
			data.append(buf, n);
			
			if (data.size())
			{
				lock();
				{
					if (session)
						session->addMessage(timer.getElapsedSeconds(), data);
				}
				unlock();
				
				if (delegate)
				{
					delegate->onDataReceived(data);
				}
			}
		}
	}
};

OFX_SQLITE_STREAM_END_NAMESPACE
