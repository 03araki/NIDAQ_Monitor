#include "ofNIDAQ.h"

// �f�o�C�X�ɑ΂���ő�`�����l������ A/D �ϊ������̎擾
// �V�����f�o�C�X��ǉ������ꍇ�͂����ɒǉ����邱�ƁD
void ofNIDAQ::getAIDevSetting(uInt32 uiDevID, int* piAIMaxNumCh, int* piTermConf) {
	switch (uiDevID) {
	case 0x73FC:	// USB-6212 (BNC)
		*piAIMaxNumCh = 8;
		*piTermConf = DAQmx_Val_Diff;
		break;
	case 0x73FE:	// USB-6218 (BNC)
		*piAIMaxNumCh = 16;
		*piTermConf = DAQmx_Val_Diff;
		break;
	case 0x7270:	// USB-6211
		*piAIMaxNumCh = 16;
		*piTermConf = DAQmx_Val_RSE;
		break;
	default:
		*piAIMaxNumCh = -1;
		*piTermConf = DAQmx_Val_RSE;
		break;
	}
}