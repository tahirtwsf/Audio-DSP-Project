/*
 * events.c
 *
 *  Created on:   Sep. 10, 2022
 *  Last updated: Nov. 13, 2023
 *      Author: Richard Dansereau
 *      Modified by: Tahir Towsif
 */

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
uint8_t display_SA_on = 0;
uint8_t first_display_SA = 0;

// Need to do some fancy debouncing for the touch screen
uint32_t button_counter = 0;
int32_t prev_press = -1;

float32_t trim_val = 0.5;

uint16_t ypos_prev = -1;

extern uint16_t new_frame;

extern void display_spectrogram(void);
extern void display_spectrum_analyzer_init(void);
extern void display_spectrum_analyzer(void);


void handle_events(void)
{
	// If user button pressed, toggle display of the spectrum
	if ( Check_User_Button() )
	{
		// Toggle display spectrum variable
		if ( splash_screen_on || display_spectrum_on )
		{
			// Turn on displaying of plot and turn off all other displays
			display_SA_on = 1;
			first_display_SA = 0;

			display_spectrum_on = 0;
			splash_screen_on = 0;

			/* Set the LCD Text Color */
			UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
			UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);

			/* Display LCD messages */
			//UTIL_LCD_SetFont(&Font12);
			//UTIL_LCD_DisplayStringAt(0, 0, (uint8_t *)"<-- Press USER button to switch to spectrum of audio", LEFT_MODE);

			Touchscreen_Init();     // Should I do this only once?
		} else
		{
			display_spectrum_on = 1;
			first_display_spectrum = 0;

			display_SA_on = 0;
			splash_screen_on = 0;

			//Touchscreen_Deactivate();    // Deactivates USER button as well?
		}

		Clear_User_Button();   // Clear that the user button has been pressed
	}

	// Display the plot
	if ( display_SA_on && new_frame )
	{
		if ( !first_display_SA ) {
			display_spectrum_analyzer_init();
			first_display_SA = 1;
		}
		display_spectrum_analyzer();
		new_frame = 0;

		uint16_t ypos;
		float32_t slider_val = 0.2;

		if ( Touchscreen_Trim(&ypos, &slider_val) )
		{
			if ( ypos != 0 && ypos != ypos_prev )
			{
				// slider_val has range [0,1]. Scale by trim_val
				trim_val = 2.5 * slider_val;

				redraw_slider(ypos);
				ypos_prev = ypos;
			}
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
