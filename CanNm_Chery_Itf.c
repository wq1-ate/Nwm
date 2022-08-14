#include "CanNm_Chery_Itf.h"

boolean b_canLocalActive = FALSE;
uint8 app_cheryNm_Itf_GetIgnStatus(void)
{
    uint8 result = E_OK;

    return result
}
boolean app_cheryNwm_Itf_checkSleepCondition(void)
{
    b_canLocalActive = TRUE;
    return FALSE;
}

boolean app_canNwm_Itf_checkTxCondition(void)
{
    static boolean b_TxEnable = TRUE;
    static uint32 u32_delayTime = 0;
    static uint32 u32_delayTime1 = 0;
    
    if(hwa_getVbat() <= VBAT_6V)
    {
        b_TxEnable = FALSE;
        u32_delayTime = 0;
        u32_delayTime1 = 0;
    }
	else if(hwa_getVbat() > VBAT_6V
        && hwa_getVbat() < VBAT_16V)
	{
        u32_delayTime = 0;
        u32_delayTime1 = 0;
    	b_TxEnable = TRUE;           
    }
    else if( (hwa_getVbat() > VBAT_16V)
        && (hwa_getVbat() < VBAT_18V))
	{
        if(u32_delayTime < 360000)
        {
            u32_delayTime ++;
            b_TxEnable = TRUE;
        }
        else
        {
            b_TxEnable = FALSE; 
        } 
        u32_delayTime1 = 0;
    }
    else
	{
        if(u32_delayTime1 < 6000)
        {
            u32_delayTime1 ++;
            b_TxEnable = TRUE;
        }
        else
        {
            b_TxEnable = FALSE; 
        } 
        u32_delayTime = 0;
    }
    return b_TxEnable;
}


boolean app_canNwm_Itf_checkCommDtc(void)
{
    return FALSE;
}

uint8 app_canNwm_Itf_canSendNmFrame(uint8 *data)
{
    (void)data;
    return E_OK;
}
extern boolean b_canNwmWakeUpFlg;
void app_canNwm_Itf_RemoteWakeup(void)
{
    b_canNwmWakeUpFlg = TRUE;
}






