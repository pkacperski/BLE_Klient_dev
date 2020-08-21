/**
  ******************************************************************************
  * @file    sample_service.h 
  * @author  CL
  * @version V1.0.0
  * @date    04-July-2014
  * @brief   
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _SAMPLE_SERVICE_H_
#define _SAMPLE_SERVICE_H_

#ifdef __cplusplus
 extern "C" {
#endif 

 /* Private defines */
 #define MAX_CONNECTIONS 8 //Mode 3: master/slave, max. 8 connections

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "cmsis_os.h"
#include "app_x-cube-ble1.h"
#include "bluenrg_gap.h"
#include "bluenrg_aci_const.h"
#include "hci.h"
#include "hci_le.h"
#include "sm.h"
#include "role_type.h"

extern volatile bool set_connectable;
extern volatile bool client_ready;
extern volatile bool discovery_started;
extern volatile bool discovery_finished;
extern volatile bool start_notifications_enable;
extern volatile bool all_notifications_enabled;
extern volatile bool start_read_tx_char_handle;//?
extern volatile bool start_read_rx_char_handle;//?
extern volatile bool all_tx_char_handles_read;
extern volatile bool all_rx_char_handles_read;
extern volatile bool all_servers_connected;
extern volatile bool services_discovered;
extern volatile bool pairing_started;
extern volatile bool pairing_finished;
extern uint8_t bnrg_expansion_board;
extern uint8_t whichLoopIteration;
extern uint8_t whichServerConnecting;
extern uint8_t dataBLE[][MSG_LEN];
extern uint8_t newData;
extern BLE_RoleTypeDef BLE_Role;
extern osMutexId newDataMutexHandle;
extern bool newDataPresent;

/* Dla "drzewa" urzadzen pamietanego przez klienta */
typedef struct ConnectedSensor {
	uint8_t sensorName[MAX_NAME_LEN];
 	float lastTempValue;
 	float lastHumidValue;
} ConnectedSensor;
/* Skanowanie serverow przez klienta */
typedef enum {
	DISCONNECTED,
 	READY_TO_CONNECT,
 	CONNECTED,
	PAIRED,
	EXCHANGING_DATA,
	DISCONNECTED_AFTER_CONNECTION_CREATED
} ConnectionStatus;
typedef struct FoundDeviceInfo {
 	uint8_t deviceAddressType;
 	tBDAddr deviceAddress;
 	uint8_t deviceName[MAX_NAME_LEN];
 	ConnectionStatus connStatus;
 	uint16_t connHandle;
 	uint8_t connSensorsCount;
 	ConnectedSensor connSensors[MAX_CONNECTIONS];
} FoundDeviceInfo;

//
extern FoundDeviceInfo foundDevices[];
extern uint8_t foundDevicesCount;
extern uint8_t prevFoundDevicesCount;

/** @addtogroup Applications
 *  @{
 */

/** @addtogroup SampleAppThT
 *  @{
 */
 
/** @addtogroup SAMPLE_SERVICE 
 * @{
 */

/** @addtogroup SAMPLE_SERVICE_Exported_Defines
 *  @{
 */
#define IDB04A1 0
#define IDB05A1 1
   
/** 
* @brief Handle of TX Characteristic on the Server. The handle should be
*        discovered, but it is fixed only for this demo.
*/ 
//#define TX_HANDLE 0x0011

/** 
* @brief Handle of RX Characteristic on the Client. The handle should be
*        discovered, but it is fixed only for this demo.
*/ 
//#define RX_HANDLE   0x0014

/**
 * @}
 */

/** @addtogroup SAMPLE_SERVICE_Exported_Functions
 *  @{
 */
tBleStatus Add_Sample_Service(void);
void Make_Connection(void);
void receiveData(uint8_t* data_buffer, uint8_t Nb_bytes);
//void sendData(uint8_t* data_buffer, uint8_t Nb_bytes);
void sendData(uint8_t server_index, uint8_t* data_buffer, uint8_t Nb_bytes);
void startReadTXCharHandle(void);
void startReadRXCharHandle(void);
void enableNotification(void);
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length,
                           uint8_t *att_data);
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle);
void GAP_DisconnectionComplete_CB(uint16_t conn_handle);
void GATT_Notification_CB(uint16_t attr_handle, uint8_t attr_len,
                          uint8_t *attr_value, uint16_t conn_handle);
void GAP_AdvertisingReport_CB(le_advertising_info *adv_info);
void user_notify(void * pData);
void Pair_Devices(void);
/**
 * @}
 */
 
/**
 * @}
 */
 
/**
 * @}
 */

/**
 * @}
 */
 
#ifdef __cplusplus
}
#endif

#endif /* _SAMPLE_SERVICE_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

