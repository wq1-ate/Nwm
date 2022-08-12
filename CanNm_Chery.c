
#include "CanNm_Chery.h"
#include "CanNm_Chery_Itf.h"

#define CAN_NWM_RESET_FSM  e_canNwmState = CAN_NWM_POWER_OFF


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
void app_cheryNmHandler(void)
{

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

}