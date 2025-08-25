/*
 * WeatherServer.c
 *
 *  Created on: May 19, 2020
 *      Author: jrodden
 */


#include "WeatherServer.h"


/* USER CODE BEGIN PV */
bool LD1ON = false; // this variable will indicate if the LD3 LED on the board is ON or not
bool LD2ON = false; // this variable will indicate if our LD2 LED on the board is ON or not

char InputWeatherBuffer[WEATHER_BUFF_SIZE] = {0};
char OutputWeatherBuffer[WEATHER_BUFF_SIZE+1] = {0};// put null chars last one should never be overwritten


// just declaring the function for the compiler [= CGI #2 =]
const char* LedCGIhandler(int iIndex, int iNumParams, char *pcParam[],char *pcValue[]);

// in our SHTML file <form method="get" action="/leds.cgi"> [= CGI #3 =]
const tCGI LedCGI = { "/leds.cgi", LedCGIhandler };


// [= CGI #4 =]
tCGI theCGItable[2];

// just declaring SSI handler function [* SSI #1 *]
u16_t mySSIHandler(int iIndex, char *pcInsert, int iInsertLen);

// [* SSI #2 *]
#define numSSItags 3

// [* SSI #3 *]
char const *theSSItags[numSSItags] = { "tag1", "tag2", "weathertag" };


uint32_t adcbilgi[2];// for reading adc


/* USER CODE END PV */


// passed pointers to CubeMX globals
ADC_HandleTypeDef *hadc1_ptr;
DMA_HandleTypeDef *hdma_adc1_ptr;
UART_HandleTypeDef *huart2_ptr;
DMA_HandleTypeDef *hdma_usart2_rx_ptr;


/* USER CODE BEGIN 0 */

// the actual function for handling CGI [= CGI #5 =]
const char* LedCGIhandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	uint32_t i = 0;
	if (iIndex == 0)
	{
		//turning the LED lights off
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		// we put this variable to false to indicate that the LD2 LED on the board is not ON
		LD2ON = false;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
		// we put this variable to false to indicate that the LD* LED on the board is not ON
		LD1ON = false;
	}

	for (i = 0; i < iNumParams; i++)
	{
		if (strcmp(pcParam[i], "led") == 0)
		{
			if (strcmp(pcValue[i], "1") == 0)
			{
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
				// LD3 LED (red) on the board is ON!
				LD1ON = true;
			}
			else if (strcmp(pcValue[i], "2") == 0)
			{
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
				// LD2 LED (blue) on the board is ON!
				LD2ON = true;
			}
		}
	}
	// the extension .shtml for SSI to work
	return "/test.shtml";
}

// END [= CGI #5 =]

// function to initialize CGI [= CGI #6 =]
void myCGIinit(void)
{
	//add LED control CGI to the table
	theCGItable[0] = LedCGI;

	//give the table to the HTTP server
	http_set_cgi_handlers(theCGItable, 1);
} // END [= CGI #6 =]

// the actual function for SSI [* SSI #4 *]
u16_t mySSIHandler(int iIndex, char *pcInsert, int iInsertLen)
{

	if (iIndex == 0)
	{
		if (LD1ON == false)
		{
			char myStr1[] = "<input value=\"1\" name=\"led\" type=\"checkbox\">";
			strcpy(pcInsert, myStr1);
			return strlen(myStr1);
		}
		else if (LD1ON == true)
		{
			// since the LD3 red LED on the board is ON we make its checkbox checked!
			char myStr1[] =
			"<input value=\"1\" name=\"led\" type=\"checkbox\" checked>";
			strcpy(pcInsert, myStr1);
			return strlen(myStr1);
		}
	}
	else if (iIndex == 1)
	{
		if (LD2ON == false)
		{
			char myStr2[] = "<input value=\"2\" name=\"led\" type=\"checkbox\">";
			strcpy(pcInsert, myStr2);
			return strlen(myStr2);
		}
		else if (LD2ON == true)
		{
			// since the LD2 blue LED on the board is ON we make its checkbox checked!
			char myStr2[] =
			"<input value=\"2\" name=\"led\" type=\"checkbox\" checked>";
			strcpy(pcInsert, myStr2);
			return strlen(myStr2);
		}
	}
	else if (iIndex == 2)
	{
		if (LD2ON == false)
		{
//			osSemaphoreWait(OutBufferSemHandle, osWaitForever);
			strcpy(pcInsert, OutputWeatherBuffer);
			u16_t stringLength = strlen(OutputWeatherBuffer);
//			osSemaphoreRelease(OutBufferSemHandle);
			return stringLength;
		}
		else if (LD2ON == true)
		{
			// since the LD2 blue LED on the board is ON we make its checkbox checked!
			char myStr2[] = "Here is todays weather nice and blue!";
			strcpy(pcInsert, myStr2);
			return strlen(myStr2);
		}
	}
	return 0;
}


// function to initialize SSI [* SSI #5 *]
void mySSIinit(void)
{
	http_set_ssi_handler(mySSIHandler, (char const**) theSSItags,numSSItags);
}

/* USER CODE END 0 */

/* USER CODE BEGIN 2 */
void WeatherServerInit(	ADC_HandleTypeDef *hadc1, DMA_HandleTypeDef *hdma_adc1, UART_HandleTypeDef *huart2,	DMA_HandleTypeDef *hdma_usart2_rx)// todo rename as handlePtrs
{
	hadc1_ptr = hadc1;
	hdma_adc1_ptr = hdma_adc1;
	huart2_ptr = huart2;
	hdma_usart2_rx_ptr = hdma_usart2_rx;

	Ringbuf_init(huart2_ptr);

}
/* USER CODE END 2 */

/* USER CODE BEGIN 5 */
void WeatherServerDefaultTask()
{
	HAL_ADC_Start_DMA(hadc1_ptr, (uint32_t *) adcbilgi,1);  //not used atm
	httpd_init();// initializing the HTTPd [-HTTPd #2-]
	myCGIinit(); // initializing CGI  [= CGI #7 =]
	mySSIinit();  // initializing SSI [* SSI #6 *]

	/* Infinite loop */
	int index = 0;
	for(;;)
	{
//		  index = sprintf(InputWeatherBuffer, "ADC:%u\r\n", adcbilgi[0]);

	//	  HAL_UART_Transmit(&huart2, (uint8_t *) TxData, 10, TX_TIMEOUT);
		  if(IsDataAvailable())
		  {
			  int c = Uart_read();
			  Uart_write(c);
			  if(c >= 0)// write negative?
			  {
				  InputWeatherBuffer[index++] = c;
			  }
		  }
		  if(InputWeatherBuffer[index-1]== 0x0D || index >= WEATHER_BUFF_SIZE)
		  {
//			  osSemaphoreWait(OutBufferSemHandle, osWaitForever);
			  int j= 0;
			  for(int i = 0; i<index; i++)// copy WeatherBuffer
			  {
				  if(InputWeatherBuffer[i] >= ' ' && InputWeatherBuffer[i] <= '`')  //is visible ascii
				  {
					  OutputWeatherBuffer[j++] = InputWeatherBuffer[i];
				  }
			  }
			  OutputWeatherBuffer[j] = 0; // to make sure null termination.
//			  osSemaphoreRelease(OutBufferSemHandle);
			  index = 0;
			  //flush();// probably not could erase data todo
		  }

//		  osDelay(1);

	}

}

/* USER CODE END 5 */
