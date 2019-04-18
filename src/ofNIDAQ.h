#pragma once
#ifndef _OF_NIDAQ_H_
#define _OF_NIDAQ_H_

// NIDAQmx ���C�u���� (NIDAQmx �̃C���X�g�[���K�v)
#include <NIDAQmx.h>
#pragma comment(lib, "NIDAQmx.lib")
#include <Windows.h>

#define DAQmxErrChk(functionCall) if(DAQmxFailed(error=(fanctionCall))) goto Error; else


// OpenFrameworks �̃��C�u����
#include "ofMain.h"


class ofNIDAQ {
public:
	ofNIDAQ();
	~ofNIDAQ();

	bool chkConnectedDev();		// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�̊m�F
	char* getDevName() { return ((m_iDevIndex < 0) ? NULL : m_cDevName); };		// �f�o�C�X���̎擾
	int getDevAIMaxNum() { return m_iAIMaxNumCh; };		// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�� Analog Input �̍ő�`�����l�������擾
	int getCurrentAINumCh() { return m_iAINumCh; };		// ���݂� Analog Input �̐ݒ�`�����l�������擾
	float getCurrentAIFreq() { return m_fAIFreq; };		// ���݂� Analog Input �̃T���v�����O���g��
	int getCurrentDispTime() { return m_uiDispTimeSec; }; // �\�����Ԃ��擾
	bool isDataAquisition() { return m_bAqStatus; };	// �v�����ł��邩�ǂ������擾
	bool startSaveData(char* saveFilePath);				// �t�@�C���̕ۑ��J�n
	bool stopSaveData();								// �t�@�C���̕ۑ��I��
	bool isDataSaving() { return m_bDataSaveStatus; };		// �t�@�C����ۑ������ǂ������擾
	double getCurrentSaveTime() { return m_dTimeSavingData; };	// �t�@�C���ۑ����J�n���Ă��猻�݂܂ł̎���

	int getDispBufNum() { return m_uiDispBufNum; };		// �\���p�f�[�^�o�b�t�@�� 1 Ch ������̐����擾
	float64* getDispBuf() { return m_pdDispBuf; };		// �\���p�o�b�t�@�̎擾
	int getCurrentDispBufNum() { return m_iDataCounter; };		// �\���p�o�b�t�@�ɂ����錻�݂̃f�[�^�ʒu�̎擾

	bool initAISetting(int iNumCh, float fFreq, int iDispTimeSec );		// Analog Input �ݒ�̏�����
	bool cleanAISetting();		// Analog Input �ݒ�̏I������

	bool startAISampling();		// Analog Input �T���v�����O���J�n
	bool stopAISampling();		// Analog Input �T���v�����O���I��

private:
	TaskHandle		m_hTask;			// NI-DAQ �������f�o�C�X�n���h��
	int				m_iDevIndex;		// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�̃C���f�b�N�X
	uInt32			m_uiDevID;			// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�� ID �ԍ�
	char			m_cDevName[20];		// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�̖���
	int				m_iAIMaxNumCh;		// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�� Analog Input �̍ő�`�����l����
	int				m_iTermConf;		// PC �ɐڑ�����Ă��� DAQ �f�o�C�X�� A/D �ϊ�����
	int				m_iAINumCh;			// Analog Input �̃`�����l����
	float			m_fAIFreq;			// Analog Input �̃T���v�����O���g��
	bool			m_bAqStatus;		// ���蒆�ł��邩�ǂ����̃t���O
	FILE*			m_pSaveFile;		// ����f�[�^�̕ۑ��p�t�@�C���|�C���^

	float64*		m_pdAIData;			// �v�������f�[�^�p�̃o�b�t�@
	double*			m_pdDispBuf;		// �\���p�f�[�^�̃o�b�t�@
	unsigned long   m_ulSmplEventNum;	// Analog Input �ň��Ɏ擾����f�[�^��
	unsigned int	m_uiDispBufNum;		// �\���p�f�[�^�o�b�t�@��
	unsigned int	m_uiUpdatePeriod;	// �\���p�f�[�^�o�b�t�@�̍X�V����
	unsigned int	m_uiDispTimeSec;	// �\������ (sec)

	int				m_iLoopCounter;
	int				m_iDataCounter;

	bool			m_bDataSaveStatus;
	double			m_dTimeSavingData;

	void getAIDevSetting(uInt32 uiDevID, int* piAIMaxNumCh, int* piTermConf);		// �f�o�C�X�ɑ΂���ő�`�����l������ A/D �ϊ������̎擾

	// �w�萔�T���v�����O���ɌĂ΂��R�[���o�b�N�֐�
	static int32 CVICALLBACK AIEveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	// �^�X�N�I�����ɌĂ΂��R�[���o�b�N�֐�
	static int32 CVICALLBACK AIDoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);
};

#endif //_OF_NIDAQ_H_