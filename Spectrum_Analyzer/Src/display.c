/*
 * display.c
 *
 *  Created on:   Sep. 10, 2022
 *  Last updated: Nov. 26, 2023
 *      Author: Richard Dansereau, Modified by: Tahir Towsif
 *      		Modified from code by Dan Jacobellis, Univ. of Texas at Austin, 2021
 */

#include "main.h"

#ifdef ENABLE_VISUALIZATION
/* This lookup table maps linear frequency bins to 1/36 octave bins */
uint16_t FFT_to_CQ_LUT[161] = { 4, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7,
		7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13,
		13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 20, 20, 21, 22, 23, 23,
		24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 38, 39, 40, 42, 43,
		45, 46, 48, 50, 51, 53, 55, 57, 59, 61, 63, 65, 68, 70, 73, 75, 78, 81,
		83, 86, 90, 93, 96, 99, 103, 107, 110, 114, 119, 123, 127, 132, 136,
		141, 146, 152, 157, 163, 168, 174, 181, 187, 194, 201, 208, 215, 223,
		231, 239, 248, 257, 266, 276, 286, 296, 306, 317, 329, 341, 353, 366,
		379, 392, 406, 421, 436, 452, 468, 485, 502, 520, 539, 558, 578, 599,
		621, 643, 666, 690, 715, 741, 767, 795, 824, 853, 884, 916, 949, 983 };

arm_rfft_fast_instance_f32 fft_inst;
float32_t fft_in[FRAME_SIZE/4];
float32_t fft_out[FRAME_SIZE/4];
float32_t fft_mag[FRAME_SIZE/8];
#endif /* ENABLE_VISUALIZATION */



uint16_t xpos_spectrogram = 0;

uint16_t new_frame = 0;
uint8_t pRGBData[272*4];

float32_t sa_state[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/**
  * @brief  Initialize anything needed for the display
  * @retval None
  */
void display_init(void)
{
#ifdef ENABLE_VISUALIZATION
	arm_rfft_fast_init_f32(&fft_inst, FRAME_SIZE/4);
#endif

	display_splash_page();
}


/**
  * @brief  Display the magnitude of an FFT
  * @retval None
  */
#ifdef ENABLE_VISUALIZATION
void display_spectrum(float32_t *fft_mag)
{
	uint32_t x1, x2;
	float32_t height;

	UTIL_LCD_SetFont(&Font12);
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
	UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);

	UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"<-- Press USER button to switch to user plot", LEFT_MODE);

	UTIL_LCD_DrawVLine(0, 256, 4, UTIL_LCD_COLOR_WHITE);  // 1 Hz
	UTIL_LCD_DisplayStringAt(1, 261, (uint8_t *)"1 Hz", LEFT_MODE);

	UTIL_LCD_DrawVLine(3 * 34, 256, 4, UTIL_LCD_COLOR_WHITE);  // 100 Hz
	UTIL_LCD_DisplayStringAt(3*34 - 10, 261, (uint8_t *)"100 Hz", LEFT_MODE);

	UTIL_LCD_DrawVLine(3 * 55, 256, 4, UTIL_LCD_COLOR_WHITE);  // 200 Hz
	UTIL_LCD_DisplayStringAt(3*55 - 10, 261, (uint8_t *)"200", LEFT_MODE);

	UTIL_LCD_DrawVLine(3 * 67, 256, 4, UTIL_LCD_COLOR_WHITE);  // 300 Hz
	UTIL_LCD_DisplayStringAt(3*67 - 10, 261, (uint8_t *)"300", LEFT_MODE);

	UTIL_LCD_DrawVLine(3 * 82, 256, 4, UTIL_LCD_COLOR_WHITE);  // 500 Hz
	UTIL_LCD_DisplayStringAt(3*82 - 10, 261, (uint8_t *)"500", LEFT_MODE);

	UTIL_LCD_DrawVLine(3 * 102, 256, 4, UTIL_LCD_COLOR_WHITE);  // 1000 Hz
	UTIL_LCD_DisplayStringAt(3*102 - 10, 261, (uint8_t *)"1k", LEFT_MODE);

	UTIL_LCD_DrawVLine(3 * 121, 256, 4, UTIL_LCD_COLOR_WHITE);  // 2000 Hz
	UTIL_LCD_DisplayStringAt(3*121 - 10, 261, (uint8_t *)"2k", LEFT_MODE);

	UTIL_LCD_DrawVLine(3 * 141, 256, 4, UTIL_LCD_COLOR_WHITE);  // 4000 Hz
	UTIL_LCD_DisplayStringAt(3*141 - 10, 261, (uint8_t *)"4k", LEFT_MODE);

	UTIL_LCD_DrawVLine(479, 256, 4, UTIL_LCD_COLOR_WHITE);  // 8000 Hz
	UTIL_LCD_DisplayStringAt(466, 261, (uint8_t *)"8k", LEFT_MODE);

	UTIL_LCD_DrawHLine(0, 256, 480, UTIL_LCD_COLOR_WHITE);

	for(uint32_t i_bar = 0; i_bar<160; i_bar+=1)
	{
		x1 = FFT_to_CQ_LUT[i_bar];
		x2 = FFT_to_CQ_LUT[i_bar+1];
		if (x1 == x2){height = 20*logf(fft_mag[x1]);}  // Should use log10f, but range is 20*logf(32767) = 208 is a good range for 270 pixel height LCD
		else
		{
			height = 0;
			for (uint32_t bin = x1; bin<x2; bin+=1)
			{
				height += fft_mag[bin];
			}
			height = 20*logf(height/(x2-x1+1));
		}

		height = (height - 80) * 2;
		if (height>243){height = 243;}
		if (height<0){height = 0;}
		UTIL_LCD_DrawVLine(3*i_bar, 12, 243, UTIL_LCD_COLOR_BLACK);
		UTIL_LCD_DrawVLine(3*i_bar, (uint32_t)(255-height), (uint32_t)height, UTIL_LCD_COLOR_BLUE);
	}
}
#endif /* ENABLE_VISUALIZATION */


/**
  * @brief  Display main splash page messages
  * @retval None
  */
void display_splash_page(void)
{
  uint32_t x_size;
  uint32_t y_size;

  UTIL_LCD_SetFont(&UTIL_LCD_DEFAULT_FONT);

  /* Clear the LCD */
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);

  /* Set the LCD Text Color */
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_RED);

  /* Display LCD messages */
  UTIL_LCD_DisplayStringAt(0, 5, (uint8_t *)"SYSC 4405:", CENTER_MODE);
  UTIL_LCD_DisplayStringAt(0, 30, (uint8_t *)"Digital Signal Processing", CENTER_MODE);

  /* Set the LCD Text Color */
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_DARKBLUE);

  /* Display LCD messages */
  UTIL_LCD_DisplayStringAt(0, 55, (uint8_t *)"Lab 6:", CENTER_MODE);
  UTIL_LCD_DisplayStringAt(0, 80, (uint8_t *)"Real-Time Spectrum Analyzer", CENTER_MODE);

  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  UTIL_LCD_SetFont(&Font12);
  UTIL_LCD_DisplayStringAt(0, y_size - 45, (uint8_t *)"Copyright (c) STMicroelectronics 2019", CENTER_MODE);
  UTIL_LCD_DisplayStringAt(0, y_size - 30, (uint8_t *)"Modified by Dan Jacobellis, Univ. of Texas at Austin, 2021", CENTER_MODE);
  UTIL_LCD_DisplayStringAt(0, y_size - 15, (uint8_t *)"Modified by Richard Dansereau, Carleton University, 2023", CENTER_MODE);

  UTIL_LCD_SetFont(&Font16);
  BSP_LCD_FillRect(0, 0, y_size/2 - 20, x_size, 16, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_DisplayStringAt(0, y_size/2 - 20, (uint8_t *)"<-- Press user button to start", CENTER_MODE);

  BSP_LCD_FillRect(0, 0, y_size/2 + 26, x_size, 16, UTIL_LCD_COLOR_LIGHTGREEN);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_GREEN);
  UTIL_LCD_DisplayStringAt(0, y_size/2 + 26, (uint8_t *)"Plug headphones/speaker into green jack", CENTER_MODE);

  BSP_LCD_FillRect(0, 0, y_size/2 + 42, x_size, 16, UTIL_LCD_COLOR_LIGHTBLUE);
  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_LIGHTBLUE);
  UTIL_LCD_DisplayStringAt(0, y_size/2 + 42, (uint8_t *)"Plug audio input into blue jack", CENTER_MODE);

  return;
}


void display_spectrogram(void)
{
//	UTIL_LCD_DrawVLine(0, 0, 272, UTIL_LCD_COLOR_WHITE);
//	UTIL_LCD_DrawHLine(0, 0, 480, UTIL_LCD_COLOR_WHITE);
//	UTIL_LCD_DrawVLine(479, 0, 272, UTIL_LCD_COLOR_WHITE);
//	UTIL_LCD_DrawHLine(0, 271, 480, UTIL_LCD_COLOR_WHITE);

	if ( xpos_spectrogram >= 480 ) {
		xpos_spectrogram = 0;

		/* Clear the LCD */
		UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
		UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);
	}

//	uint16_t tmp = xpos_spectrogram % 3;
//	switch( tmp )
//	{
//		case 0:
//			UTIL_LCD_DrawVLine(xpos_spectrogram, 0, 272, UTIL_LCD_COLOR_WHITE);
//			break;
//		case 1:
//			UTIL_LCD_DrawVLine(xpos_spectrogram, 0, 272, UTIL_LCD_COLOR_GREEN);
//			break;
//		case 2:
//			UTIL_LCD_DrawVLine(xpos_spectrogram, 0, 272, UTIL_LCD_COLOR_RED);
//			break;
//		default:
//			break;
//	}

	float32_t val;
	for (uint16_t i = 0; i < 272; i++)
	{
		val = 0.0;

		val = 20.0f * logf(fft_mag[i]);
		//for (uint8_t i2 = 0; i2 < 3; i2++)
			//val += 20.0f * logf(fft_mag[i * 3 + i2]) / 2.0f;

		pRGBData[i * 4] = val;
		pRGBData[i * 4 + 1] = val;
		pRGBData[i * 4 + 2] = val;
		pRGBData[i * 4 + 3] = val;
	}
	UTIL_LCD_FillRGBRect(xpos_spectrogram, 0, pRGBData, 1, 272);

	xpos_spectrogram++;
	new_frame = 0;

	return;
}


void draw_trim_scale(void)
{
	enum {xoffset = 20, yoffset = 30, barwidth = 41, vertspacing = 13};
	uint16_t xpos = 454;

	UTIL_LCD_DrawVLine(xpos, 15, 250, UTIL_LCD_COLOR_GREEN);

	for (uint16_t i = 0; i < 17; i++)
	{
		if (i % 2 == 0 )
			UTIL_LCD_DrawHLine(xpos - xoffset, yoffset + i * vertspacing, barwidth, UTIL_LCD_COLOR_GREEN);
		else
			UTIL_LCD_DrawHLine(xpos - xoffset / 2, yoffset + i * vertspacing, barwidth / 2, UTIL_LCD_COLOR_GREEN);
	}

	UTIL_LCD_DisplayStringAt(431, 0, (uint8_t *)"Trim", LEFT_MODE);

	return;
}


uint16_t slider_state;


void draw_slider(uint16_t ypos)
{
	uint16_t xpos = 429;

	slider_state = ypos;

	UTIL_LCD_FillRect(xpos, ypos, 50, 15, UTIL_LCD_COLOR_LIGHTMAGENTA);

	return;
}


void redraw_slider(uint16_t ypos)
{
	uint16_t xpos = 429;
	UTIL_LCD_FillRect(xpos, slider_state, 50, 15, UTIL_LCD_COLOR_BLACK);
	draw_trim_scale();
	if ( ypos < 30 )
		ypos = 30 - 8;
	if ( ypos > 30 + 13 * 16 - 7 )
		ypos = 30 + 13 * 16 - 7;
	draw_slider(ypos);

	return;
}


void display_spectrum_analyzer_init(void)
{
	enum {state_size = 10, w = 40, h= 20, wspace = 3, hspace = 4};
	char labels[10][5] = {"31.5", "63 ", "125 ", "250", "500", " 1k", " 2k", " 4k", " 8k", "16k"};
	int8_t label_xpos[10] = {0, 9, 2, 2, 2, -1, -1, -1, -1, 1};
	uint32_t xpos;

	UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);
	UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
	UTIL_LCD_SetFont(&UTIL_LCD_DEFAULT_FONT);
	UTIL_LCD_SetFont(&Font16);
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
	UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);

	for (uint8_t x = 0; x < state_size; x++)
	{
		xpos = x * (w + wspace) + label_xpos[x];
		UTIL_LCD_DisplayStringAt(xpos, 260, (uint8_t *)labels[x], LEFT_MODE);
	}

	draw_trim_scale();
	draw_slider(195);

	return;
}


void display_spectrum_analyzer(void)
{
	enum {state_size = 10, w = 40, h= 20, wspace = 3, hspace = 4};
	uint32_t bar_colors[10]= {UTIL_LCD_COLOR_LIGHTBLUE, UTIL_LCD_COLOR_LIGHTBLUE, UTIL_LCD_COLOR_LIGHTBLUE, UTIL_LCD_COLOR_LIGHTBLUE, UTIL_LCD_COLOR_LIGHTBLUE, UTIL_LCD_COLOR_LIGHTBLUE, UTIL_LCD_COLOR_LIGHTYELLOW, UTIL_LCD_COLOR_LIGHTYELLOW, UTIL_LCD_COLOR_LIGHTRED, UTIL_LCD_COLOR_LIGHTRED};
	uint32_t xpos, ypos;

	for (uint8_t x = 0; x < state_size; x++)
	{
		float32_t sa_tmp;
		if ( sa_state[x] < state_size )
			sa_tmp = sa_state[x];
		else
			sa_tmp = state_size;

		xpos = x * (w + wspace);

		for (uint8_t y = 0; y < sa_tmp; y++)
		{
			ypos = 230 - y * (h + hspace);
			UTIL_LCD_FillRect(xpos, ypos, w, h, bar_colors[y]);
		}
		for (uint8_t y = (uint8_t) sa_tmp; y < state_size; y++)
		{
			ypos = 230 - y * (h + hspace);
			UTIL_LCD_FillRect(xpos, ypos, w, h, UTIL_LCD_COLOR_BLACK);
		}
	}

	return;
}



