#include "ofApp.h"
#include <stdio.h>

#define GUI_MENU_WIDTH	262
//--------------------------------------------------------------
void ofApp::setup(){
	bTMStatus = false;

	// NI-DAQ の存在確認とデバイス名取得
	if (!niDaq.chkConnectedDev()) {		// 接続デバイスの確認
		printf("Device not found.\n");
		ofExit();
	}
	if (niDaq.getDevAIMaxNum() == -1) {		// 接続されているデバイス情報の確認
		printf("Device information not found.\nPlease check and add information to ofNIDAQDev.cpp\n");
		ofExit();
	}
	if (!niDaq.initAISetting(niDaq.getDevAIMaxNum(), niDaq.getCurrentAIFreq(), niDaq.getCurrentDispTime())) {
		printf("init failed.\n");
		ofExit();
	}

	// 初期画面設定
	ofBackground(255, 255, 255);
	ofSetVerticalSync(true);
	ofSetFullscreen(true);

	// GUI の表示と設定
	string str;
	gui = new ofxUICanvas(0, 0, GUI_MENU_WIDTH, ofGetWindowHeight());
	//gui->setColorFill(ofxUIColor(0, 0, 0));		// フォントの色
	gui->addLabel("Data Acquisition System", OFX_UI_FONT_LARGE);
	gui->addLabel("For NI-DAQ Device", OFX_UI_FONT_LARGE);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	str = "Device: " + ofToString(niDaq.getDevName());
	gui->addLabel(str, OFX_UI_FONT_LARGE);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);

	gui->addSpacer();
	gui->addLabel("Analog Input Setting", OFX_UI_FONT_LARGE);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	gui->addLabel("Sampling Frequency [>=200 Hz]", OFX_UI_FONT_MEDIUM);
	char cFreq[20];
	sprintf_s(cFreq, 20, "%.1f", niDaq.getCurrentAIFreq());
	guiTextSmplFreq = gui->addTextInput("SmplFreq", ofToString(cFreq), OFX_UI_FONT_MEDIUM);
	guiTextSmplFreq->setAutoClear(false);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	// 測定チャンネル設定用 Toggle Matrix
	gui->addLabel("Channel Setting");
	str = "";
	for (int i = 0; i < ceil(niDaq.getDevAIMaxNum() / 8.0); i++) {
		if (i != 0) str += " / ";
		str += ofToString(i * 8 + 1) + "-";
		if (niDaq.getDevAIMaxNum() <= 8) str += ofToString(niDaq.getDevAIMaxNum()) + " Ch";
		else str += ofToString((i + 1) * 8) + " Ch";
	}
	gui->addLabel(str, OFX_UI_FONT_SMALL);
	if(niDaq.getDevAIMaxNum() < 8)
		guiTM = gui->addToggleMatrix("ChannelSelector", 1, niDaq.getDevAIMaxNum(), 30, 30);
	else
		guiTM = gui->addToggleMatrix("ChannelSelector", niDaq.getDevAIMaxNum() / 8, 8, 30, 30);
	guiTM->setAllToggles(false);
	for (int i = 0; i < niDaq.getCurrentAINumCh(); i++) {
		guiTM->setToggle(i / 8, i % 8, true);
	
	}
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	gui->addLabel("Window Width [sec]", OFX_UI_FONT_MEDIUM);
	guiTextDispTime = gui->addTextInput("Interval", ofToString(niDaq.getCurrentDispTime()), OFX_UI_FONT_MEDIUM);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	gui->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
	guiSmplBtn = gui->addLabelToggle("Start Data Sampling", niDaq.isDataAquisition());
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	gui->setWidgetFontSize(OFX_UI_FONT_MEDIUM);
	guiSaveBtn = gui->addLabelToggle("Start Data Saving", false);
	guiSaveBtn->setVisible(false);
	guiTextArea = gui->addTextArea("TEXT AREA", "filenale\nsave time", OFX_UI_FONT_MEDIUM);
	guiTextArea->setVisible(false);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	gui->addSpacer();
	gui->addLabel("App Control", OFX_UI_FONT_LARGE);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	guiFullScrBtn = gui->addLabelToggle("Full Screen Mode", true);
	gui->addLabel(" ", OFX_UI_FONT_SMALL);
	guiExitBtn = gui->addLabelToggle("Exit Application", false);
	

	ofAddListener(gui->newGUIEvent, this, &ofApp::guiEvent);	// GUI イベント処理用関数を登録
}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e) {
	char cFreq[20];
	if (e.widget->getName() == "SmplFreq") {	// 周波数設定が変更された場合
		if (niDaq.isDataAquisition()) {
			sprintf_s(cFreq, 20, "%.1f", niDaq.getCurrentAIFreq());
			guiTextSmplFreq->setTextString(ofToString(cFreq));
		}
		else {
			string str = guiTextSmplFreq->getTextString();
			if (atof(str.c_str()) < 200) {
				sprintf_s(cFreq, 20, "%.1f", niDaq.getCurrentAIFreq());
				guiTextSmplFreq->setTextString(ofToString(cFreq));
			}
			else {
				if (!niDaq.initAISetting(niDaq.getCurrentAINumCh(), atof(str.c_str()), niDaq.getCurrentDispTime())) {
					printf("device initialization error.\n");
					ofExit();
				}
				sprintf_s(cFreq, 20, "%.1f", niDaq.getCurrentAIFreq());
				guiTextSmplFreq->setTextString(ofToString(cFreq));
			}
			
		}
	}
	else if (e.widget->getName() == "Start Data Sampling") {	// データサンプリング
		if (guiSmplBtn->getValue()) niDaq.startAISampling();
		else niDaq.stopAISampling();

		if (niDaq.isDataAquisition()) {
			guiSmplBtn->setLabelText("Stop Data Sampling");
		}
		else {
			guiSmplBtn->setLabelText("Start Data Sampling");
		}
		guiSmplBtn->setValue(niDaq.isDataAquisition());
		guiSaveBtn->setVisible(niDaq.isDataAquisition());
	}
	else if (e.widget->getName() == "Start Data Saving") {	// データの保存
		if (guiSaveBtn->getValue()) {
			sprintf_s(cSaveFileName, 256, "%d%02d%02d_%02d%02d%02d.csv", ofGetYear(), ofGetMonth(), ofGetDay(),
				ofGetHours(), ofGetMinutes(), ofGetSeconds());
			string fileStr = ofToString(cSaveFileName);
			niDaq.startSaveData((char *)ofToDataPath(fileStr, true).c_str());
		}
		else {
			niDaq.stopSaveData();
		}
		// ファイル名および測定時間表示エリアの更新
		if(niDaq.isDataSaving()) {
			string str = "File: " + ofToString(cSaveFileName) + "\nSaving Time : 0.0 s";
			guiTextArea->setTextString(str);
			guiTextArea->setVisible(true);
			guiSaveBtn->setLabelText("Stop Data Saving");
		}
		else {
			guiTextArea->setVisible(false);
			guiSaveBtn->setLabelText("Start Data Saving");
		}
	}
	else if (e.widget->getName() == "Interval") {
		char cInterval[20];
		if (niDaq.isDataAquisition()) {
			sprintf_s(cInterval, 20, "%d", niDaq.getCurrentDispTime());
			guiTextDispTime->setTextString(ofToString(cInterval));
		}
		else {
			string str = guiTextDispTime->getTextString();
			if (atoi(str.c_str()) < 2 || atoi(str.c_str()) > 30) {
				sprintf_s(cInterval, 20, "%d", niDaq.getCurrentDispTime());
				guiTextDispTime->setTextString(ofToString(cInterval));
			}
			else {
				if (!niDaq.initAISetting(niDaq.getCurrentAINumCh(), niDaq.getCurrentAIFreq(), atoi(str.c_str()))) {
					printf("device initialization error.\n");
					ofExit();
				}
			}
			sprintf_s(cInterval, 20, "%d", niDaq.getCurrentDispTime());
			guiTextDispTime->setTextString(ofToString(cInterval));
		}
	}
	else if (e.widget->getName() == "Full Screen Mode") {
		ofSetFullscreen(guiFullScrBtn->getValue());
	}
	else if (e.widget->getName() == "Exit Application") {
		if (niDaq.isDataSaving()) niDaq.stopSaveData();
		if (niDaq.isDataAquisition()) niDaq.stopAISampling();
		ofExit();
	}

	else if(!bTMStatus) { // Toggle Matrix が押された時の処理（なぜか e.widget では detect できない ...）
		bTMStatus = true;
		if (niDaq.isDataAquisition()) { // データ測定中の場合は設定を変更しない
			guiTM->setAllToggles(false);
			for (int i = 0; i < niDaq.getCurrentAINumCh(); i++) {
				guiTM->setToggle(i / 8, i % 8, true);
			}
		}
		else {
			int iNewAINumCh = 0;
			// Toggle Matrix の押された場所を取得し，新しい測定チャンネル数を定義
			for (int i = 0; i < niDaq.getCurrentAINumCh(); i++) {
				if (!guiTM->getState(i / 8, i % 8)) {
					iNewAINumCh = i;
					if (iNewAINumCh == 0) iNewAINumCh = 1;
					break;
				}
			}
			if (iNewAINumCh == 0) {
				for (int i = niDaq.getCurrentAINumCh(); i < niDaq.getDevAIMaxNum(); i++) {
					if (guiTM->getState(i / 8, i % 8)) {
						iNewAINumCh = i + 1;
						break;
					}
				}
			}

			if (!niDaq.initAISetting(iNewAINumCh, niDaq.getCurrentAIFreq(), niDaq.getCurrentDispTime())) {
				printf("device initialization error.\n");
				ofExit();
			}
			guiTM->setAllToggles(false);
			for (int i = 0; i < niDaq.getCurrentAINumCh(); i++) {
				guiTM->setToggle(i / 8, i % 8, true);
			}
		}
		bTMStatus = false;
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	if (niDaq.isDataSaving()) {
		char cData[256];
		sprintf_s(cData, 256, "File: %s\nSaving Time: %.1f s", cSaveFileName, niDaq.getCurrentSaveTime());
		guiTextArea->setTextString(ofToString(cData));
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	// データ表示ウィンドウの描画
	int dataWindowWidth = ofGetWindowWidth() - GUI_MENU_WIDTH;
	int dataWindowHeight = ofGetWindowHeight();
	ofTranslate(GUI_MENU_WIDTH, 0);
	ofSetColor(0, 0, 0);
	ofRect(0, 0, dataWindowWidth, dataWindowHeight);

	// 時間のグリッド
	ofSetLineWidth(1);
	ofSetColor(80, 80, 80);
	for (int i = 1; i < niDaq.getCurrentDispTime(); i++)
		ofLine(i*(dataWindowWidth / (double)niDaq.getCurrentDispTime()), 0,
			i*(dataWindowWidth / (double)niDaq.getCurrentDispTime()), dataWindowHeight);
	// 信号のベースライン (0 V ライン)
	float fDataHeight = dataWindowHeight / (double)niDaq.getCurrentAINumCh();
	float fCenter = fDataHeight / 2.0;
	for (int i = 0; i < niDaq.getCurrentAINumCh(); i++)
		ofLine(0, i * fDataHeight + fCenter, dataWindowWidth, i * fDataHeight + fCenter);
	// チャンネルごとのグリッド
	ofSetLineWidth(2);
	ofSetColor(150, 150, 150);
	for (int i = 1; i < niDaq.getCurrentAINumCh(); i++) {
		ofLine(0, i*(dataWindowHeight / (double)niDaq.getCurrentAINumCh()),
			dataWindowWidth, i*(dataWindowHeight / (double)niDaq.getCurrentAINumCh()));
	}

	// チャンネルごとのデータの描画
	float64* DispBuf = niDaq.getDispBuf();
	for (int i = 0; i < niDaq.getCurrentAINumCh(); i++) {
		// データライン
		switch (i) {
		case 0: ofSetColor(255, 40, 0);	break;	// 赤
		case 1: ofSetColor(255, 245, 0); break;		// 黄
		case 2: ofSetColor(53, 161, 107); break;	// 緑
		case 3: ofSetColor(0, 65, 255);	break;	// 青
		case 4: ofSetColor(102, 204, 255); break; // 空色
		case 5: ofSetColor(255, 153, 160); break;	// ピンク
		case 6: ofSetColor(255, 153, 0); break;		// オレンジ
		case 7: ofSetColor(154, 0, 121); break;		// 紫
		case 8: ofSetColor(102, 51, 0);	break;	// 茶
		case 9: ofSetColor(255, 209, 209); break;	// 明るいピンク
		case 10: ofSetColor(255, 255, 153); break;	// クリーム
		case 11: ofSetColor(203, 242, 102); break;	// 明るい黄緑
		case 12: ofSetColor(180, 235, 250); break;	// 明るい空色
		case 13: ofSetColor(237, 197, 143); break;	// ベージュ
		case 14: ofSetColor(135, 231, 176); break;	// 明るい緑
		case 15: ofSetColor(199, 178, 222);	break; // 明るい紫
		}
		
		if (niDaq.isDataAquisition()) {
			ofNoFill();
			ofSetLineWidth(2);
			ofBeginShape();
			for (int j = 0; j < niDaq.getDispBufNum(); j++) {
				ofVertex(dataWindowWidth * (j / (double)(niDaq.getDispBufNum() - 1)),
					((-1 * DispBuf[j * niDaq.getCurrentAINumCh() + i] + 10) / 20.0) * fDataHeight);
			}
			ofEndShape();
			ofFill();
		}
		ofDrawBitmapString(ofToString(i + 1) + " Ch", 10, 20);
		ofTranslate(0, fDataHeight);
	}
	ofTranslate(0, -dataWindowHeight);

	// トレースラインの表示
	if (niDaq.isDataAquisition()) {
		ofSetColor(200, 200, 200);
		ofSetLineWidth(4);
		ofLine(dataWindowWidth * ((niDaq.getCurrentDispBufNum() - 1) / (double)(niDaq.getDispBufNum() - 1)), 0,
			dataWindowWidth * ((niDaq.getCurrentDispBufNum() - 1) / (double)(niDaq.getDispBufNum() - 1)), dataWindowHeight);
	}
	
	ofTranslate(-GUI_MENU_WIDTH, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
