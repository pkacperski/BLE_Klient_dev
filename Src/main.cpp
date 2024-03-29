/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "app_x-cube-ble1.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <cstdio>
#include "sample_service.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;
osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
osThreadId askForDataTaskHandle;
osThreadId presentationTaskHandle;
osThreadId communicationTaskHandle;
osMutexId uartMutexHandle;
osMutexId newDataMutexHandle;
xQueueHandle msgQueueHandle;
uint16_t counter;
uint8_t newData;
uint8_t sensorObjectCount;
uint8_t whichSensorWrites;
uint8_t whichLoopIteration;
uint8_t whichServerReceivesConfiguration;
uint8_t dataBLE[MAX_MSGS][MSG_LEN];
uint8_t sentConfigurationMsg[MSG_LEN];
uint8_t uartRcv[RCV_CONFIG_MSG_LEN];
FoundDeviceInfo foundDevices[MAX_CONNECTIONS];
char uartData[BUF_LEN];
bool newConfig;
bool promptForInitConfig;
bool deviceDisconnected;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void const * argument);
/* USER CODE BEGIN PFP */
void AskForDataTaskThread(void const * argument);
void PresentationTaskThread(void const * argument);
void CommunicationTaskThread(void const * argument);
void presentDataFromSensor(uint8_t which);
void delayMicroseconds(uint32_t us);
void prepareNewConfig(MessageType msgType, uint8_t deviceInd, uint8_t sensorType, uint16_t interval, uint8_t *name);
bool checkIfTempSensorReadoutCorrect(uint32_t dataBits, uint8_t checksumBits);
void printConnectedDevicesTree(void);
void updateReadoutValues(void);
void prepareForScanning(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_BlueNRG_MS_Init();
  /* USER CODE BEGIN 2 */

  //TODO: ilosc znakow po ktorych jest generowane przerwanie
  HAL_UART_Receive_IT(&huart3, uartRcv, RCV_CONFIG_MSG_LEN);
  promptForInitConfig = true;

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  uartMutexHandle = xSemaphoreCreateMutex();
  newDataMutexHandle = xSemaphoreCreateMutex();
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  msgQueueHandle = xQueueCreate(MAX_MSGS, sizeof(uartData));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(askForDataTask, AskForDataTaskThread, osPriorityNormal, 0, 128);
  askForDataTaskHandle = osThreadCreate(osThread(askForDataTask), NULL);

  osThreadDef(presentationTask, PresentationTaskThread, osPriorityNormal, 0, 512);
  presentationTaskHandle = osThreadCreate(osThread(presentationTask), NULL);

  osThreadDef(communicationTask, CommunicationTaskThread, osPriorityLow, 0, 128);
  communicationTaskHandle = osThreadCreate(osThread(communicationTask), NULL);
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
 
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_USART3;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;

  //dla RPi
  huart3.Init.BaudRate = 115200;
//  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|LD2_Pin|GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 LD2_Pin PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|LD2_Pin|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
	  if(client_ready){
		  //Wyslij sygnal do taska odczytu ze powinien teraz sie uruchomic
		  xTaskNotify(askForDataTaskHandle, 0x01, eSetBits);
	  }
	  else {
	  	  MX_BlueNRG_MS_Process();
	  }

	  //
	  //osDelay(DELAY_TIME/30); //wlaczenie delaya na 100 milisek. powoduje wypisywanie smieci w terminalu - czemu?

  }
  /* USER CODE END 5 */ 
}

/* USER CODE BEGIN 6 */
void AskForDataTaskThread(void const * argument)
{
	/* Podejscie: trzy taski - supervisor -> odczyt -> prezentacja: task odczytu danych */
	uint32_t notifValue;
	/* Infinite loop */
	for(;;)
	{
		xTaskNotifyWait(pdFALSE, 0xFF, &notifValue, portMAX_DELAY);
		if((notifValue&0x01) != 0x00) //Sprawdza czy notifValue zawiera wartosc ktora wyslal task supervisora
		{
		  MX_BlueNRG_MS_Process();
		  xSemaphoreTake(newDataMutexHandle, DELAY_TIME);
		  //Uruchom task prezentacji tylko wtedy, gdy przyjda nowe dane i na poczatku, zeby zachecic do dodania nowej konfiguracji
		  if(newData || promptForInitConfig || deviceDisconnected){
		  	  //Wyslij sygnal do taska od prezentacji ze powinien teraz sie uruchomic
			  xTaskNotify(presentationTaskHandle, 0x02, eSetBits);
		  }
		  xSemaphoreGive(newDataMutexHandle);
		}
	}
}

void PresentationTaskThread(void const * argument)
{
	/* Podejscie: trzy taski - supervisor -> odczyt -> prezentacja: task prezentacji danych */
	uint32_t notifValue;
	/* Infinite loop */
	for(;;)
	{
		//Czekaj na sygnal od taska odczytu
		xTaskNotifyWait(pdFALSE, 0xFF, &notifValue, portMAX_DELAY);
		if((notifValue&0x02) != 0x00) //Sprawdza czy notifValue zawiera wartosc ktora wyslal task odczytu
		{
			updateReadoutValues();
			printConnectedDevicesTree();
		}
	}
}

//
void CommunicationTaskThread(void const * argument)
{
	char receivedData[50];
	/* Infinite loop */
	for(;;)
	{
		xQueueReceive(msgQueueHandle, receivedData, DELAY_TIME); //delayTime?
		xSemaphoreTake(uartMutexHandle, DELAY_TIME); //delayTime?
		printf(receivedData);
		xSemaphoreGive(uartMutexHandle);
	}
}
/* USER CODE END 6 */

/* USER CODE BEGIN 7 */
void delayMicroseconds(uint32_t us){
	//Average, experimental time for 1 rotation of the 'for' loop with nops: ~140ns
	//for an 80MHz processor@max speed; that gives ~7.143 loop rotations for 1 ms
	//Use this fact and the processor frequency to adjust the loop counter value for any processor speed
	uint32_t clockFreq = HAL_RCC_GetHCLKFreq();	//Current processor frequency
	float clockFreqRel = clockFreq/(float)80000000.0;//Current processor frequency relative to base of 80MHz
	uint32_t loopCounter = (us > 0 ? (uint32_t)(us*clockFreqRel*7.143) : (uint32_t)(clockFreqRel*7.143));
	//uint32_t loopCounter = (us > 0 ? (uint32_t)(us*7.143) : 7); //A minimum delay of 1 us - 80MHz only
	for(uint32_t tmp = 0; tmp < loopCounter; tmp++) {asm("nop");}
	//previously there was tmp < 800 giving 3200 processor cycles, each lasting 12.5 ns = 40 us delay
	//UINT_MAX	Maximum value for a variable of type unsigned int	4,294,967,295 (0xffffffff)
}

//Parametry: do ktorego urzadzenia (indeks w foundDevices[]) wysylamy konf., typ sensora (DHT22), interwal odczytu, nazwa sensora
void prepareNewConfig(MessageType msg_type, uint8_t deviceInd, uint8_t sensorType, uint16_t interval, uint8_t *name){
	/* Format wiadomosci: <typ_sensora:1B> <interwal:2B> <nazwa:max.12B> */
	memset(sentConfigurationMsg, 0x0, sizeof(sentConfigurationMsg));
	newConfig = true; int i=0;
	//TODO: jesli server dostanie 0xFF jako typ sensora, to ma wiedziec ze ma usunac ten czujnik
	sentConfigurationMsg[0] = (msg_type == ADD_SENSOR ? sensorType : 0xFF);
	sentConfigurationMsg[1] = interval/16; //16 nie 256!
	sentConfigurationMsg[2] = interval%16;
	for(i=3; name[i-3] != '\0' && i<MSG_LEN; i++){ sentConfigurationMsg[i] = name[i-3]; }
	if(i < MSG_LEN){ sentConfigurationMsg[i] = '\0'; }
    uint8_t connSensorsCnt = foundDevices[deviceInd].connSensorsCount;
	if(msg_type == ADD_SENSOR){ //wpisywanie do foundDevices informacji o nowo wyslanym czujniku
		memcpy(foundDevices[deviceInd].connSensors[connSensorsCnt].sensorName, name, i-3);
		foundDevices[deviceInd].connSensors[connSensorsCnt].lastTempValue = 0.0F;
		foundDevices[deviceInd].connSensors[connSensorsCnt].lastHumidValue = 0.0F;
		foundDevices[deviceInd].connSensorsCount++;
	}
	else if(msg_type == DELETE_SENSOR){
		if(memcmp(name, foundDevices[deviceInd].connSensors[connSensorsCnt-1].sensorName, i-3) == 0){ //o ile sensor istnieje
			foundDevices[deviceInd].connSensorsCount--; //TODO: usuwanie tylko ostatniego sensora; dodac znajdowanie sensora po nazwie
		}
	}
	whichServerReceivesConfiguration = deviceInd;
}

bool checkIfTempSensorReadoutCorrect(uint32_t dataBits, uint8_t checksumBits){
	uint8_t value = ((dataBits >> 24) & 0xFF) + ((dataBits >> 16) & 0xFF) + ((dataBits >> 8) & 0xFF) + (dataBits & 0xFF);
	if(value == checksumBits)
		return true;
	return false;
}

void updateReadoutValues(void){
	xSemaphoreTake(newDataMutexHandle, DELAY_TIME);
	//Format jednej wiadomosci w dataBLE: nazwa_czujnika '\0' dane handle_do_pol
	char sensorName[MAX_NAME_LEN]; char deviceName[MAX_NAME_LEN]; int i;
	memset(sensorName, 0x00, sizeof(sensorName));
	memset(deviceName, 0x00, sizeof(deviceName));
	while(newData){
		newData--;
		for(i=0; dataBLE[newData][i] != '\0' && i<MAX_NAME_LEN; i++){ sensorName[i] = dataBLE[newData][i]; }
		uint32_t dataBits = (dataBLE[newData][i+1] << 24) + (dataBLE[newData][i+2] << 16)
						  + (dataBLE[newData][i+3] << 8)  + (dataBLE[newData][i+4]);
		uint8_t checksumBits = dataBLE[newData][i+5];
		if(checkIfTempSensorReadoutCorrect(dataBits, checksumBits)){
			//znajdz (przez connHandle - 2 ost. bajty w dataBLE[]) odp. urzadzenie, znajdz odp. czujnik
			//i zaktualizuj odczytane wartosci, wypisz cale drzewo klienta (wszystkie polaczone urz. i ich wszystkie sensory)
			uint16_t connHandle = (dataBLE[newData][i+6] << 8) | dataBLE[newData][i+7];
			for(int k=0; k<foundDevicesCount; k++){
				if(connHandle == foundDevices[k].connHandle){ //znajdz odp. urzadzenie
					for(int m=0; foundDevices[k].deviceName[m] != '\0' && m<MAX_NAME_LEN; m++){
						deviceName[m] = foundDevices[k].deviceName[m];
					}
					for(int m=0; m<foundDevices[k].connSensorsCount; m++){ //znajdz odp. czujnik
						if(strcmp((char *)sensorName, (char *)foundDevices[k].connSensors[m].sensorName) == 0){
							//aktualizuj odczytane wartosci czujnika
							foundDevices[k].connSensors[m].lastHumidValue
								= ((float)((dataBLE[newData][i+1] << 8) | dataBLE[newData][i+2])) / (float)10.0;
							foundDevices[k].connSensors[m].lastTempValue
								= ((float)((dataBLE[newData][i+3] << 8) | dataBLE[newData][i+4])) / (float)10.0;
						}
					}
				}
			}
		} //if(checkIfTempSensorReadoutCorrect(dataBits, checksumBits))
	} //while(newData)
	xSemaphoreGive(newDataMutexHandle);
}

void printConnectedDevicesTree(void){
	//wypisz cale drzewo polaczen klienta
	uint8_t len = 0; uint8_t pos = 0; char buf[120]; promptForInitConfig = false; deviceDisconnected = false;
	memset(uartData, 0x0, sizeof(uartData)); memset(buf, 0x0, sizeof(buf));
	printf("=====\r\nUrzadzenie centralne (adres "); len=sprintf(buf,"<h4>Lista urządzeń połączonych z urządzeniem centralnym (adres ");
	memcpy(uartData+pos, buf, len); pos += len;
	for(int p = 5; p > 0; p--){
		printf("%02X-", bdaddr[p]); len = sprintf(buf, "%02X-", bdaddr[p]); memcpy(uartData+pos, buf, len); pos += len;
	}
	printf("%02X)\r\nPolaczone urzadzenia peryferyjne:\r\n\r\n", bdaddr[0]);
	len = sprintf(buf, "%02X)</h4>", bdaddr[0]);
	memcpy(uartData+pos, buf, len); pos += len;
	HAL_UART_Transmit(&huart3, (uint8_t *)uartData, pos, TRANSMIT_TIME);
	memset(uartData, 0x0, sizeof(uartData)); memset(buf, 0x0, sizeof(buf)); len = 0; pos = 0;
	for(int k=0; k<foundDevicesCount; k++){
		printf("%d. Urzadzenie %s (adres ", k+1, foundDevices[k].deviceName);
		len = sprintf(buf, "<h5>%d. Urządzenie %s (adres ", k+1, foundDevices[k].deviceName);
		memcpy(uartData+pos, buf, len); pos += len;
		for(int p = 5; p > 0; p--){
			printf("%02X-", foundDevices[k].deviceAddress[p]); len = sprintf(buf, "%02X-", foundDevices[k].deviceAddress[p]);
			memcpy(uartData+pos, buf, len); pos += len;
		}
		printf("%02X)\r\n", foundDevices[k].deviceAddress[0]); len = sprintf(buf, "%02X)</h5>", foundDevices[k].deviceAddress[0]);
		memcpy(uartData+pos, buf, len); pos += len;
		if(foundDevices[k].connStatus == EXCHANGING_DATA){
			if(foundDevices[k].connSensorsCount == 0) {
				printf("To urządzenie nie otrzymało jeszcze żadnych konfiguracji czujników\r\n\r\n");
				len = sprintf(buf, "To urządzenie nie ma jeszcze dodanych żadnych czujników<br><br>");
				memcpy(uartData+pos, buf, len); pos += len;
			}
			HAL_UART_Transmit(&huart3, (uint8_t *)uartData, pos, TRANSMIT_TIME);
			memset(uartData, 0x0, sizeof(uartData)); memset(buf, 0x0, sizeof(buf)); len = 0; pos = 0;
			for(int m=0; m<foundDevices[k].connSensorsCount; m++){
				//nazwa czujnika
				printf("Czujnik %s", foundDevices[k].connSensors[m].sensorName);
				len =  sprintf(buf, "<h6>Czujnik <strong>%s</strong> ", foundDevices[k].connSensors[m].sensorName); memcpy(uartData+pos, buf, len);pos+=len;
				//pin do ktorego ma byc dolaczony czujnik
				int pinNumber = 0;
				switch(m){
				case 0:
					pinNumber = 4;
					break;
				case 1:
					pinNumber = 9;
					break;
				case 2:
					pinNumber = 10;
					break;
				default:
					break;
				}
				printf("\tpin PA%d\r\n", pinNumber);
				len = sprintf(buf, "- pin PA%d</h6>", pinNumber); memcpy(uartData+pos, buf, len); pos += len;
				//temperatura i wilgotnosc
				uint16_t humid = (uint16_t)foundDevices[k].connSensors[m].lastHumidValue;
				uint16_t temp  = (uint16_t)foundDevices[k].connSensors[m].lastTempValue;
				uint16_t humidDecimal = ((int)(foundDevices[k].connSensors[m].lastHumidValue*10))%10;
				uint16_t tempDecimal  = ((int)(foundDevices[k].connSensors[m].lastTempValue*10))%10;
				printf("Temperatura\t %hu,%huC\r\n", temp, tempDecimal);
				len = sprintf(buf, "<table class=\"dane_tabela\"><thead><tr><th>Temperatura</th><th>Wilgotność</th></tr></thead>");
				memcpy(uartData+pos, buf, len); pos += len;
				printf("Wilgotnosc\t %hu,%hu%%\r\n\r\n", humid, humidDecimal);
				len = sprintf(buf, "<tbody><tr><td>%hu,%hu°C</td><td>%hu,%hu%%</td></tr></tbody></table>", temp, tempDecimal, humid, humidDecimal);
				memcpy(uartData+pos, buf, len); pos += len;
				HAL_UART_Transmit(&huart3, (uint8_t *)uartData, pos, TRANSMIT_TIME);
				memset(uartData, 0x0, sizeof(uartData)); memset(buf, 0x0, sizeof(buf)); len = 0; pos = 0;
			}
		}//status == EXCHANGING_DATA
		if(foundDevices[k].connStatus == DISCONNECTED_AFTER_CONNECTION_CREATED){
			printf("Utracono połączenie - sprawdź stan urządzenia\r\n\r\n");
			len = sprintf(buf, "Utracono połączenie - sprawdź stan urządzenia<br><br>");
			memcpy(uartData+pos, buf, len); pos += len;
			HAL_UART_Transmit(&huart3, (uint8_t *)uartData, pos, TRANSMIT_TIME);
			memset(uartData, 0x0, sizeof(uartData)); memset(buf, 0x0, sizeof(buf)); len = 0; pos = 0;
		}//status == DISCONNECTED_AFTER_CONNECTION_CREATED
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART3){ //Odebrano wiadomosc z konfiguracja - wyciagnij info z wiadomosci i wyslij info do odp. servera
		//Format wiadomosci: (dodawanie) nazwa_urzadzenia \0 nazwa_czujnika \0 interwal \0; (usuwanie) '\1' i tak samo; (skan) samo '\2'
		uint8_t ind = 0; uint8_t deviceName[MAX_NAME_LEN]; uint8_t sensorName[MAX_NAME_LEN]; uint8_t intervalChar[MAX_NAME_LEN];
		uint16_t interval = 0; uint8_t whichCmd = uartRcv[0];
		memset(deviceName, 0, MAX_NAME_LEN); memset(sensorName, 0, MAX_NAME_LEN); memset(intervalChar, 0, MAX_NAME_LEN);

		//3 typy wiadomosci - dodac sensor, usunac go albo szukac urzadzen; wiadomosc o usunieciu zaczyna sie "\1", o skanowaniu "\2"
		if(whichCmd != '\1' && whichCmd != '\2'){ //dodawanie sensora o podanej nazwie
			for(; ind < MAX_NAME_LEN && uartRcv[ind] != '\0'; ind++) { } //znajdz indeks pierwszego \0 (dla nazwy urzadzenia)
			memcpy(deviceName, uartRcv, ind); uint8_t start = ++ind;
			for(; ind < start+MAX_NAME_LEN && uartRcv[ind] != '\0'; ind++) { } //znajdz indeks drugiego \0 (dla nazwy czujnika)
			memcpy(sensorName, uartRcv+start, ind-start); start = ++ind;
			for(; ind < start+MAX_NAME_LEN && uartRcv[ind] != '\0'; ind++) { } //znajdz indeks trzeciego \0 (dla interwalu)
			memcpy(intervalChar, uartRcv+start, ind-start);	if(ind-start == 1) { interval = intervalChar[0]-'0'; }
			else { //przeksztalcenie stringa typu 123 na liczbe
				uint8_t end = --ind;
				for(; ind >= start; ind--) { interval += (intervalChar[ind-start]-'0')*(pow(10, end-ind)); }
			}
			printf("Received configuration: device name \"%s\" sensor name \"%s\" interval %hhu\r\n\r\n",deviceName,sensorName,interval);
		} //dodawanie sensora
		else if(whichCmd == '\1') { //usuwanie sensora
			ind = 1;
			for(; ind <= MAX_NAME_LEN && uartRcv[ind] != '\0'; ind++) { } //znajdz indeks pierwszego \0 (dla nazwy urzadzenia)
			memcpy(deviceName, uartRcv+1, ind-1); uint8_t start = ++ind;
			for(; ind <= start+MAX_NAME_LEN && uartRcv[ind] != '\0'; ind++) { } //znajdz indeks drugiego \0 (dla nazwy czujnika)
			memcpy(sensorName, uartRcv+start, ind-start);
			printf("Received message to delete sensor %s from device %s\r\n\r\n", sensorName, deviceName);
		}
		else if(whichCmd == '\2'){ //skanowanie urzadzen
			//TODO
			printf("Scanning for new devices!\r\n\r\n");
			//POMYSL 1: obsluzyc procedure laczenia normalnie (kazde znal. urz. jako nowe), w User_Process()
			//= najpierw odp. ustawic wszystkie zmienne globalne i odpalic User_Process()
			prepareForScanning();
			//User_Process() odpala sie samo przy nastepnym obrocie StartDefaultTask
		}

		//Wysylanie konfiguracji do odp. urzadzenia - przy dodawaniu i usuwaniu czujnikow
		if(whichCmd != '\2'){
			uint8_t devInd = 0;
			for(; devInd<foundDevicesCount; devInd++){ //Znajdz urzadzenie o odp. nazwie
				if(memcmp(deviceName, foundDevices[devInd].deviceName, MAX_NAME_LEN) == 0) { break; }
			}
			if(devInd == foundDevicesCount){ //Nie znaleziono urzadzenia
				printf("Device with given name \"%s\" is not connected - configuration will not be sent\r\n\r\n", deviceName);
			}
			else if(foundDevices[devInd].connStatus != EXCHANGING_DATA){ //Urzadzenie rozlaczone lub nieskonfigurowane
				printf("Device with given name \"%s\" not ready to exchange data - configuration will not be sent\r\n\r\n", deviceName);
			}
			else {
				if(uartRcv[0] != '\1'){ prepareNewConfig(ADD_SENSOR, devInd, DHT22, interval, sensorName); }
				else { prepareNewConfig(DELETE_SENSOR, devInd, DHT22, interval, sensorName); }
			}
		}

		//Ponowne wlaczenie nasluchiwania
		memset(uartRcv, 0, sizeof(uartRcv));
		HAL_UART_Receive_IT(&huart3, uartRcv, RCV_CONFIG_MSG_LEN);
	}
}

void prepareForScanning(void){
	//Ustaw odp. wartosci zmiennych (dla User_Process()), aby mozna bylo zrobic kolejne skanownanie na zyczenie uzytkownika
	discovery_started = false;
	set_connectable = true;
	discovery_finished = false;
	all_servers_connected = false;
	pairing_started = false;
	pairing_finished = false;
	start_read_tx_char_handle = false;
	all_tx_char_handles_read = false;
	start_read_rx_char_handle = false;
	all_rx_char_handles_read = false;
	start_notifications_enable = false;
	all_notifications_enabled = false;
	client_ready = false;
}

/* USER CODE END 7 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM3) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
