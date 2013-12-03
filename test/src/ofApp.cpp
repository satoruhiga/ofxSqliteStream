#include "ofMain.h"

#include "ofxSqliteStream.h"
#include "test.h"

class ofApp : public ofBaseApp
{
public:
	
	ofxSqliteStream::Player player;
	ofxSqliteStream::UDPPlayerDelegate::Ref player_delegate;
	
	Poco::Net::DatagramSocket sock;
	
	void setup()
	{
		ofSetFrameRate(60);
		ofSetVerticalSync(true);
		ofBackground(0);
		
		test();
		
		player_delegate = ofxSqliteStream::UDPPlayerDelegate::Ref(new ofxSqliteStream::UDPPlayerDelegate);
		player_delegate->addHost("localhost", 20202);
		player.setLoopState(true);
		player.setDelegate(player_delegate);
		
		player.open(test_file_name);
		player.start();
		
		sock.setReceiveTimeout(0);
		sock.setBlocking(false);
		sock.bind(Poco::Net::SocketAddress("0.0.0.0", 20202));
	}
	
	void update()
	{
		int length = 256;
		char buffer[length];
		
		try
		{
			int n = sock.receiveBytes(buffer, length);
			if (n > 0)
			{
				cout << string(buffer, n) << endl;
			}
		}
		catch (...) {}
		
		player.setPlaybackRatio(ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 5));
	}
	
	void draw()
	{
		ofRect(0, 0, ofGetWidth() * (player.getPosition() / player.getDuration()), 10);
	}

	void keyPressed(int key)
	{
	}

	void keyReleased(int key)
	{
	}
	
	void mouseMoved(int x, int y)
	{
	}

	void mouseDragged(int x, int y, int button)
	{
	}

	void mousePressed(int x, int y, int button)
	{
	}

	void mouseReleased(int x, int y, int button)
	{
	}
	
	void windowResized(int w, int h)
	{
	}
};


int main(int argc, const char** argv)
{
	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp);
	return 0;
}
