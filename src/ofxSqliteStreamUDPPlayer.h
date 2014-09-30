#pragma once

#include "ofMain.h"

#include <Poco/Net/DatagramSocket.h>

#include "ofxSqliteStreamPlayer.h"

OFX_SQLITE_STREAM_BEGIN_NAMESPACE

class UDPPlayer : public Player
{
public:
	UDPPlayer()
	{
		setupSocket();
		ofAddListener(dataReceived, this, &UDPPlayer::onDataReceived);
	}
	virtual ~UDPPlayer()
	{
		close();
	}
	void addHost(string ip, int port)
	{
		mutex.lock();
		addresses.push_back(Poco::Net::SocketAddress(ip, port));
		mutex.unlock();
	}
	void clearHosts()
	{
		mutex.lock();
		addresses.clear();
		mutex.unlock();
	}
	vector<Poco::Net::SocketAddress> getAddresses() const
	{
		return addresses;
	}
	void onDataReceived(DataEventArg& e)
	{
		mutex.lock();
		for (int n = 0; n < addresses.size(); n++)
		{
			sock.sendTo(e.data.data(), e.data.size(), addresses[n]);
		}
		mutex.unlock();
	}
	void setupSocket()
	{
		sock.setSendTimeout(0);
		sock.setBlocking(false);
	}

protected:
	ofMutex mutex;
	Poco::Net::DatagramSocket sock;
	vector<Poco::Net::SocketAddress> addresses;
};

OFX_SQLITE_STREAM_END_NAMESPACE