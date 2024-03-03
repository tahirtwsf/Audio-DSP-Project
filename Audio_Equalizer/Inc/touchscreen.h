/*
 * touchscreen.h
 *
 *  Created on: Nov. 27, 2022
 *      Author: Richard Dansereau
 */

#ifndef INC_TOUCHSCREEN_H_
#define INC_TOUCHSCREEN_H_

extern TS_Init_t hTS;
extern uint32_t TS_input_state;
extern __ALIGN_END const unsigned char pianokeyboardmask_bmp_start[];

void Touchscreen_Init(void);                /* Initialize touchscreen */
void Touchscreen_Deactivate(void);          /* Deactivate touchscreen */
void BSP_TS_Callback(uint32_t Instance);    /* Set TS_input_state */
void Touchscreen_Clear_Input_State(void);   /* Clear TS_input_state */
uint8_t Touchscreen_Check_User_Touch(void); /* Check TS_input_state*/

/* Check touch events on the sliders */
void Touchscreen_Input(uint16_t *slider_num, uint16_t *ypos, float32_t *slider_val, uint16_t *waveform);

#endif /* INC_TOUCHSCREEN_H_ */
