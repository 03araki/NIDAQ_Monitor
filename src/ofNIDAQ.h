#pragma once
#ifndef _OF_NIDAQ_H_
#define _OF_NIDAQ_H_

// NIDAQmx ライブラリ (NIDAQmx のインストール必要)
#include <NIDAQmx.h>
#pragma comment(lib, "NIDAQmx.lib")
#include <Windows.h>

#define DAQmxErrChk(functionCall) if(DAQmxFailed(error=(fanctionCall))) goto Error; else


// OpenFrameworks のライブラリ
#include "ofMain.h"


class ofNIDAQ {
public:
	ofNIDAQ();
	~ofNIDAQ();

	bool chkConnectedDev();		// PC に接続されている DAQ デバイスの確認
	char* getDevName() { return ((m_iDevIndex < 0) ? NULL : m_cDevName); };		// デバイス名の取得
	int getDevAIMaxNum() { return m_iAIMaxNumCh; };		// PC に接続されている DAQ デバイスの Analog Input の最大チャンネル数を取得
	int getCurrentAINumCh() { return m_iAINumCh; };		// 現在の Analog Input の設定チャンネル数を取得
	float getCurrentAIFreq() { return m_fAIFreq; };		// 現在の Analog Input のサンプリング周波数
	int getCurrentDispTime() { return m_uiDispTimeSec; }; // 表示時間を取得
	bool isDataAquisition() { return m_bAqStatus; };	// 計測中であるかどうかを取得
	bool startSaveData(char* saveFilePath);				// ファイルの保存開始
	bool stopSaveData();								// ファイルの保存終了
	bool isDataSaving() { return m_bDataSaveStatus; };		// ファイルを保存中かどうかを取得
	double getCurrentSaveTime() { return m_dTimeSavingData; };	// ファイル保存を開始してから現在までの時刻

	int getDispBufNum() { return m_uiDispBufNum; };		// 表示用データバッファの 1 Ch あたりの数を取得
	float64* getDispBuf() { return m_pdDispBuf; };		// 表示用バッファの取得
	int getCurrentDispBufNum() { return m_iDataCounter; };		// 表示用バッファにおける現在のデータ位置の取得

	bool initAISetting(int iNumCh, float fFreq, int iDispTimeSec );		// Analog Input 設定の初期化
	bool cleanAISetting();		// Analog Input 設定の終了処理

	bool startAISampling();		// Analog Input サンプリングを開始
	bool stopAISampling();		// Analog Input サンプリングを終了

private:
	TaskHandle		m_hTask;			// NI-DAQ を扱うデバイスハンドル
	int				m_iDevIndex;		// PC に接続されている DAQ デバイスのインデックス
	uInt32			m_uiDevID;			// PC に接続されている DAQ デバイスの ID 番号
	char			m_cDevName[20];		// PC に接続されている DAQ デバイスの名称
	int				m_iAIMaxNumCh;		// PC に接続されている DAQ デバイスの Analog Input の最大チャンネル数
	int				m_iTermConf;		// PC に接続されている DAQ デバイスの A/D 変換方式
	int				m_iAINumCh;			// Analog Input のチャンネル数
	float			m_fAIFreq;			// Analog Input のサンプリング周波数
	bool			m_bAqStatus;		// 測定中であるかどうかのフラグ
	FILE*			m_pSaveFile;		// 測定データの保存用ファイルポインタ

	float64*		m_pdAIData;			// 計測したデータ用のバッファ
	double*			m_pdDispBuf;		// 表示用データのバッファ
	unsigned long   m_ulSmplEventNum;	// Analog Input で一回に取得するデータ数
	unsigned int	m_uiDispBufNum;		// 表示用データバッファ数
	unsigned int	m_uiUpdatePeriod;	// 表示用データバッファの更新周期
	unsigned int	m_uiDispTimeSec;	// 表示時間 (sec)

	int				m_iLoopCounter;
	int				m_iDataCounter;

	bool			m_bDataSaveStatus;
	double			m_dTimeSavingData;

	void getAIDevSetting(uInt32 uiDevID, int* piAIMaxNumCh, int* piTermConf);		// デバイスに対する最大チャンネル数と A/D 変換方式の取得

	// 指定数サンプリング毎に呼ばれるコールバック関数
	static int32 CVICALLBACK AIEveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	// タスク終了時に呼ばれるコールバック関数
	static int32 CVICALLBACK AIDoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
};

#endif //_OF_NIDAQ_H_