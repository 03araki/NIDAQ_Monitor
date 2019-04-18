#include "ofNIDAQ.h"

// デバイスに対する最大チャンネル数と A/D 変換方式の取得
// 新しいデバイスを追加した場合はここに追加すること．
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