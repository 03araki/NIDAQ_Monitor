#include "ofNIDAQ.h"

ofNIDAQ::ofNIDAQ()
{
	m_hTask = NULL;
	m_iDevIndex = -1;
	m_uiDevID = 0;
	m_bAqStatus = false;
	m_iAIFreq = 1000;

	m_pdAIData = NULL;
	m_pdDispBuf = NULL;
	m_uiDispTimeSec = 10;
	m_uiUpdatePeriod = 1;

	m_pSaveFile = NULL;
	m_bDataSaveStatus = false;
	m_dTimeSavingData = 0;

	for (int i = 0; i < MAX_AI_NUM; i++) {
		m_srcData[i] = NULL;
		m_FFTData[i] = NULL;
	}
	m_bDFTAnalysis = true;
	m_iFFTWDataCounter = 0;
	m_fOverLap = 0.8;
}

ofNIDAQ::~ofNIDAQ()
{
	if (m_bDataSaveStatus) stopSaveData();
	if (m_bAqStatus) stopAISampling();

	cleanAISetting();

	delete[] m_pdDispBuf;
	delete[] m_pdAIData;

}

// PC に接続されている DAQ デバイスの確認
bool ofNIDAQ::chkConnectedDev()
{
	if (DAQmxFailed(DAQmxCreateTask("", &m_hTask))) return false;

	char cDevStr[30];
	for (int i = 1; i < 10; i++) {
		sprintf_s(cDevStr, 30, "Dev%d/ai0", i);	// 接続確認用の物理チャンネル (AI0) を指定
		if (DAQmxFailed(DAQmxCreateAIVoltageChan(m_hTask, cDevStr, "", DAQmx_Val_RSE, -10.0, 10.0, DAQmx_Val_Volts, NULL))) {
			continue;
		}
		else { // 接続されているデバイスを発見した場合，デバイスのインデックスと名称を保存
			m_iDevIndex = i;
			sprintf_s(cDevStr, 30, "Dev%d", m_iDevIndex);
			DAQmxGetDevProductType(cDevStr, m_cDevName, 20);	// デバイスの名称取得
			DAQmxGetDevProductNum(cDevStr, &m_uiDevID);			// デバイスの ID を取得
			getAIDevSetting(m_uiDevID, &m_iAIMaxNumCh, &m_iTermConf);		// デバイス ID から最大測定チャンネル数と変換方式を取得
			DAQmxStopTask(m_hTask);
			DAQmxClearTask(m_hTask);
			m_hTask = NULL;
			return true;
		}
		
	}
	return false;
}

bool ofNIDAQ::startSaveData(char * saveFilePath)
{
	if (!m_bAqStatus) return false;
	if (m_pSaveFile != NULL) return true;

	if (NULL == (m_pSaveFile = fopen(saveFilePath, "w"))) return false;
	fprintf(m_pSaveFile, "time");
	for (int i = 0; i < m_iAINumCh; i++) fprintf(m_pSaveFile, ",AI%d", i);
	fprintf(m_pSaveFile, "\n");
	m_dTimeSavingData = 0;
	m_bDataSaveStatus = true;
	return true;
}

bool ofNIDAQ::stopSaveData()
{
	if (!m_bAqStatus) return false;
	if (m_pSaveFile == NULL) return true;

	m_bDataSaveStatus = false;
	return true;
}

// Analog Input 設定の初期化
bool ofNIDAQ::initAISetting(int iNumCh, int iFreq, int iDispTimeSec)
{
	if (m_iDevIndex < 0) {	// 接続されている DAQ デバイスの確認が終わっていない場合
		if (!chkConnectedDev()) return false;
	}

	if (m_bAqStatus) return false;	// 測定中の場合は変更しない

	cleanAISetting();

	delete[] m_pdAIData; m_pdAIData = NULL;
	delete[] m_pdDispBuf; m_pdDispBuf = NULL;

	if (DAQmxFailed(DAQmxCreateTask("", &m_hTask))) return false;
	// 物理チャンネルの設定
	if (iNumCh <= 0 || iNumCh >= m_iAIMaxNumCh + 1) return false;
	char cPhysicalChan[40];
	if (m_iTermConf == DAQmx_Val_RSE) {
		sprintf_s(cPhysicalChan, 40, "Dev%d/ai0:%d", m_iDevIndex, iNumCh - 1);
	}
	else if (m_iTermConf == DAQmx_Val_Diff) {
		if (iNumCh <= 8) sprintf_s(cPhysicalChan, 40, "Dev%d/ai0:%d", m_iDevIndex, iNumCh - 1);
		else sprintf_s(cPhysicalChan, 40, "Dev%d/ai0:7, Dev%d/ai16:%d", m_iDevIndex, m_iDevIndex, (iNumCh - 9) + 16);
	}
	//printf("%s\n", cPhysicalChan);

	// Analog Input の初期化
	if (DAQmxFailed(DAQmxCreateAIVoltageChan(m_hTask, cPhysicalChan, "", m_iTermConf, -10.0, 10.0, DAQmx_Val_Volts, NULL))) return false;
	// サンプリングタイミング設定
	m_ulSmplEventNum = (unsigned long) floor(iFreq / 200.0); // 一度に取得するデータ数
	if (DAQmxFailed(DAQmxCfgSampClkTiming(m_hTask, "", iFreq, DAQmx_Val_Rising, DAQmx_Val_ContSamps, m_ulSmplEventNum))) return false;
	// 指定サンプリング回数毎に呼ばれるコールバック関数の登録
	if (DAQmxFailed(DAQmxRegisterEveryNSamplesEvent(m_hTask, DAQmx_Val_Acquired_Into_Buffer, m_ulSmplEventNum, 0, AIEveryNCallback, (void *)this))) return false;
	// タスク終了時に呼び出されるコールバック関数の登録
	if (DAQmxFailed(DAQmxRegisterDoneEvent(m_hTask, 0, AIDoneCallback, (void*) this))) return false;

	m_iAINumCh = iNumCh;
	m_iAIFreq = iFreq;
	m_uiDispTimeSec = iDispTimeSec;

	// データバッファ領域の確保
	m_pdAIData = new float64[m_iAINumCh * m_ulSmplEventNum];

	m_uiDispBufNum = m_uiDispTimeSec * (int)round(m_iAIFreq / (double)m_ulSmplEventNum);		// ディスプレイ用バッファの数
	m_uiUpdatePeriod = (int) ceil(m_uiDispBufNum / 2000.0);		// データの更新周期
	m_uiDispBufNum = m_uiDispBufNum / m_uiUpdatePeriod;			// 更新周期によりバッファ数を補正
	//printf("%d, %d\n", m_uiDispBufNum, m_uiUpdatePeriod);
	m_pdDispBuf = new double[m_uiDispBufNum * m_iAINumCh];

	// Data Buffer for FFT
	for (int i = 0; i < MAX_AI_NUM; i++) {
		// allocate and initialize temporal data buffer
		delete[] m_srcData[i];
		m_srcData[i] = new double[m_iAIFreq];
		for (int j = 0; j < m_iAIFreq; j++) m_srcData[i][j] = 0;
		// allocate and initialize FFT result data buffer
		delete[] m_FFTData[i];
		m_iNumFFTData = (int)floor(m_iAIFreq / 2.0) + 1;
		m_FFTData[i] = new double[m_iNumFFTData];
		for (int j = 0; j < m_iNumFFTData; j++) m_FFTData[i][j] = 0;

		// allocate and initialize data buffer for FFTW
		fftw_free(m_srcFFTWData[i]);
		fftw_free(m_dstFFTWData[i]);
		m_srcFFTWData[i] = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * m_iAIFreq);
		m_dstFFTWData[i] = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * m_iAIFreq);
		fftPlan[i] = fftw_plan_dft_1d(m_iAIFreq, m_srcFFTWData[i], m_dstFFTWData[i], FFTW_FORWARD, FFTW_ESTIMATE);
		for (int j = 0; j < m_iAIFreq; j++) {
			m_srcFFTWData[i][j][0] = 0;	// real part
			m_srcFFTWData[i][j][1] = 0;	// imaginary part
		}
	}
	

	return true;
}

// Analog Input 設定の終了処理
bool ofNIDAQ::cleanAISetting() {
	if (m_hTask == NULL) return true;

	if (m_bAqStatus) stopAISampling();

	DAQmxStopTask(m_hTask);
	DAQmxClearTask(m_hTask);
	m_hTask = NULL;

	return true;
}

bool ofNIDAQ::startAISampling()
{
	if (m_hTask == NULL) return false;
	if (m_bAqStatus) return true;

	for (int i = 0; i < m_uiDispBufNum * m_iAINumCh; i++) m_pdDispBuf[i] = 0;
	m_iLoopCounter = 0;
	m_iDataCounter = 0;
	if (DAQmxFailed(DAQmxStartTask(m_hTask))) return false;
	m_bAqStatus = true;

	return true;
}

bool ofNIDAQ::stopAISampling()
{
	if (m_hTask == NULL) return false;
	if (!m_bAqStatus) return true;
	
	m_bAqStatus = false;
	DAQmxStopTask(m_hTask);

	return true;
}



// 指定サンプリング回数毎に呼ばれるコールバック関数
int32 CVICALLBACK ofNIDAQ::AIEveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData) {

	ofNIDAQ* niDaq = (ofNIDAQ *)callbackData;
	int32       read = 0;

	float fAITimeout = niDaq->m_ulSmplEventNum * (1.0 / niDaq->m_iAIFreq) * 2;
	// データの取得
	if (DAQmxFailed(DAQmxReadAnalogF64(taskHandle, niDaq->m_ulSmplEventNum, fAITimeout,
		DAQmx_Val_GroupByScanNumber, niDaq->m_pdAIData, niDaq->m_ulSmplEventNum * niDaq->m_iAINumCh, &read, NULL))) return false;

	// データを保存する場合
	if (niDaq->m_bDataSaveStatus) {
		for (int i = 0; i < niDaq->m_ulSmplEventNum; i++) {
			fprintf(niDaq->m_pSaveFile, "%lf", niDaq->m_dTimeSavingData);
			for (int j = 0; j < niDaq->m_iAINumCh; j++)
				fprintf(niDaq->m_pSaveFile, ",%lf", niDaq->m_pdAIData[i * niDaq->m_iAINumCh + j]);
			fprintf(niDaq->m_pSaveFile, "\n");
			niDaq->m_dTimeSavingData += (1.0 / niDaq->m_iAIFreq);
		}
	}

	// データの保存を終了する場合
	if (!niDaq->m_bDataSaveStatus && niDaq->m_pSaveFile != NULL) {
		fclose(niDaq->m_pSaveFile);
		niDaq->m_pSaveFile = NULL;
	}

	// 表示用データのアップデートタイミングを計算
	niDaq->m_iLoopCounter = (niDaq->m_iLoopCounter + 1) % niDaq->m_uiUpdatePeriod;	
	if (niDaq->m_iLoopCounter == 0) {
		for (int i = 0; i < niDaq->m_iAINumCh; i++)
			niDaq->m_pdDispBuf[niDaq->m_iDataCounter * niDaq->m_iAINumCh + i]
				= niDaq->m_pdAIData[(niDaq->m_ulSmplEventNum - 1)*niDaq->m_iAINumCh + i];
		niDaq->m_iDataCounter = (niDaq->m_iDataCounter + 1) % niDaq->m_uiDispBufNum;
	}

	if (niDaq->m_bDFTAnalysis) {
		// store data for DFT
		for (int i = 0; i < niDaq->m_iAINumCh; i++) {
			memcpy(niDaq->m_srcData[i], niDaq->m_srcData[i] + niDaq->m_ulSmplEventNum, 
				sizeof(double)*(niDaq->m_iAIFreq - niDaq->m_ulSmplEventNum));
			for (int j = 0; j < niDaq->m_ulSmplEventNum; j++) {
				niDaq->m_srcData[i][niDaq->m_iAIFreq - niDaq->m_ulSmplEventNum + j]
					= niDaq->m_pdAIData[j * niDaq->m_iAINumCh + i];
			}
		}
		niDaq->m_iFFTWDataCounter += niDaq->m_ulSmplEventNum;

		// calculate DFT
		if (niDaq->m_iFFTWDataCounter >= (int)ceil(niDaq->m_iAIFreq * (1 - niDaq->m_fOverLap))) {
			for (int i = 0; i < niDaq->m_iAINumCh; i++) {
				// calculate signal mean due to detrend
				double signalMean = 0;
				for (int j = 0; j < niDaq->m_iAIFreq; j++) signalMean += niDaq->m_srcData[i][j];
				// copy data to FFTW buffer with detrend
				for (int j = 0; j < niDaq->m_iAIFreq; j++)
					niDaq->m_srcFFTWData[i][j][0] = niDaq->m_srcData[i][j] - (signalMean / (double)niDaq->m_iAIFreq);
				fftw_execute(niDaq->fftPlan[i]);	// FFT calculation using FFTW
				double specMax = -1;
				for (int j = 0; j < niDaq->m_iNumFFTData; j++) { // calculate spectrum
					niDaq->m_FFTData[i][j] = (2.0 / niDaq->m_iAIFreq) *
						sqrt(niDaq->m_dstFFTWData[i][j][0] * niDaq->m_dstFFTWData[i][j][0] + niDaq->m_dstFFTWData[i][j][1] * niDaq->m_dstFFTWData[i][j][1]);
					if (specMax < niDaq->m_FFTData[i][j]) specMax = niDaq->m_FFTData[i][j];
				}
				// normalize obtained spectrum
				for (int j = 0; j < niDaq->m_iNumFFTData; j++) niDaq->m_FFTData[i][j] /= specMax;
			}
		}
	}

	return 0;
}


// タスク終了時に呼ばれるコールバック関数
int32 CVICALLBACK ofNIDAQ::AIDoneCallback(TaskHandle taskHandle, int32 status, void *callbackData) {

	return 0;
}