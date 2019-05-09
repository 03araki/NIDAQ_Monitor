#pragma once

#include "ofMain.h"
#include "ofNIDAQ.h"
#include "ofxUI.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		// ofxUI に関連する変数
		ofxUICanvas *gui;
		ofxUITextInput *guiTextSmplFreq;
		ofxUIToggleMatrix *guiTM;
		ofxUILabelToggle *guiModeBtn;
		ofxUILabelToggle *guiSmplBtn;
		ofxUILabelToggle *guiFullScrBtn;
		ofxUILabelToggle *guiExitBtn;
		ofxUILabelToggle *guiSaveBtn;
		ofxUITextArea  *guiTextArea;
		ofxUILabel *guiParamLabel;
		ofxUITextInput *guiTextParamValue;

		bool bTMStatus;
		void guiEvent(ofxUIEventArgs &e);	// gui のイベント処理関数

		char cSaveFileName[256];

		ofFbo fbo; // frame buffer object

		// ni-DAQ（Analog Input）に関連する変数
		ofNIDAQ niDaq;

		// fft analyzer parameter
		int iMaxFreq;
};
