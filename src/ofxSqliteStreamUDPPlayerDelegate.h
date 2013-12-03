#pragma once

#include "ofxSqliteStreamPlayer.h"

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

class UDPPlayerDelegate : public PlayerDelegate
{
public:
	
	typedef ofPtr<UDPPlayerDelegate> Ref;
	
	UDPPlayerDelegate()
	{
		setupSocket();
	}
	
	void addHost(string ip, int port)
	{
		mutex.lock();
		addresses.push_back(Poco::Net::SocketAddress(ip, port));
		mutex.unlock();
	}
	
	void clearHost()
	{
		mutex.lock();
		addresses.clear();
		mutex.unlock();
	}
	
	vector<Poco::Net::SocketAddress> getAddresses() const
	{
		return addresses;
	}
	
	string getLatestMessage()
	{
		mutex.lock();
		string ret = data;
		mutex.unlock();
		return ret;
	}
	
	void onDataReceived(float t, const string& data)
	{
		mutex.lock();
		for (int n = 0; n < addresses.size(); n++)
		{
			sock.sendTo(data.data(), data.size(), addresses[n]);
		}
		this->data = data;
		mutex.unlock();
	}
	
	void setupSocket()
	{
		sock.setSendTimeout(0);
		sock.setBlocking(false);
	}
	
protected:
	
	string data;
	
	ofMutex mutex;
	Poco::Net::DatagramSocket sock;
	vector<Poco::Net::SocketAddress> addresses;

};

OFX_SQLITE_STREAM_END_NAMESPACE