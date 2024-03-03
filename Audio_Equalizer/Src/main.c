/**
******************************************************************************
* @file    BSP/Src/main.c
* @author  MCD Application Team.
* 				Modified by Dan Jacobellis, Univ. of Texas at Austin, 2021
* 				Modified by Richard Dansereau, Carleton University, 2023
          Modified by Tahir Towsif, Carleton University, 2023
* @brief   Main program body
******************************************************************************
* @attention
*
* Copyright (c) 2019 STMicroelectronics.
* All rights reserved.
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this file except in compliance with the
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
*/


//This file contains most of the setup code to perform audio processing (see lab.c and lab.h)
//The main while loop calls the display_spectrum function, which visualizes the input signal.

#include "main.h"

typedef enum
{
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF = 1,
  BUFFER_OFFSET_FULL = 2,
}BUFFER_StateTypeDef;

uint32_t user_button_state = 0;
uint32_t audio_buffer_offset;
uint32_t t;
uint32_t* systick = (uint32_t*) 0xe000e018;
int16_t play_buffer[FRAME_SIZE] __attribute__((aligned (32)));
int16_t record_buffer[FRAME_SIZE] __attribute__((aligned (32)));
BSP_AUDIO_Init_t  audio_in_init;
BSP_AUDIO_Init_t  audio_out_init;

void audio_init(void);
void tic(void);
uint32_t toc(void);
static void SystemClock_Config(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
static void Error_Handler(void);


int main(void)
{
	MPU_Config();
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();
	HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
	BSP_LED_Init(LED1);
	BSP_LCD_Init(0, LCD_ORIENTATION_PORTRAIT);
	UTIL_LCD_SetFuncDriver(&LCD_Driver);
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

	audio_init();
	display_init();
	lab_init(play_buffer);

	while(1)
	{
		handle_events();
	}
}


void audio_init(void)
{
  audio_out_init.Device = AUDIO_OUT_DEVICE_HEADPHONE;
  audio_out_init.ChannelsNbr = 2;
  audio_out_init.SampleRate = SAMPLE_RATE;
  audio_out_init.BitsPerSample = AUDIO_RESOLUTION_16B;
  audio_out_init.Volume = 50;
  audio_in_init.Device = AUDIO_IN_DEVICE_ANALOG_MIC;
  audio_in_init.ChannelsNbr = 2;
  audio_in_init.SampleRate = SAMPLE_RATE;
  audio_in_init.BitsPerSample = AUDIO_RESOLUTION_16B;
  audio_in_init.Volume = 100;
  BSP_AUDIO_IN_OUT_Init_Ex(0, 0, &audio_in_init, &audio_in_init);
  BSP_AUDIO_IN_Record_Ex(0, (uint8_t*) record_buffer, FRAME_SIZE*2);
  BSP_AUDIO_OUT_Play(0, (uint8_t*) play_buffer, FRAME_SIZE*2);
  audio_buffer_offset  = BUFFER_OFFSET_NONE;
}

void tic(void)
{
	t = *systick;
}

uint32_t toc(void)
{
	return t - *systick;
}

/**
  * @brief  User Callback when first half of audio buffer has been filled.
  *         The input record_buffer is processed by the active lab.c functions and then
  *         passed on to the output play_buffer.
  * @param  Not used
* @retval Input state (1 : active / 0 : Inactive)
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance)
{
	// Invalidate first-half of buffer, but second argument is in bytes and FRAME_SIZE is for 16-bit words
    SCB_InvalidateDCache_by_Addr((uint32_t *) record_buffer, FRAME_SIZE);

    #ifdef PROCESS_INPUT_BUFFER
        process_input_buffer(record_buffer);
    #endif /* PROCESS_INPUT_BUFFER */

    #ifdef PROCESS_INPUT_TO_OUTPUT_BUFFER
        process_input_to_output_buffer(record_buffer, play_buffer);
    #else
        for(uint32_t i_sample = 0 ; i_sample < FRAME_SIZE/2; i_sample+=1)
        {
        #ifdef PROCESS_LEFT_SAMPLE
        	play_buffer[i_sample] = process_left_sample(record_buffer[i_sample]);
        #elif defined(PERIODIC_LOOKUP_TABLE)
        #else
            play_buffer[i_sample] = record_buffer[i_sample];
        #endif
    	i_sample +=1;
        #ifdef PROCESS_RIGHT_SAMPLE
        	play_buffer[i_sample] = process_right_sample(record_buffer[i_sample]);
        #elif defined(PERIODIC_LOOKUP_TABLE)
        #else
        	play_buffer[i_sample] = record_buffer[i_sample];
        #endif
        }
    #endif

    #ifdef PROCESS_OUTPUT_BUFFER
        process_output_buffer(play_buffer);
    #endif /* PROCESS_OUTPUT_BUFFER */

    // Clean first-half of buffer, but second argument is in bytes and FRAME_SIZE is for 16-bit words
    SCB_CleanDCache_by_Addr((uint32_t *)play_buffer, FRAME_SIZE);

    audio_buffer_offset = BUFFER_OFFSET_HALF;
}

/**
  * @brief  User Callback when second half of audio buffer has been filled.
  *         The input record_buffer is processed by the active lab.c functions and then
  *         passed on to the output play_buffer.
  * @param  Not used
* @retval Input state (1 : active / 0 : Inactive)
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance)
{
	// Invalidate second-half of buffer, but second argument is in bytes and FRAME_SIZE is for 16-bit words
    SCB_InvalidateDCache_by_Addr((uint32_t*) &record_buffer[FRAME_SIZE/2], FRAME_SIZE);

	#ifdef PROCESS_INPUT_BUFFER
		process_input_buffer(&record_buffer[FRAME_SIZE/2]);
	#endif /* PROCESS_INPUT_BUFFER */

    #ifdef PROCESS_INPUT_TO_OUTPUT_BUFFER
        process_input_to_output_buffer(&record_buffer[FRAME_SIZE/2], &play_buffer[FRAME_SIZE/2]);
    #else
	    for(uint32_t i_sample = FRAME_SIZE/2; i_sample < FRAME_SIZE; i_sample+=1)
	    {
	    	#ifdef PROCESS_LEFT_SAMPLE
		    	play_buffer[i_sample] = process_left_sample(record_buffer[i_sample]);
		    #elif defined(PERIODIC_LOOKUP_TABLE)
		    #else
		    	play_buffer[i_sample] = record_buffer[i_sample];
		    #endif
		    i_sample +=1;
		    #ifdef PROCESS_RIGHT_SAMPLE
		    	play_buffer[i_sample] = process_right_sample(record_buffer[i_sample]);
		    #elif defined(PERIODIC_LOOKUP_TABLE)
		    #else
		    	play_buffer[i_sample] = record_buffer[i_sample];
		    #endif
	    }
    #endif

	#ifdef PROCESS_OUTPUT_BUFFER
		process_output_buffer(&play_buffer[FRAME_SIZE/2]);
	#endif /* PROCESS_OUTPUT_BUFFER */

	// Clean second-half of buffer, but second argument is in bytes and FRAME_SIZE is for 16-bit words
    SCB_CleanDCache_by_Addr((uint32_t*) &play_buffer[FRAME_SIZE/2], FRAME_SIZE);

    audio_buffer_offset = BUFFER_OFFSET_FULL;
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 520000000 (CPU Clock)
  *            HCLK(Hz)                       = 260000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  130MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  130MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  130MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  130MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 104
  *            PLL_P                          = 1
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 3
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 104;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

/* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

/*
  Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
          (GPIO, SPI, FMC, OSPI ...)  when  operating at  high frequencies(please refer to product datasheet)
          The I/O Compensation Cell activation  procedure requires :
        - The activation of the CSI clock
        - The activation of the SYSCFG clock
        - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
*/


  __HAL_RCC_CSI_ENABLE() ;

  __HAL_RCC_SYSCFG_CLK_ENABLE() ;

  HAL_EnableCompensationCell();

}

/**
  * @brief  Check if user button has been pressed
  * @param  None
* @retval Input state (1 : active / 0 : Inactive)
  */
uint8_t Check_User_Button(void)
{
  return user_button_state;
}

/**
  * @brief  Clear user button variable
  * @param  None
* @retval None
  */
void Clear_User_Button(void)
{
	user_button_state = 0;

	return;
}

/**
  * @brief  Button Callback
  * @param  Button Specifies the pin connected EXTI line
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if(Button == BUTTON_USER)
  {
    user_button_state = 1;
  }
}

/**
  * @brief  CPU MPU Config.
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for OctoSPI RAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = OSPI_RAM_WRITE_READ_ADDR;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
