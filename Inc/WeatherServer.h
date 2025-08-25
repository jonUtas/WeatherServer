/*
 * WeatherServer.h
 *
 *  Created on: May 19, 2020
 *      Author: jrodden
 */

#ifndef WEATHERSERVER_H_
#define WEATHERSERVER_H_

/* USER CODE BEGIN Includes */




// including httpd.h [- HTTPd #1 -]
#include "lwip/apps/httpd.h"
//#include "cmsis_os.h"
// we include String.h for the strcmp() function [= CGI #1 =]
#include <String.h>

// we include this library to be able to use boolean variables for SSI
#include <Stdbool.h>
/* USER CODE END Includes */

#define WEATHER_BUFF_SIZE 90


void WeatherServerInit(ADC_HandleTypeDef *hadc1, DMA_HandleTypeDef *hdma_adc1, UART_HandleTypeDef *huart2,	DMA_HandleTypeDef *hdma_usart2_rx);
void WeatherServerDefaultTask();


#endif /* WEATHERSERVER_H_ */
