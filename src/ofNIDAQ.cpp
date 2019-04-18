#include "ofNIDAQ.h"

ofNIDAQ::ofNIDAQ()
{
	m_hTask = NULL;
	m_iDevIndex = -1;
	m_uiDevID = 0;
	m_bAqStatus = false;
	m_fAIFreq = 1000;

	m_pdAIData = NULL;
	m_pdDispBuf = NULL;
	m_uiDispTimeSec = 10;
	m_uiUpdatePeriod = 1;

	m_pSaveFile = NULL;
	m_bDataSaveStatus = false;
	m_dTimeSavingData = 0;
}

ofNIDAQ::~ofNIDAQ()
{
	if (m_bDataSaveStatus) stopSaveData();
	if (m_bAqStatus) stopAISampling();

	cleanAISetting();

	delete[] m_pdDispBuf;
	delete[] m_pdAIData;

}

// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�̊m�F
bool ofNIDAQ::chkConnectedDev()
{
	if (DAQmxFailed(DAQmxCreateTask("", &m_hTask))) return false;

	char cDevStr[30];
	for (int i = 1; i < 10; i++) {
		sprintf_s(cDevStr, 30, "Dev%d/ai0", i);	// �ڑ��m�F�p�̕����`�����l�� (AI0) ���w��
		if (DAQmxFailed(DAQmxCreateAIVoltageChan(m_hTask, cDevStr, "", DAQmx_Val_RSE, -10.0, 10.0, DAQmx_Val_Volts, NULL))) {
			continue;
		}
		else { // �ڑ�����Ă���f�o�C�X�𔭌������ꍇ�C�f�o�C�X�̃C���f�b�N�X�Ɩ��̂�ۑ�
			m_iDevIndex = i;
			sprintf_s(cDevStr, 30, "Dev%d", m_iDevIndex);
			DAQmxGetDevProductType(cDevStr, m_cDevName, 20);	// �f�o�C�X�̖��̎擾
			DAQmxGetDevProductNum(cDevStr, &m_uiDevID);			// �f�o�C�X�� ID ���擾
			getAIDevSetting(m_uiDevID, &m_iAIMaxNumCh, &m_iTermConf);		// �f�o�C�X ID ����ő呪��`�����l�����ƕϊ��������擾
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

// Analog Input �ݒ�̏�����
bool ofNIDAQ::initAISetting(int iNumCh, float fFreq, int iDispTimeSec)
{
	if (m_iDevIndex < 0) {	// �ڑ�����Ă��� DAQ �f�o�C�X�̊m�F���I����Ă��Ȃ��ꍇ
		if (!chkConnectedDev()) return false;
	}

	if (m_bAqStatus) return false;	// ���蒆�̏ꍇ�͕ύX���Ȃ�

	cleanAISetting();

	delete[] m_pdAIData; m_pdAIData = NULL;
	delete[] m_pdDispBuf; m_pdDispBuf = NULL;

	if (DAQmxFailed(DAQmxCreateTask("", &m_hTask))) return false;
	// �����`�����l���̐ݒ�
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

	// Analog Input �̏�����
	if (DAQmxFailed(DAQmxCreateAIVoltageChan(m_hTask, cPhysicalChan, "", m_iTermConf, -10.0, 10.0, DAQmx_Val_Volts, NULL))) return false;
	// �T���v�����O�^�C�~���O�ݒ�
	m_ulSmplEventNum = (unsigned long) floor(fFreq / 200.0); // ��x�Ɏ擾����f�[�^��
	if (DAQmxFailed(DAQmxCfgSampClkTiming(m_hTask, "", fFreq, DAQmx_Val_Rising, DAQmx_Val_ContSamps, m_ulSmplEventNum))) return false;
	// �w��T���v�����O�񐔖��ɌĂ΂��R�[���o�b�N�֐��̓o�^
	if (DAQmxFailed(DAQmxRegisterEveryNSamplesEvent(m_hTask, DAQmx_Val_Acquired_Into_Buffer, m_ulSmplEventNum, 0, AIEveryNCallback, (void *)this))) return false;
	// �^�X�N�I�����ɌĂяo�����R�[���o�b�N�֐��̓o�^
	if (DAQmxFailed(DAQmxRegisterDoneEvent(m_hTask, 0, AIDoneCallback, (void*) this))) return false;

	m_iAINumCh = iNumCh;
	m_fAIFreq = fFreq;
	m_uiDispTimeSec = iDispTimeSec;

	// �f�[�^�o�b�t�@�̈�̊m��
	m_pdAIData = new float64[m_iAINumCh * m_ulSmplEventNum];

	m_uiDispBufNum = m_uiDispTimeSec * (int)round(m_fAIFreq / m_ulSmplEventNum);		// �f�B�X�v���C�p�o�b�t�@�̐�
	m_uiUpdatePeriod = (int) ceil(m_uiDispBufNum / 2000.0);		// �f�[�^�̍X�V����
	m_uiDispBufNum = m_uiDispBufNum / m_uiUpdatePeriod;			// �X�V�����ɂ��o�b�t�@����␳
	//printf("%d, %d\n", m_uiDispBufNum, m_uiUpdatePeriod);
	m_pdDispBuf = new double[m_uiDispBufNum * m_iAINumCh];
	return true;
}

// Analog Input �ݒ�̏I������
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



// �w��T���v�����O�񐔖��ɌĂ΂��R�[���o�b�N�֐�
int32 CVICALLBACK ofNIDAQ::AIEveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData) {

	ofNIDAQ* niDaq = (ofNIDAQ *)callbackData;
	int32       read = 0;

	float fAITimeout = niDaq->m_ulSmplEventNum * (1 / niDaq->m_fAIFreq) * 2;
	// �f�[�^�̎擾
	if (DAQmxFailed(DAQmxReadAnalogF64(taskHandle, niDaq->m_ulSmplEventNum, fAITimeout,
		DAQmx_Val_GroupByScanNumber, niDaq->m_pdAIData, niDaq->m_ulSmplEventNum * niDaq->m_iAINumCh, &read, NULL))) return false;

	// �f�[�^��ۑ�����ꍇ
	if (niDaq->m_bDataSaveStatus) {
		for (int i = 0; i < niDaq->m_ulSmplEventNum; i++) {
			fprintf(niDaq->m_pSaveFile, "%lf", niDaq->m_dTimeSavingData);
			for (int j = 0; j < niDaq->m_iAINumCh; j++)
				fprintf(niDaq->m_pSaveFile, ",%lf", niDaq->m_pdAIData[i * niDaq->m_iAINumCh + j]);
			fprintf(niDaq->m_pSaveFile, "\n");
			niDaq->m_dTimeSavingData += (1 / niDaq->m_fAIFreq);
		}
	}

	// �f�[�^�̕ۑ����I������ꍇ
	if (!niDaq->m_bDataSaveStatus && niDaq->m_pSaveFile != NULL) {
		fclose(niDaq->m_pSaveFile);
		niDaq->m_pSaveFile = NULL;
	}

	// �\���p�f�[�^�̃A�b�v�f�[�g�^�C�~���O���v�Z
	niDaq->m_iLoopCounter = (niDaq->m_iLoopCounter + 1) % niDaq->m_uiUpdatePeriod;	
	if (niDaq->m_iLoopCounter == 0) {
		for (int i = 0; i < niDaq->m_iAINumCh; i++)
			niDaq->m_pdDispBuf[niDaq->m_iDataCounter * niDaq->m_iAINumCh + i]
				= niDaq->m_pdAIData[(niDaq->m_ulSmplEventNum - 1)*niDaq->m_iAINumCh + i];
		niDaq->m_iDataCounter = (niDaq->m_iDataCounter + 1) % niDaq->m_uiDispBufNum;
	}

	//printf("call.\n");

	return 0;
}


// �^�X�N�I�����ɌĂ΂��R�[���o�b�N�֐�
int32 CVICALLBACK ofNIDAQ::AIDoneCallback(TaskHandle taskHandle, int32 status, void *callbackData) {

	return 0;
}