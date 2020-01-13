/********************************************************************************************************
 * @file     zb_appCb.c
 *
 * @brief    Call back function for zigbee
 *
 * @author
 * @date     Jan. 4, 2018
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/

#if (__PROJECT_TL_CONTACT_SENSOR__)

/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "zb_api.h"
#include "zcl_include.h"
#include "bdb.h"
#include "ota.h"
#include "sampleSensor.h"
#include "app_ui.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
void zbdemo_bdbInitCb(u8 status, u8 joinedNetwork);
void zbdemo_bdbCommissioningCb(u8 status, void *arg);
void zbdemo_bdbIdentifyCb(u8 endpoint, u16 srcAddr, u16 identifyTime);


/**********************************************************************
 * LOCAL VARIABLES
 */
bdb_appCb_t g_zbDemoBdbCb = 
{
	zbdemo_bdbInitCb, 
	zbdemo_bdbCommissioningCb, 
	zbdemo_bdbIdentifyCb, 
	NULL
};

#ifdef ZCL_OTA
ota_callBack_t sampleSensor_otaCb =
{
	sampleSensor_otaProcessMsgHandler,
};
#endif


volatile u8 T_zbdemoBdbInfo[6] = {0};




/**********************************************************************
 * FUNCTIONS
 */
void sampleSensor_bdbRejoinStart(void *arg){
	zb_rejoin_mode_set(REJOIN_INSECURITY);
	bdb_init((af_simple_descriptor_t *)&sampleSensor_simpleDesc, &g_bdbCommissionSetting, &g_zbDemoBdbCb, 1);
}

/*********************************************************************
  * @fn      zbdemo_bdbInitCb
  *
  * @brief   application callback for bdb initiation
  *
  * @param   status - the status of bdb init BDB_INIT_STATUS_SUCCESS or BDB_INIT_STATUS_FAILURE
  *
  * @param   joinedNetwork  - 1: node is on a network, 0: node isn't on a network
  *
  * @return  None
  */
void zbdemo_bdbInitCb(u8 status, u8 joinedNetwork){
	if(status == BDB_INIT_STATUS_SUCCESS){
		T_zbdemoBdbInfo[0]++;

		if(joinedNetwork){
			zb_setPollRate(POLL_RATE);

#ifdef ZCL_OTA
			ota_queryStart(30);
#endif

#ifdef ZCL_POLL_CTRL
			sampleSensor_zclCheckInStart();
#endif
		}

#if	(!ZBHCI_EN)
		/*
		 * start bdb commissioning
		 * */
		if(!joinedNetwork){
#if 1
			bdb_networkSteerStart();
#else
			bdb_networkTouchLinkStart(BDB_COMMISSIONING_ROLE_INITIATOR);
#endif
		}
#endif
	}else{
		T_zbdemoBdbInfo[1]++;
		if(joinedNetwork){
			T_zbdemoBdbInfo[2]++;
			//TL_SCHEDULE_TASK(sampleSensor_bdbRejoinStart, NULL);
		}
	}
}

/*********************************************************************
  * @fn      zbdemo_bdbCommissioningCb
  *
  * @brief   application callback for bdb commissioning
  *
  * @param   status - the status of bdb commissioning
  *
  * @param   arg
  *
  * @return  None
  */
void zbdemo_bdbCommissioningCb(u8 status, void *arg){
	T_zbdemoBdbInfo[3]++;
	if(status == BDB_COMMISSION_STA_SUCCESS){
		T_zbdemoBdbInfo[4]++;

		zb_setPollRate(POLL_RATE);

#ifdef ZCL_POLL_CTRL
		sampleSensor_zclCheckInStart();
#endif

		light_blink_start(2, 200, 200);

#ifdef ZCL_OTA
        ota_queryStart(30);
#endif
	}else if(status == BDB_COMMISSION_STA_IN_PROGRESS){

	}else if(status == BDB_COMMISSION_STA_NOT_AA_CAPABLE){

	}else if(status == BDB_COMMISSION_STA_NO_NETWORK){

	}else if(status == BDB_COMMISSION_STA_TARGET_FAILURE){

	}else if(status == BDB_COMMISSION_STA_FORMATION_FAILURE){

	}else if(status == BDB_COMMISSION_STA_NO_IDENTIFY_QUERY_RESPONSE){

	}else if(status == BDB_COMMISSION_STA_BINDING_TABLE_FULL){

	}else if(status == BDB_COMMISSION_STA_NO_SCAN_RESPONSE){

	}else if(status == BDB_COMMISSION_STA_NOT_PERMITTED){

	}else if(status == BDB_COMMISSION_STA_TCLK_EX_FAILURE){

	}else if(status == BDB_COMMISSION_STA_PARENT_LOST){

	}
}


extern void sampleSensor_zclIdentifyCmdHandler(u8 endpoint, u16 srcAddr, u16 identifyTime);
void zbdemo_bdbIdentifyCb(u8 endpoint, u16 srcAddr, u16 identifyTime){
	sampleSensor_zclIdentifyCmdHandler(endpoint, srcAddr, identifyTime);
}



#ifdef ZCL_OTA
void sampleSensor_otaProcessMsgHandler(u8 evt, u8 status)
{
	if(evt == OTA_EVT_START){
		if(status == ZCL_STA_SUCCESS){
			zb_setPollRate(QUEUE_POLL_RATE);
		}else{

		}
	}else if(evt == OTA_EVT_COMPLETE){
		zb_setPollRate(POLL_RATE);

		if(status == ZCL_STA_SUCCESS){
			ota_mcuReboot();
		}else{
			ota_queryStart(30);
		}
	}
}
#endif

/*********************************************************************
  * @fn      sampleSwitch_leaveCnfHandler
  *
  * @brief   Handler for ZDO Leave Confirm message.
  *
  * @param   pRsp - parameter of leave confirm
  *
  * @return  None
  */
void sampleSensor_leaveCnfHandler(void *p)
{
	nlmeLeaveConf_t *pCnf = (nlmeLeaveConf_t *)p;
	//printf("sampleSwitch_leaveCnfHandler, status = %x\n", pCnf->status);
    if(pCnf->status == SUCCESS ){
    	//SYSTEM_RESET();
    }
}

/*********************************************************************
  * @fn      sampleSwitch_leaveIndHandler
  *
  * @brief   Handler for ZDO leave indication message.
  *
  * @param   pInd - parameter of leave indication
  *
  * @return  None
  */

void sampleSensor_leaveIndHandler(void *p)
{
	//nlmeLeaveInd_t *pInd = (nlmeLeaveInd_t *)p;
    //printf("sampleSwitch_leaveIndHandler, rejoin = %d\n", pInd->rejoin);
    //printfArray(pInd->device_address, 8);
}


#endif  /* __PROJECT_TL_CONTACT_SENSOR__ */