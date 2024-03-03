/*
 * events.c
 *
 *  Created on:   Sep. 10, 2022
 *  Last updated: Sep. 25, 2023
 *      Author: Richard Dansereau
		Modified by: Tahir Towsif
 */

#include <stdlib.h>
#include "main.h"
#include "touchscreen.h"


#ifdef ENABLE_VISUALIZATION
	// Variables used for the spectrum visualization
	extern float32_t fft_mag[CHANNEL_SIZE/2];  // Only need positive half of FFT magnitude
#endif /* ENABLE_VISUALIZATION */

#define PLOT_MAX_LEN 1000
float32_t plot_out[PLOT_MAX_LEN];  // Used in events.c and is plotted using display_plot()
uint16_t plot_len = 0;             // Length of current plot_out vector
int16_t plot_page = 1;
int16_t plot_max_page = 4;

// Start with splash screen displayed
uint8_t splash_screen_on = 1;
uint8_t first_display_spectrum = 0;
uint8_t display_spectrum_on = 0;
uint8_t first_display_FG = 0;
uint8_t display_FG_on = 0;
uint8_t spectrum_input_or_output = 0;     // Compute spectrumm for either input (0) or output (1)


// Need to do some fancy debouncing for the touch screen
uint32_t button_counter = 0;
int32_t prev_press = -1;

float32_t trim_val = 0.5;

uint16_t ypos_prev = -1;

float32_t freq = 0000.0f;       // frequency of generated signal (value between 0 to 9999 Hz)
float32_t amp = 9.0f;          // amplitude of generated output (0 to 9)
float32_t amp_scaling = 1000.0f;// multiplicative scaling factor for amplitude
uint16_t signalselect = 3;     // 0 = none, 1 = square wave, 2 = sawtooth wave, 3 = sine wave


extern void display_FG_init(void);
extern void display_FG_controls(void);
extern void redraw_info(void);
extern void display_FG_buttons(void);

extern uint16_t signalselect;
extern float32_t freq;
extern float32_t amp;

void handle_events(void)
{
	// If user button pressed, toggle display of the spectrum
	if ( Check_User_Button() )
	{
		// Toggle display spectrum variable
		if ( splash_screen_on || display_spectrum_on )
		{
			if ( spectrum_input_or_output == 0 )
			{
				spectrum_input_or_output = 1;
			}
			else
			{
				// Turn on displaying of plot and turn off all other displays
				display_FG_on = 1;
				first_display_FG = 0;

				display_spectrum_on = 0;
				splash_screen_on = 0;

				/* Set the LCD Text Color */
				UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
				UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);

				/* Display LCD messages */
				//UTIL_LCD_SetFont(&Font12);
				//UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"<-- Press USER button to switch to spectrum of audio", LEFT_MODE);
			}
			Touchscreen_Init();     // Should I do this only once?
		} else
		{
			display_spectrum_on = 1;
			first_display_spectrum = 0;
			spectrum_input_or_output = 0;

			display_FG_on = 0;
			splash_screen_on = 0;

			//Touchscreen_Deactivate();    // Deactivates USER button as well?
		}

		Clear_User_Button();   // Clear that the user button has been pressed
	}

	// Display function generator controls
	if ( display_FG_on )
	{
		if ( !first_display_FG ) {
			display_FG_init();
			first_display_FG = 1;
		}
		display_FG_controls();

		uint16_t slider_num = 0;
		float32_t slider_val = 0;
		uint16_t waveform = 0;
		uint16_t ypos;

		Touchscreen_Input(&slider_num, &ypos, &slider_val, &waveform);

		if ( slider_num != 0 )
		{
			char str1[5];
			char str2[2];
			sprintf(str1, "%04d", (uint16_t) round(freq));
			sprintf(str2, "%1d", (uint16_t) round(slider_val));

			if ( slider_num == 1 )
			{
				str1[0] = str2[0];
				freq = (float32_t) atoi(str1);
			}
			else if ( slider_num == 2 )
			{
				str1[1] = str2[0];
				freq = (float32_t) atoi(str1);
			}
			else if ( slider_num == 3 )
			{
				str1[2] = str2[0];
				freq = (float32_t) atoi(str1);
			}
			else if ( slider_num == 4 )
			{
				str1[3] = str2[0];
				freq = (float32_t) atoi(str1);
			}
			else if ( slider_num == 5 )
			{
				amp = slider_val;
			}

			redraw_info();
			redraw_slider(slider_num, ypos);
			lab_event_update(2);
		}
		if ( waveform > 0 )
		{
			//char str[2];
			//sprintf(str,"%1d",waveform);
			//UTIL_LCD_DisplayStringAt(4, 250, (uint8_t *)str, LEFT_MODE);
			signalselect = waveform;
            lab_event_update(1);
            display_FG_buttons();
		}
	}

	// Display the spectrum
	if ( display_spectrum_on )
	{
		/* Clear the screen if first time to display spectrum for this toggle */
		if ( !first_display_spectrum )
		{
			UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
			UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
			first_display_spectrum = 1;
		}
#ifdef ENABLE_VISUALIZATION
		// Note: #ifdef ENABLE_VISUALIZATION needs to be reworked throughout handle_events()
		display_spectrum(fft_mag);
#endif /* ENABLE_VISUALIZATION */
	}

	return;
}
