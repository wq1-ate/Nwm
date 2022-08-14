
#include "CanNm_Chery.h"
#include "CanNm_Chery_Itf.h"

#define CAN_NWM_RESET_FSM  e_canNwmState = CAN_NWM_POWER_OFF
#define app_can_MSG_TX_DELAY_TIME ((uint8)1)/* about 80ms*/
#define STAY_ACTIVE_TIME  2000u   /* 2000ms */
#define NWM_TX_CYCLE_TIME  2000u   /* 2000ms */
#define SLEEP_DELAY1_TIME  2000u   /* 2000ms */
#define SLEEP_DELAY2_TIME  100u   /* 2000ms */

static struct
{
    CAN_NWM_IND_NET_STAT_E  e_NET_STAT;
    uint16                  u16_SYS_CONF;
    CAN_NWM_IND_CONTROLLER_STATUS_E     e_CONTROLLER_STATUS;
}s_nwm_ind;

static struct
{
    CAN_NWM_REQ_CMD_E      e_NET_CMD;
    boolean                b_LOCAL_ACTIVE_LOADS;
    boolean                b_CURR_FAIL_STATUS;
    boolean                b_KEY_STATUS;
}s_nwm_req;

static CHERY_NWM_STS_E e_canNwmState = CAN_NWM_POWER_OFF;

static NwSysStatus  sysStatusTmp;

static NWM_MasterMsg_U  s_u_NwMasterMsg = {.au8_NwMsg = {0u}};

static NWM_MasterMsg_U  s_u_NwSlaveMsg  = {.au8_NwMsg = {0u}};

static NWM_RX_EVENT_U   s_u_nwmRxEvent  = {.u8_data = 0u};

static NM_MSG_NC_E e_netOnNc = NM_MSG_NC_MULTIMASTER;

static boolean b_canBusOffrecoveryFlg = TRUE;

static boolean b_canNwmEnableAppTx = TRUE;

static boolean b_canNwmEnableNwmTx = TRUE;

static boolean b_CAN_VbatSendNwmMsgEnable = TRUE;

static boolean b_receiveMasterNodeMsg = FALSE;

static boolean b_canNwmAL = FALSE;

boolean b_canNwmWakeUpFlg = FALSE;

static uint16 u16_SA_Timer = 0u;

static uint16 u16_NmmsgSendCycle = 0u;

static uint16 u16_ST1_Timer = 0u; // Sleep Timer1

static uint16 u16_ST2_Timer = 0u; // Sleep Timer2

/****************************************************************************/
/****************************************************************************/
/**
 * Function Name: app_cheryNmHandler
 * Description: none
 *
 * Param:   none
 * Return:  none
 * Author:  2022/08/12, Wq create this function
 ****************************************************************************/
extern boolean b_canLocalActive;
static void app_cherySlaveNmHandler(void)
{
    switch(e_canNwmState)
    {
        case CAN_NWM_POWER_OFF:     //上电初始化
            s_nwm_ind.e_NET_STAT = STAT_NET_OFF;
            //AL=FALSE
            b_canNwmAL = FALSE;
            
            e_canNwmState = CAN_NWM_NET_OFF; // T1

        case CAN_NWM_NET_OFF:
            if(b_canLocalActive)
            {
                //AL=TRUE
                b_canNwmAL = TRUE;
            }
            else
            {
                //AL=FALSE
                b_canNwmAL = FALSE;
            }
            
            if(s_nwm_req.b_KEY_STATUS == TRUE)//IGN_ON
            {
        /*T13 to CAN_NWM_NET_ON*/
                //wakeup
                // app_can2Nwm_Itf_canWakeUp();/*do sth*/
                // app_can2Nwm_Itf_STBEnable();
                b_canNwmWakeUpFlg = FALSE;
                
                //Send NMmXXX(MultiMaster),
                e_netOnNc = NM_MSG_NC_MULTIMASTER;
                Send_NMmXXX(e_netOnNc);
                
                //Start SA Timer
                u16_SA_Timer = STAY_ACTIVE_TIME;
                u16_NmmsgSendCycle = NWM_TX_CYCLE_TIME;
                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_ON;

                //CAN_NWM_NET_ON
                e_canNwmState = CAN_NWM_NET_ON;

                /*clear event in this state*/
                s_u_nwmRxEvent.detail.check2on = 0;
            }
            else//IGN_OFF
            {
        /*T2 to CAN_NWM_WAIT_NETWORK_STARTUP*/
                if(s_nwm_req.e_NET_CMD == CMD_NET_ON)
                {
                    //Init CAN,wakeup
                    //app_can2Nwm_Itf_canWakeUp();/*do sth*/
                    //app_can2Nwm_Itf_STBEnable();
                    b_canNwmWakeUpFlg = FALSE;
                    
                    //Start SA Timer
                    u16_SA_Timer = STAY_ACTIVE_TIME;
                    
                    //Send NMmXXX(WU),
                    Send_NMmXXX(NM_MSG_NC_WU);
                    
                    //NM_IND(NET_STARTUP)
                    s_nwm_ind.e_NET_STAT = STAT_NET_STARTUP;

                    //CAN_NWM_WAIT_NETWORK_STARTUP
                    e_canNwmState = CAN_NWM_WAIT_NETWORK_STARTUP;

                    /*clear event in this state*/
                    s_u_nwmRxEvent.detail.check2on = 0;
                }
                else//
                {
        /*T8 to CAN_NWM_WAIT_CHECK_ACTIVATION*/
                    if((b_canNwmWakeUpFlg) || (s_nwm_req.e_NET_CMD == CMD_NET_CHECK))
                    {
                        //Init CAN,wakeup
                        //app_can2Nwm_Itf_canWakeUp();/*do sth*/
                        //app_can2Nwm_Itf_STBEnable();
                        b_canNwmWakeUpFlg = FALSE;
                        
                        //Start Sleep Timer 1,
                        u16_ST1_Timer = SLEEP_DELAY1_TIME;
                        
                        //NM_IND(NET_CHECK)
                        s_nwm_ind.e_NET_STAT = STAT_NET_CHECK;
                        
                        //CAN_NWM_WAIT_CHECK_ACTIVATION
                        e_canNwmState = CAN_NWM_WAIT_CHECK_ACTIVATION;
                    }

        /*************************** sleeping *********************************/ 
                    else
                    {
                    	
                    }
                }
            }
            break;
            
        case CAN_NWM_WAIT_NETWORK_STARTUP:
        /*T3 change AL*/
            if(s_nwm_req.b_KEY_STATUS == FALSE)
            {
                if(b_canLocalActive)
                {
                    b_canNwmAL = TRUE;
                }
                else
                {
                    b_canNwmAL = FALSE;
                }
                
            }
            
        /*T4 to CAN_NWM_NET_ON*/
            //IGN_ON
            if(s_nwm_req.b_KEY_STATUS == TRUE)
            {
                //Send NMmXXX(MultiMaster),
                e_netOnNc = NM_MSG_NC_MULTIMASTER;
                Send_NMmXXX(e_netOnNc);
                
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;
                u16_NmmsgSendCycle = NWM_TX_CYCLE_TIME;
                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_ON;
                
                //CAN_NWM_NET_ON
                e_canNwmState = CAN_NWM_NET_ON;

                /*clear event in this state*/
                s_u_nwmRxEvent.detail.startup2on = 0;
            }
            //IGN_OFF
            else
            {
                if(u16_SA_Timer)
                {
                    u16_SA_Timer--;
                }

                if((u16_SA_Timer == 0)
                 ||s_u_nwmRxEvent.detail.startup2on//(s_u_NwMasterMsg.seg.NC == NM_MSG_NC_SA)//NmmMaster(SA) received
                )
                {
                    s_u_nwmRxEvent.detail.startup2on = 0;
                    //Send NMmXXX(SA),
                    Send_NMmXXX(NM_MSG_NC_SA);
                    
                    //Start SA Timer,
                    u16_SA_Timer = STAY_ACTIVE_TIME;
                    u16_NmmsgSendCycle = NWM_TX_CYCLE_TIME;
                    //NM_IND(NET_ON)
                    s_nwm_ind.e_NET_STAT = STAT_NET_ON;  
                    
                    //CAN_NWM_NET_ON
                    e_canNwmState = CAN_NWM_NET_ON;                    
                }
            }         
            break;

        case CAN_NWM_WAIT_CHECK_ACTIVATION:
            if(u16_ST1_Timer)
            {
                u16_ST1_Timer--;
            }
            
        /*T11 to CAN_NWM_NET_ON*/
            //IGN_ON  multimster
            if(s_nwm_req.b_KEY_STATUS == TRUE)
            {
                //Send NMmXXX(MultiMaster),
                e_netOnNc = NM_MSG_NC_MULTIMASTER;
                Send_NMmXXX(e_netOnNc);
                    
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;
                u16_NmmsgSendCycle = NWM_TX_CYCLE_TIME;
                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_ON;
                
                //CAN_NWM_NET_ON
                e_canNwmState = CAN_NWM_NET_ON;
                
                /*clear event in this state*/
                s_u_nwmRxEvent.detail.check2on = 0;
            }
            //IGN_OFF SA
            else if(s_u_nwmRxEvent.detail.check2on)//(s_u_NwMasterMsg.seg.NC == NM_MSG_NC_SA)//NmmMaster(SA) received
            {
                /*clear event in this state*/
                s_u_nwmRxEvent.detail.check2on = 0;
                
                //Send NMmXXX(SA),
                Send_NMmXXX(NM_MSG_NC_SA);
                    
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;
                u16_NmmsgSendCycle = NWM_TX_CYCLE_TIME;
                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_ON;  
                
                //CAN_NWM_NET_ON
                e_canNwmState = CAN_NWM_NET_ON;  
            }
        /*T12 to CAN_NWM_WAIT_NETWORK_STARTUP*/
            else if(s_nwm_req.e_NET_CMD == CMD_NET_ON)
            {
                //AL=TRUE
                b_canNwmAL = TRUE;
                
                //Send NMmXXX(WU),
                Send_NMmXXX(NM_MSG_NC_WU);
                    
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;

                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_STARTUP;  
                
                //CAN_NWM_WAIT_NETWORK_STARTUP
                e_canNwmState = CAN_NWM_WAIT_NETWORK_STARTUP; 

                /*clear event in this state*/
                s_u_nwmRxEvent.detail.check2on = 0;
            }  
        /*T9 to CAN_NWM_NET_OFF*/
            else if(u16_ST1_Timer == 0)
            {
                //Shutdown CAN: can goto sleep
                b_canNwmWakeUpFlg = FALSE;
                if(app_can2Nwm_Itf_canGotoSleep())
                {
                    app_can2Nwm_Itf_STBDisable();
                                        
                    //NM_IND(NET_OFF)
                    s_nwm_ind.e_NET_STAT = STAT_NET_OFF;

                    //CAN_NWM_NET_OFF
                    e_canNwmState = CAN_NWM_NET_OFF;
                    
                    /*clear event in this state*/
                    s_u_nwmRxEvent.detail.check2on = 0;   
                }                
            }            
            break;

        case CAN_NWM_WAIT_NETWORK_SILENT:
            if(u16_ST2_Timer)
            {
                u16_ST2_Timer--;
            }
        /*T14 to CAN_NWM_NET_ON*/
            //IGN_ON  multimster
            if(s_nwm_req.b_KEY_STATUS == TRUE)
            {
                //Send NMmXXX(MultiMaster),
                e_netOnNc = NM_MSG_NC_MULTIMASTER;
                Send_NMmXXX(e_netOnNc);
                    
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;
                u16_NmmsgSendCycle = NWM_TX_CYCLE_TIME;
                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_ON;
                
                //CAN_NWM_NET_ON
                e_canNwmState = CAN_NWM_NET_ON;

                /*clear event in this state*/
                s_u_nwmRxEvent.detail.silent2startup = 0;
            }
        /*T10 to CAN_NWM_WAIT_NETWORK_STARTUP*/
            else if(s_nwm_req.e_NET_CMD == CMD_NET_ON)
            {
                //AL=TRUE
                b_canNwmAL = TRUE;
                
                //Send NMmXXX(WU),
                Send_NMmXXX(NM_MSG_NC_WU);
                    
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;

                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_STARTUP;  
                
                //CAN_NWM_WAIT_NETWORK_STARTUP
                e_canNwmState = CAN_NWM_WAIT_NETWORK_STARTUP; 

                /*clear event in this state*/
                s_u_nwmRxEvent.detail.silent2startup = 0;
            } 
            else if(s_u_nwmRxEvent.detail.silent2startup)//(s_u_NwMasterMsg.seg.NC == NM_MSG_NC_WU)//NmmMaster(WU) received
            {                    
                /*clear event in this state*/
                s_u_nwmRxEvent.detail.silent2startup = 0;
                
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;

                //NM_IND(NET_ON)
                s_nwm_ind.e_NET_STAT = STAT_NET_STARTUP;  
                
                //CAN_NWM_WAIT_NETWORK_STARTUP
                e_canNwmState = CAN_NWM_WAIT_NETWORK_STARTUP; 
            }
        /*T7 to CAN_NWM_NET_OFF*/
            else if(u16_ST2_Timer == 0)
            {
                //Shutdown CAN: can goto sleep
                b_canNwmWakeUpFlg = FALSE;
                if(app_can2Nwm_Itf_canGotoSleep())
                {
                    app_can2Nwm_Itf_STBDisable();
                                        
                    //NM_IND(NET_OFF)
                    s_nwm_ind.e_NET_STAT = STAT_NET_OFF;

                    //CAN_NWM_NET_OFF
                    e_canNwmState = CAN_NWM_NET_OFF;
                    
                    /*clear event in this state*/
                    s_u_nwmRxEvent.detail.silent2startup = 0;  
                }   
            }  
            break;

        case CAN_NWM_NET_ON:                        
            /*SA timer*/
            if(u16_SA_Timer)
            {
                u16_SA_Timer--;
            }

            /*IGN == OFF*/
        /*T6  goto silent*/
            if(s_nwm_req.b_KEY_STATUS == FALSE)
            {
                if(s_nwm_req.e_NET_CMD == CMD_NET_OFF)
                {
                    //AL=FALSE
                    b_canNwmAL = FALSE;
                }
                
                if(((s_nwm_req.b_LOCAL_ACTIVE_LOADS == FALSE) &&(u16_SA_Timer == 0))
                 ||(s_u_nwmRxEvent.detail.on2silent)//(s_u_NwMasterMsg.seg.NC == NM_MSG_NC_S)//NmmMaster(S) received
                )
                {
                    e_netOnNc = NM_MSG_NC_SA;
                    /*clear event in this state*/
                    s_u_nwmRxEvent.detail.on2silent = 0;
                    
                    //Start Sleep Timer 2,
                    u16_ST2_Timer = SLEEP_DELAY2_TIME;
                    
                    //NM_IND(NET_SDN)
                    s_nwm_ind.e_NET_STAT = STAT_NET_SDN;

                    //CAN_NWM_WAIT_NETWORK_SILENT
                    e_canNwmState = CAN_NWM_WAIT_NETWORK_SILENT;
                }
        /*T5  IGN_OFF and  (SA Timer Timeout AND (NM_REQ(LOCAL_ACTIVE_LOADS) ==TRUE))*/                
                else if((s_nwm_req.b_LOCAL_ACTIVE_LOADS == TRUE) &&(u16_SA_Timer == 0))
                {
                    //AL=TRUE
                    if(b_canLocalActive)
                    {
                        b_canNwmAL = TRUE;
                    }
                    else
                    {
                        b_canNwmAL = FALSE;
                    }
                    
                    //Send NMmXXX (SA),
                    e_netOnNc = NM_MSG_NC_SA;
                    
                    //Start SA Timer,
                    u16_SA_Timer = STAY_ACTIVE_TIME;
                    
                    //NM_IND(NET_ON)      
                    s_nwm_ind.e_NET_STAT = STAT_NET_ON;
                }
            }
        /*T5  IGN_ON and  (SA Timer Timeout AND (NM_REQ(LOCAL_ACTIVE_LOADS) ==TRUE))*/  
            else
            {
                if((s_nwm_req.b_LOCAL_ACTIVE_LOADS == TRUE) &&(u16_SA_Timer == 0))
                {
                    //AL=TRUE
                    if(b_canLocalActive)
                    {
                        b_canNwmAL = TRUE;
                    }
                    else
                    {
                        b_canNwmAL = FALSE;
                    }
                    
                    //Send NMmXXX (MultiMaster),
                    e_netOnNc = NM_MSG_NC_MULTIMASTER;
                    
                    //Start SA Timer,
                    u16_SA_Timer = STAY_ACTIVE_TIME;
                    
                    //NM_IND(NET_ON)      
                    s_nwm_ind.e_NET_STAT = STAT_NET_ON;
                }
            }   

            /*clear event in this state*/
            s_u_nwmRxEvent.detail.on2silent = 0;

            /* cycle send NWM msg*/
            if(--u16_NmmsgSendCycle==0)
            {
                u16_NmmsgSendCycle = NWM_TX_CYCLE_TIME;
                Send_NMmXXX(e_netOnNc);
            }
            
            break;            
        
        default :
            e_canNwmState = CAN_NWM_POWER_OFF;
            break;
    }

}

/****************************************************************************/
/****************************************************************************/
/**
 * Function Name: app_canNmCheryMainFun
 * Description: none
 *
 * Param:   none
 * Return:  none
 * Author:  2022/08/12, Wq create this function
 ****************************************************************************/
void app_canNmCheryMainFun(void)
{
    s_nwm_req.b_KEY_STATUS = app_cheryNmGetIgnStatus();
    if((app_cheryNwm_Itf_checkSleepCondition() == TRUE) && (sysStatusTmp.actvLoad == FALSE)) {
        s_nwm_req.e_NET_CMD = CMD_NET_OFF;
        s_nwm_req.b_LOCAL_ACTIVE_LOADS = FALSE;
    } else {
        s_nwm_req.e_NET_CMD = CMD_NET_ON;
        s_nwm_req.b_LOCAL_ACTIVE_LOADS = TRUE;
    }
    app_cherySlaveNmHandler();
    /* check can  app&nwm msg tx premission*/
    b_CAN_VbatSendNwmMsgEnable = app_canNwm_Itf_checkTxCondition();
    checkCanNmMsgTxPermission();
}

static void checkCanNmMsgTxPermission(void)
{
    static uint8 u8_msgTxDelay = 0;
    
    if(  b_canBusOffrecoveryFlg
      && ((e_canNwmState == CAN_NWM_NET_ON) )
      && b_CAN_VbatSendNwmMsgEnable
      && b_canNwmEnableAppTx)
    {
        if(u8_msgTxDelay < app_can_MSG_TX_DELAY_TIME) 
        {
            u8_msgTxDelay++;
        }
        else
        {
		    app_canTxCtrl(TRUE); 
        } 
    }
    else
    {
        u8_msgTxDelay = 0;
        app_canTxCtrl(FALSE); 
    }
}

void app_canNwm_appTxCtrl(boolean cmd) // 28 服务控制
{
    if(cmd == FALSE)
    {
        b_canNwmEnableNwmTx = FALSE;
    }
    else
    {
        b_canNwmEnableNwmTx = TRUE;
    }
}

static void Send_NMmXXX(NM_MSG_NC_E e_NC)
{
    uint8 u8_data[2];
    boolean b_currentFailSts = FALSE;

    b_currentFailSts = app_can2Nwm_Itf_checkCommDtc();

    if(b_canNwmEnableNwmTx == FALSE)return;
    
    u8_data[0] = (uint8)e_NC;
    if(b_receiveMasterNodeMsg)
    {
        u8_data[1] = (b_canNwmAL<<7)|(s_u_NwMasterMsg.seg.Counter)|(b_currentFailSts<<6);
        b_receiveMasterNodeMsg = FALSE;
    }
    else
    {
        u8_data[1] = (b_canNwmAL<<7)|(b_currentFailSts<<6);
    }
    (void)app_canNwm_Itf_canSendNmFrame(u8_data);
}

void app_canNwm_v_receive600Msg(uint8* p_u8_data)
{
	uint8 i = 0;
	static uint8 canNwm_idx;

	////////////////////////////////	

    /*check recieve event*/
//master    
    
        for( i = 0; i < 8; i++ )
    	{
    		s_u_NwMasterMsg.au8_NwMsg[i] = p_u8_data[i];
    	}
	
    	/*s_u_nwmRxEvent.detail.startup2on*/
    	if(CAN_NWM_WAIT_NETWORK_STARTUP == e_canNwmState)
    	{
    	    if(NM_MSG_NC_SA == s_u_NwMasterMsg.seg.NC)
    	    {
    	        s_u_nwmRxEvent.detail.startup2on = 1;
    	    }
    	}
    	
    	/*s_u_nwmRxEvent.detail.check2on*/
    	if(CAN_NWM_WAIT_CHECK_ACTIVATION == e_canNwmState)
    	{
    	    if(NM_MSG_NC_SA == s_u_NwMasterMsg.seg.NC)
    	    {
    	        s_u_nwmRxEvent.detail.check2on = 1;
    	    }
    	}
    	    /*
    	     * it is special handle.
    	     * there is a frame msg wake up can-bus,and the msg is master frame and nc = sa.
    	     * we need set check2on.
    	     */
    	if(CAN_NWM_NET_OFF == e_canNwmState)
    	{
    	    if(b_canNwmWakeUpFlg)
    	    {
    	        if(NM_MSG_NC_SA == s_u_NwMasterMsg.seg.NC)
        	    {
        	        s_u_nwmRxEvent.detail.check2on = 1;
        	    }
    	    }
    	}
    	
    	/*s_u_nwmRxEvent.detail.silent2startup*/
    	if(CAN_NWM_WAIT_NETWORK_SILENT == e_canNwmState)
    	{
    	    if(NM_MSG_NC_WU == s_u_NwMasterMsg.seg.NC)
    	    {
    	        s_u_nwmRxEvent.detail.silent2startup = 1;
    	    }
    	}

    	/*s_u_nwmRxEvent.detail.on2silent*/
    	if(CAN_NWM_NET_ON== e_canNwmState)
    	{
    	    if((NM_MSG_NC_S == s_u_NwMasterMsg.seg.NC)&&(s_nwm_req.b_KEY_STATUS == FALSE))
    	    {
    	        s_u_nwmRxEvent.detail.on2silent = 1;
//                u16_Can2SleepDelayCount = 0;
    	    }

    	    if((NM_MSG_NC_SA == s_u_NwMasterMsg.seg.NC)&&(s_nwm_req.b_KEY_STATUS == FALSE))
    	    {
                e_netOnNc = NM_MSG_NC_SA;
                
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;
                
                //NM_IND(NET_ON)      
                s_nwm_ind.e_NET_STAT = STAT_NET_ON;
                u16_NmmsgSendCycle = 1;
				b_receiveMasterNodeMsg = TRUE;
    	    }
    	    if((NM_MSG_NC_MULTIMASTER == s_u_NwMasterMsg.seg.NC)&&(s_nwm_req.b_KEY_STATUS == TRUE))
    	    {
                //Send NMmXXX (NM_MSG_NC_MULTIMASTER),
                e_netOnNc = NM_MSG_NC_MULTIMASTER;
                
                //Start SA Timer,
                u16_SA_Timer = STAY_ACTIVE_TIME;
                
                //NM_IND(NET_ON)      
                s_nwm_ind.e_NET_STAT = STAT_NET_ON;
                u16_NmmsgSendCycle = 1;
				b_receiveMasterNodeMsg = TRUE;
    	    }
    	}  
}
