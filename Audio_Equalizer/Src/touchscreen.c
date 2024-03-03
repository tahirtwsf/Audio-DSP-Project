/*
 * touchscreen.c
 *
 *  Created on:  Aug. 15, 2022
 *  Last update: Nov. 27, 2023
 *      Author: Richard Dansereau
 *      Modified by: Tahir Towsif
 */

#include "main.h"
#include "touchscreen.h"

TS_Init_t hTS;                            // handle for touchscreen
//TS_MultiTouch_State_t  TS_MTState = {0};  // Not used (yet)
uint32_t TS_input_state = 0;              // User input state for touchscreen


/**
  * @brief  Initialize touchscreen
  * @param  None (assumed instance of 0)
  * @retval None
  */
void Touchscreen_Init(void)
{
	TS_input_state = 0;
	uint32_t x_size, y_size;

	/* Get size of touchscreen */
	BSP_LCD_GetXSize(0, &x_size);
	BSP_LCD_GetYSize(0, &y_size);

	/* Set parameters for touchscreen handle */
	hTS.Width = x_size;
	hTS.Height = y_size;
	hTS.Orientation = TS_SWAP_XY;
	hTS.Accuracy = 0;

	/* Touchscreen initialization */
	BSP_TS_Init(0, &hTS);

	/* Enable interrupts for touchscreen */
	BSP_TS_EnableIT(0);

	return;
}


/**
  * @brief  Deactivate touchscreen interrupts and deinitialize
  * @param  None (assumed instance of 0)
  * @retval None
  */
void Touchscreen_Deactivate(void)
{
	BSP_TS_DisableIT(0);   // Do these actually work?
	BSP_TS_DeInit(0);

	return;
}


/**
  * @brief  Touchscreen Callback (Sets user touch state)
  * @param  Instance TS instance. Can be only 0.
  * @retval None
  */
void BSP_TS_Callback(uint32_t Instance)
{
	TS_input_state = 1;
}


/**
  * @brief  Clear the user touch state
  * @param  None
  * @retval None
  */
void Touchscreen_Clear_Input_State(void)
{
	TS_input_state = 0;

	return;
}


/**
  * @brief  Check for user input
  * @param  None
  * @retval Input state (1 : active / 0 : Inactive)
  */
uint8_t Touchscreen_Check_User_Touch(void)
{
	return TS_input_state;
}


void Touchscreen_Input(uint16_t *slider_num, uint16_t *ypos, float32_t *slider_val, uint16_t *waveform)
{
	uint16_t xpos[5] = {130, 210, 290, 370, 450};
	uint16_t width = 51;
	*slider_num = 0;
	*waveform = 0;

	/* Check if the touchscreen was touched */
	if( Touchscreen_Check_User_Touch() > 0 )
	{
		TS_State_t TS_State;

		/* Get position of the single touch on the touchscreen */
		BSP_TS_GetState(0, &TS_State);

		/* Clear the touch input state */
		Touchscreen_Clear_Input_State();
		*ypos = TS_State.TouchY;

		for (uint16_t i = 0; i < 5; i++)
			if ( TS_State.TouchX >= xpos[i]-25 && TS_State.TouchX < xpos[i] + width )
			{
				*slider_num = i + 1;
				*slider_val = 9.0 * (225.0f - ((int16_t) TS_State.TouchY - 22)) / 225.0f;
				if ( *slider_val > 9.0 )
				   *slider_val = 9.0;
				if ( *slider_val < 0.0 )
				   *slider_val = 0.0;

				return;
			}

		for (uint16_t i = 0; i < 3; i++)
		{
			if ( TS_State.TouchX >= 0 && TS_State.TouchX < 96 )
			{
			   if ( TS_State.TouchY >= 10-3 && TS_State.TouchY < 10-3+36+6 )
				   *waveform = 1;   // square wave
			   if ( TS_State.TouchY >= 10-3+36+50 && TS_State.TouchY < 10-3+36+50+36 )
				   *waveform = 2;   // sawtooth wave
			   if ( TS_State.TouchY >= 10-3+2*(36+50) && TS_State.TouchY < 10-32*(36+50)+36 )
				   *waveform = 3;   // sine wave
			}
		}
	}

	return;
}

