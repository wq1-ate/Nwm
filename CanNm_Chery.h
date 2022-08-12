#ifndef _CanNm_Chery_H_
#define _CanNm_Chery_H_

#include "System.h"

/* gaobal or Extern*/



/* enum */
/*NM_IND*/
typedef enum
{
    STAT_NET_OFF = 0,
    STAT_NET_ON,
    STAT_NET_STARTUP,
    STAT_NET_SDN,
    STAT_NET_CHECK    
}CAN_NWM_IND_NET_STAT_E;

/*NM_REQ*/
typedef enum
{
    CMD_NET_ON = 0,
    CMD_NET_OFF,
    CMD_NET_CHECK  //just for slave
}CAN_NWM_REQ_CMD_E;

typedef enum
{
    ERROR_ACTIVE = 0,
    ERROR_PASSIVE,
    BUS_OFF  
}CAN_NWM_IND_CONTROLLER_STATUS_E;

/*NC*/
typedef enum
{
    NM_MSG_NC_WU = 0,
    NM_MSG_NC_SA,
    NM_MSG_NC_S,//just for master
    NM_MSG_NC_MULTIMASTER
}NM_MSG_NC_E;

/* STD */
typedef  enum{
    CAN_NWM_POWER_OFF = 0,
    CAN_NWM_NET_OFF,
    CAN_NWM_WAIT_NETWORK_STARTUP,
    CAN_NWM_WAIT_CHECK_ACTIVATION,
    CAN_NWM_WAIT_NETWORK_SILENT,
    CAN_NWM_NET_ON,
}CHERY_NWM_STS_E;


/* struct */
// Master NM message ----------------------------------------------------------

typedef union
{
    struct
    {
        /** NC NetworkCommand
          * 0 - WU(Wake up request)
          * 1 - SA(System stay active request)
          * 2 - S(System go to sleep request)
          * 3 - MultiMaster
         */
        uint8 NC              :2;//NetworkCommand
        uint8 CounterError    :1;
        uint8 Reserved1       :5;//Reserved
        uint8 Counter         :4;
        uint8 ControllerSts   :2;
        uint8 CurrentFailSts  :1; 
        uint8 AL              :1;//ActiveLoadMaster
        uint8 Reserved2       :8; 
        uint8 NodeByte0       :8; //Node xx 
        uint8 NodeByte1       :8; //Node xx     
        uint8 NodeByte2       :8; //Node xx  
        uint8 NodeByte3       :8; //Node xx  
        uint8 NodeByte4       :8; //Node xx                         
    } seg;
    uint8 au8_NwMsg [8];
} NWM_MasterMsg_U;

typedef union
{
    struct
    {
        uint8 startup2on		:1;
        uint8 check2on			:1;
		uint8 silent2startup	:1;
		uint8 on2silent			:1;
		uint8 reserve			:4;
    }detail;
    uint8 u8_data;
}NWM_RX_EVENT_U;


typedef struct
{
    CAN_NWM_IND_NET_STAT_E  e_NET_STAT;
    uint16                  u16_SYS_CONF;
    CAN_NWM_IND_CONTROLLER_STATUS_E     e_CONTROLLER_STATUS;
}s_nwm_ind;

typedef struct
{
    CAN_NWM_REQ_CMD_E      e_NET_CMD;
    boolean                b_LOCAL_ACTIVE_LOADS;
    boolean                b_CURR_FAIL_STATUS;
    boolean                b_KEY_STATUS;
}s_nwm_req;

typedef struct
{
    boolean actvLoad ;
    boolean actvLoadCur ;
    uint8 AC0;
    uint8 AC1;
    uint8 AC2;
    uint8 AC3;
    uint8 AC4;
    uint8 CounterCur;
    boolean CounterFail;
} NwSysStatus;


#endif