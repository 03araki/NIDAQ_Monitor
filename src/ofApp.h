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

		// ofxUI �Ɋ֘A����ϐ�
		ofxUICanvas *gui;
		ofxUITextInput *guiTextSmplFreq;
		ofxUIToggleMatrix *guiTM;
		ofxUILabelToggle *guiSmplBtn;
		ofxUILabelToggle *guiFullScrBtn;
		ofxUILabelToggle *guiExitBtn;
		ofxUILabelToggle *guiSaveBtn;
		ofxUITextArea  *guiTextArea;
		ofxUITextInput *guiTextDispTime;

		bool bTMStatus;
		void guiEvent(ofxUIEventArgs &e);	// gui �̃C�x���g�����֐�

		char cSaveFileName[256];

		// ni-DAQ�iAnalog Input�j�Ɋ֘A����ϐ�
		ofNIDAQ niDaq;
};