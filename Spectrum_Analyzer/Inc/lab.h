/*
 * lab.h
 *
 *  	Developed by Dan Jacobellis, University of Texas at Austin, 2021
 * 		Modified by Richard Dansereau, Carleton University, 2022
 */

#ifndef INC_LAB_H_
   #define INC_LAB_H_

// LAB_DEBUG sets extra features for debugging
   //#define LAB_DEBUG

// These statements enable the corresponding functions in lab.c.
// If one or more of the statements are commented out, the function will no longer be called.
// Use this to disable any functions you are not using to reduce overhead.

// When PROCESS_INPUT_BUFFER or PROCESS_OUTPUT_BUFFER is defined, then the corresponding
// functions in lab.c will be executed.
// Use this to disable any functions you are not using to reduce overhead.
   #define PROCESS_INPUT_BUFFER
   #define PROCESS_OUTPUT_BUFFER

// When PROCESS_INPUT_TO_OUTPUT_BUFFER is defined, then the corresponding
// function in lab.c will be executed. If PROCESS_INPUT_TO_OUTPUT_BUFFER is not
// defined, then corresponding PROCESS_LEFT_SAMPLE and PROCESS_RIGHT_SAMPLE
// functions are executed. Only PROCESS_INPUT_TO_OUTPUT_BUFFER needs to be
// disabled or enabled since PROCESS_LEFT_SAMPLE and PROCESS_RIGHT_SAMPLE
// depend on PROCESS_INPUT_TO_OUTPUT_BUFFER not being defined.
// Use this to disable any functions you are not using to reduce overhead.
   #define PROCESS_INPUT_TO_OUTPUT_BUFFER

// These define if PROCESS_LEFT_SAMPLE and PROCESS_RIGHT_SAMPLE functions
// are called, but only if PROCESS_INPUT_TO_OUTPUT_BUFFER not defined.
// Use this to disable any functions you are not using to reduce overhead.
   #ifndef PROCESS_INPUT_TO_OUTPUT_BUFFER
      #define PROCESS_LEFT_SAMPLE
      #define PROCESS_RIGHT_SAMPLE
   #endif

// When PERIODIC_LOOKUP_TABLE is defined, the output signal will not be derived from the input.
// The process_left_sample, process_right_sample, and process_input_buffer functions are unused.
// Instead, whatever values are contained in the output buffer will repeatedly be sent to the DAC.
   //#define PERIODIC_LOOKUP_TABLE

// When ENABLE_VISIALIZATION is defined, the LCD will show the spectrum of the input signal.
// Disable this before modifying the process_input_buffer function in lab.c.
// See the display_spectrum function in main.c.
   #define ENABLE_VISUALIZATION

// FRAME_SIZE specifies the length of the circular buffer that is used for audio I/O.
// While data is collected on one half of the buffer, the user can process the other half.
// The spectrum visualization expects a frame size of 8192.
// Disable visualization before changing the frame size.
   #define FRAME_SIZE 8192U

// BUFFER_SIZE is half the FRAME_SIZE (i.e., process one half of the circular buffer).
// Note that the buffer will have two interleaved channels (i.e., stereo).
   #define BUFFER_SIZE 4096U

// CHANNEL_SIZE is half of BUFFER_SIZE since there are two channels (i.e., stereo).
// Note that you still need to deinterleave the input buffer and interleave the output buffer
// to a buffer with size BUFFER_SIZE.
   #define CHANNEL_SIZE 2048U

// SAMPLE_RATE applies to to both the input and output.
// Values of 48,000, 44,100 and 16,000 have been tested extensively.
// Other values may require additional clock configuration
   //#define SAMPLE_RATE 16000U
   //#define SAMPLE_RATE 44100U
   #define SAMPLE_RATE 48000U

// INPUT_LEVEL and OUTPUT_LEVEL correspond to the gain parameters for the ADCs and DACs
// Both should be integers between 0 and 255. A value of 192 corresponds to 0 dB.
// The default value results in roughly -20 dB of input gain and -48 dB of output gain.
// Increasing the value will add more bits of precision but will introduce other distortions.
// For more information, see the datasheet for the WM8994
   #define INPUT_LEVEL 138
   #define OUTPUT_LEVEL 63

// IN_SCALE_FACTOR and OUT_SCALE_FACTOR map the 16-bit ADC and DAC values to the interval [-1,1]
   #define INPUT_SCALE_FACTOR 0.0003125
   #define OUTPUT_SCALE_FACTOR 3200.0

   extern void lab_init(int16_t* output_buffer);

   extern void process_input_buffer(int16_t* input_buffer);
   extern void process_output_buffer(int16_t* output_buffer);

   extern void process_input_to_output_buffer(int16_t* input_buffer, int16_t* output_buffer);
   extern int16_t process_left_sample(int16_t input_sample);
   extern int16_t process_right_sample(int16_t input_sample);
#endif /* INC_LAB_H_ */
