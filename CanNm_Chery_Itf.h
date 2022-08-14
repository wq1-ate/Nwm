#ifndef _CanNm_Chery_ITF_H_
#define _CanNm_Chery_ITF_H_

#include "System.h"




uint8 app_cheryNmGetIgnStatus(void);

boolean app_cheryNwm_Itf_checkSleepCondition(void);

boolean app_canNwm_Itf_checkTxCondition(void);

boolean app_canNwm_Itf_checkCommDtc(void);

uint8 app_canNwm_Itf_canSendNmFrame(uint8 *data);










#endif