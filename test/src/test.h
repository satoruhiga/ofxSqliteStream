#pragma once

#include "ofMain.h"

#include "ofxSqliteStreamReader.h"
#include "ofxSqliteStreamWriter.h"

using namespace ofxSqliteStream;

string test_file_name = "test.sqlite";

void open_close()
{
	Writer writer;
	ofFile::removeFile(test_file_name);
	
	assert(writer.open(test_file_name));
	assert(writer.isRecording() == false);
	assert(writer.isOpend() == true);
	
	writer.close();
	
	assert(writer.isOpend() == false);
	
	assert(!writer.open(test_file_name, false, false));
	assert(writer.isOpend() == false);
	
	
	assert(writer.open(test_file_name, true, false));
	assert(writer.isOpend() == true);
	
	cout << "success: " << __PRETTY_FUNCTION__ << endl;
}

void writing()
{
	Writer writer;
	ofFile::removeFile(test_file_name);
	assert(writer.open(test_file_name));
	
	writer.start();
	
	assert(writer.isRecording() == true);
	assert(writer.addMessage(0, "test") == true);
	
	writer.stop();
	
	assert(writer.addMessage(0, "test") == false);
	
	writer.close();
	
	cout << "success: " << __PRETTY_FUNCTION__ << endl;
}

void read_write()
{
	{
		Writer writer;
		ofFile::removeFile(test_file_name);
		assert(writer.open(test_file_name));

		writer.start();
		ofSleepMillis(100);
		assert(writer.addMessage(42, "test") == true);
		writer.close();
	}
	
	{
		Reader reader;
		assert(reader.open(test_file_name));
		assert(reader.isOpend());
		
		vector<pair<float, string> > messages;
		if (reader.getMessages(0, 1000, messages))
		{
			assert(messages.size() == 1);
			assert(reader.getLastTimeStamp() == 42);
			
			assert(messages[0].second == "test");
		}
		
		reader.close();
		
		assert(reader.getLastTimeStamp() == 0);
		
		assert(reader.isOpend() == false);
	}

	cout << "success: " << __PRETTY_FUNCTION__ << endl;
}

#include "ofxSqliteStreamUDPRecorder.h"

class TestUDPRecorderDelegate : public ofxSqliteStream::UDPRecorderDelegate
{
public:
	
	typedef ofPtr<TestUDPRecorderDelegate> Ref;
	
	string recirved_message;
	
	void onDataReceived(const string& data)
	{
		recirved_message = data;
	}
};

void stream_recorder()
{
	ofFile::removeFile(test_file_name);

	string payload = "This is a test";
	
	for (int i = 0; i < 512; i++)
	{
		payload += "This is a test";
	}
	
	TestUDPRecorderDelegate::Ref delegate = TestUDPRecorderDelegate::Ref(new TestUDPRecorderDelegate);

	UDPRecorder recorder;
	
	{
		recorder.setup(20202);
		assert(recorder.isRecording() == false);

		recorder.setDelegate(delegate);
		
		assert(recorder.start(test_file_name, false, true));
		ofSleepMillis(100);
		
		Poco::Net::DatagramSocket sock;
		
		sock.sendTo(payload.data(), payload.size(), Poco::Net::SocketAddress("127.0.0.1", "20202"));
		
		assert(recorder.isRecording() == true);
		assert(recorder.getSessionFilename() == test_file_name);
		
		ofSleepMillis(100);
	}
	
	recorder.close();
	
	assert(recorder.isRecording() == false);
	assert(delegate->recirved_message == payload);
	
	{
		Reader reader;
		reader.open(test_file_name);
		
		vector<pair<float, string> > messages;
		reader.getMessages(0, 1000, messages);
		
		assert(messages.size() == 1);
		assert(messages[0].second == payload);
	}
	
	cout << "success: " << __PRETTY_FUNCTION__ << endl;
}

#include "ofxSqliteStreamUDPPlayerDelegate.h"

void stream_player()
{
	Player player;
	
	assert(player.isOpend() == false);
	assert(player.isPlaying() == false);
	
	assert(player.getPlaybackRatio() == 1);
	assert(player.getLoopState() == false);
	
	{
		ofFile::removeFile(test_file_name);
		
		Writer w;
		w.open(test_file_name);
		
		w.addMessage(0.3, "This");
		w.addMessage(0.4, "is");
		w.addMessage(0.5, "a");
		w.addMessage(0.6, "test");
		
		w.close();
	}
	
	UDPPlayerDelegate::Ref delegate = UDPPlayerDelegate::Ref(new UDPPlayerDelegate);
	
	delegate->addHost("localhost", 20202);
	
	player.setDelegate(delegate);
	
	player.open(test_file_name);
	
	ofxMSATimer timer;
	timer.setStartTime();
	
	int count = 0;
	Poco::Net::DatagramSocket sock;
	sock.bind(Poco::Net::SocketAddress("0.0.0.0", 20202), true);
	sock.setReceiveTimeout(1000);
	sock.setBlocking(false);
	
	player.start();
	
	while (timer.getElapsedSeconds() < 1)
	{
		int length = 256;
		char buffer[length];
		
		try
		{
			int n = sock.receiveBytes(buffer, length);
			if (n > 0)
			{
				cout << string(buffer, n) << endl;
				count++;
				
				if (count == 4) break;
			}
		}
		catch(...) {}
		
		ofSleepMillis(30);
	}
	
	assert(count == 4);
	
	cout << "success: " << __PRETTY_FUNCTION__ << endl;
}

void test()
{
	open_close();
	writing();
	read_write();
	
	stream_recorder();
	stream_player();
}