/*
 * display.h
 *
 *  Created on: Nov. 13, 2022
 *      Author: Richard Dansereau
 *      		Based on code from Dan Dan Jacobellis, Univ. of Texas at Austin, 2021
 */

#ifndef INC_DISPLAY_H_
#define INC_DISPLAY_H_

extern void display_init(void);
extern void display_spectrum(float32_t *fft_mag);
extern void display_splash_page(void);
extern void redraw_slider(uint16_t slider_num, uint16_t ypos);


#endif /* INC_DISPLAY_H_ */
