/*
This file contains the functions you will modify in order to complete the labs.
These functions allow you to perform DSP on a per-frame or per-sample basis.
When processing on a per-sample basis, there are separate functions for left and right channels.
When processing on a per-frame basis, the left and right channels are interleaved.
The sample rate and frame size can be modified in lab.h.
You can also configure which of the four functions are active in lab.h

		Developed by Dan Jacobellis, University of Texas at Austin
		Modified by Richard Dansereau, Carleton University, 2023
            Modified by Tahir Towsif, Carleton University, 2023
*/

#include <malloc.h>
#include "main.h"
#include "lab.h"

// These functions allow estimation of the number of elapsed clock cycles
extern void tic(void);
extern uint32_t toc(void);

#ifdef ENABLE_VISUALIZATION
	// Variables used for the spectrum visualization
	extern arm_rfft_fast_instance_f32 fft_inst;
	extern float32_t fft_in[CHANNEL_SIZE];
	extern float32_t fft_out[CHANNEL_SIZE];
	extern float32_t fft_mag[CHANNEL_SIZE/2];  // Only need positive half of FFT magnitude
#endif /* ENABLE_VISUALIZATION */

// Declare variables local to this file
uint32_t elapsed_cycles;

extern uint16_t new_frame;

#define IIR_ORDER 10
#define IIR_NUMSTAGES (IIR_ORDER / 2)

arm_biquad_casd_df1_inst_f32 S[10];      // Filter instances
float32_t pState[10][4 * IIR_NUMSTAGES]; // Filter states

float32_t input_f32[CHANNEL_SIZE];       // 32-bit floating point input buffer for filters
float32_t output_f32[CHANNEL_SIZE];      // 32-bit floating point output buffer for filters

extern float32_t sa_state[10];           // spectrum analyzer (SA) state used to display visual bars
extern float32_t trim_val;               // spectrum analyzer (SA) trim value from slider for adjusting SA gain

static float32_t octFilt1[5 * IIR_NUMSTAGES] = {
      0.00145617521056737071, 0.00000000000000000000, -0.00145617521056737071, 1.99707053981247351082, -0.99708764957886530933,
      0.00145764077273193124, 0.00000000000000000000, -0.00145764077273193124, 1.99937778160153345475, -0.99938662109571507752,
      0.00145764077273193124, 0.00000000000000000000, -0.00145764077273193124, 1.99877853580403486156, -0.99881172025753728327,
      0.00145658005291304251, 0.00000000000000000000, -0.00145658005291304251, 1.99812831889683883979, -0.99813947767210575712,
      0.00145658005291304251, 0.00000000000000000000, -0.00145658005291304251, 1.99712092970974719464, -0.99714717873942593229};

static float32_t octFilt2[5 * IIR_NUMSTAGES] = {
      0.00290125292654364172, 0.00000000000000000000, -0.00290125292654364172, 1.99412947766114201364, -0.99419749414691283018,
      0.00290707202751546920, 0.00000000000000000000, -0.00290707202751546920, 1.99874133584578594913, -0.99877651573508763327,
      0.00290707202751546920, 0.00000000000000000000, -0.00290707202751546920, 1.99749845759174493587, -0.99763048831768341707,
      0.00290285947985342635, 0.00000000000000000000, -0.00290285947985342635, 1.99624680421282496567, -0.99629118696729235261,
      0.00290285947985342635, 0.00000000000000000000, -0.00290285947985342635, 1.99421160537957908865, -0.99431595615241863406};

static float32_t octFilt3[5 * IIR_NUMSTAGES] = {
      0.00577214194579804291, 0.00000000000000000000, -0.00577214194579804291, 1.98818571954230205456, -0.98845571610840399224,
      0.00579516802443059541, 0.00000000000000000000, -0.00579516802443059541, 1.99742030729664032940, -0.99756027432316030090,
      0.00579516802443059541, 0.00000000000000000000, -0.00579516802443059541, 1.99475293290546296099, -0.99527792021367600395,
      0.00577847873655525437, 0.00000000000000000000, -0.00577847873655525437, 1.99243710491649328453, -0.99261346859937527043,
      0.00577847873655525437, 0.00000000000000000000, -0.00577847873655525437, 1.98827672514260167524, -0.98869097307422038057};

static float32_t octFilt4[5 * IIR_NUMSTAGES] = {
      0.01145153192530310761, 0.00000000000000000000, -0.01145153192530310761, 1.97602823684500528145, -0.97709693614939385764,
      0.01154217301614529970, 0.00000000000000000000, -0.01154217301614529970, 1.99458116122095763245, -0.99513768540339764268,
      0.01154217301614529970, 0.00000000000000000000, -0.01154217301614529970, 1.98851660035293686946, -0.99060145123197540507,
      0.01147637075246498231, 0.00000000000000000000, -0.01147637075246498231, 1.98461547974144503215, -0.98531500176321740181,
      0.01147637075246498231, 0.00000000000000000000, -0.01147637075246498231, 1.97592304865200452113, -0.97756285146148924614};

static float32_t octFilt5[5 * 3] = {0.00001178181997870213, -0.00000000000189186586, -0.00001178195885334840, 1.95063718392204954455, -0.95482801920233584259,
		1.00000000000000000000, 1.99999418670581263768, 0.99999418673960571713, 1.96273873157487610541, -0.97051358858460345935,
		1.00000000000000000000, -1.99999402613080334845, 0.99999402616648991327, 1.98155218272728306239, -0.98386213763834562407};

static float32_t octFilt6[5 * IIR_NUMSTAGES] = {
      0.04411243406136240719, 0.00000000000000000000, -0.04411243406136240719, 1.89540803000165780645, -0.91177513187727510235,
      0.04545675645927165637, 0.00000000000000000000, -0.04545675645927165637, 1.97200587149114392460, -0.98075674169759952115,
      0.04545675645927165637, 0.00000000000000000000, -0.04545675645927165637, 1.93068277682861788591, -0.96318904933721460893,
      0.04447202507337371519, 0.00000000000000000000, -0.04447202507337371519, 1.93187977423559753554, -0.94272158848219778093,
      0.04447202507337371519, 0.00000000000000000000, -0.04447202507337371519, 1.88853194159457538071, -0.91364463974091547627};

static float32_t octFilt7[5 * 3] = {0.00066420341446451385, -0.00000000010665446974, -0.00066421124356174993, 1.76762204051151261552, -0.83014229035036468662,
		1.00000000000000000000, 1.99999418670581263768, 0.99999418673960571713, 1.76963908735979780218, -0.88774032076720355011,
		1.00000000000000000000, -1.99999402613080334845, 0.99999402616648991327, 1.90072900036738268170, -0.93661477090359435138};

static float32_t octFilt8[5 * IIR_NUMSTAGES] = {
      0.15660184034982840839, 0.00000000000000000000, -0.15660184034982840839, 1.46035541876931329597, -0.68679631930034323872,
      0.17320488064444117260, 0.00000000000000000000, -0.17320488064444117260, 1.79068384825854143294, -0.92397341156197299039,
      0.17320488064444117260, 0.00000000000000000000, -0.17320488064444117260, 1.39555312624029270729, -0.86614034758191338348,
      0.16070809683691975045, 0.00000000000000000000, -0.16070809683691975045, 1.62976661305432868332, -0.78612771513630308462,
      0.16070809683691975045, 0.00000000000000000000, -0.16070809683691975045, 1.35412571972493145367, -0.69947838897228098443};

static float32_t octFilt9[5 * IIR_NUMSTAGES] = {
      0.27725904149648722408, 0.00000000000000000000, -0.27725904149648722408, 0.69892681816545332296, -0.44548191700702549634,
      0.32605790246167926094, 0.00000000000000000000, -0.32605790246167926094, 1.35218784623164145486, -0.84354034816301670663,
      0.32605790246167926094, 0.00000000000000000000, -0.32605790246167926094, 0.21065779121308148003, -0.77938917800484430742,
      0.28852062599672401166, 0.00000000000000000000, -0.28852062599672401166, 1.05598394190045374863, -0.59242432971914260609,
      0.28852062599672401166, 0.00000000000000000000, -0.28852062599672401166, 0.37757678937438665656, -0.50265978452936221910};

static float32_t octFilt10[5 * IIR_NUMSTAGES] = {
      0.47271397856244212754, -0.00000000000000000000, -0.47271397856244212754, -0.83304529055760856426, -0.05457204287511568941,
      0.58384787667483839524, 0.00000000000000000000, -0.58384787667483839524, -1.85305154290621132418, -0.89624829421816842867,
      0.58384787667483839524, 0.00000000000000000000, -0.58384787667483839524, 0.14165689880724130489, -0.59136398148223023874,
      0.49696277312738701415, 0.00000000000000000000, -0.49696277312738701415, -1.65836944216376869221, -0.70607645748503555794,
      0.49696277312738701415, 0.00000000000000000000, -0.49696277312738701415, -0.01679334310594271518, -0.15368986920490210202};


/*
This function will be called once before beginning the main program loop.
This is the best place to build a lookup table.
*/
void lab_init(int16_t* output_buffer)
{
   arm_biquad_cascade_df1_init_f32(&S[0], IIR_NUMSTAGES, octFilt1, pState[0]);
   arm_biquad_cascade_df1_init_f32(&S[1], IIR_NUMSTAGES, octFilt2, pState[1]);
   arm_biquad_cascade_df1_init_f32(&S[2], IIR_NUMSTAGES, octFilt3, pState[2]);
   arm_biquad_cascade_df1_init_f32(&S[3], IIR_NUMSTAGES, octFilt4, pState[3]);
   arm_biquad_cascade_df1_init_f32(&S[4], 3, octFilt5, pState[4]);   // Will need to modify later
   arm_biquad_cascade_df1_init_f32(&S[5], IIR_NUMSTAGES, octFilt6, pState[5]);
   arm_biquad_cascade_df1_init_f32(&S[6], 3, octFilt7, pState[6]);   // Will need to modify later
   arm_biquad_cascade_df1_init_f32(&S[7], IIR_NUMSTAGES, octFilt8, pState[7]);
   arm_biquad_cascade_df1_init_f32(&S[8], IIR_NUMSTAGES, octFilt9, pState[8]);
   arm_biquad_cascade_df1_init_f32(&S[9], IIR_NUMSTAGES, octFilt10, pState[9]);

   return;
}

/*
This function will be called each time a complete frame of data is recorded.
Modify this function as needed.

*/
void process_input_buffer(int16_t* input_buffer)
{
	return;
}

/*
This function provides access to the current input buffer and output buffer and is
useful for block-based processing and filtering of the input directly to the output.
The only caveat is that both the input buffer and output buffer are interleaved with
left and right samples (i.e., input_buffer[0] is first left channel sample,
input_buffer[1] is first right channel sample, input_buffer[2] is second left channel
sample, input_buffer[3] is second right channel sample, etc.).

What is returned in output_buffer will be sent to the left channel DAC.
Default behavior:
	1. Copy input to output without modification (passthrough)
	2. Estimate the number of cycles that have elapsed during the function call
*/
void process_input_to_output_buffer(int16_t* input_buffer, int16_t* output_buffer)
{
   // Copy input_buffer to output_buffer for audio pass-through
   for (uint32_t i_sample = 0; i_sample < BUFFER_SIZE; i_sample++)
      output_buffer[i_sample] = input_buffer[i_sample];

   // Grab left channel input, convert to float32_t, and store in input_f32
   for (uint32_t i_sample = 0; i_sample < BUFFER_SIZE; i_sample+=2)
      input_f32[i_sample/2] = ((float32_t)input_buffer[i_sample]);

   // Loop through the 10 bands of the filter bank
   for (uint8_t i = 0; i < 10; i++)
   {
      // Filter input_f32 (left channel) through filter i
      arm_biquad_cascade_df1_f32(&S[i], input_f32, output_f32, CHANNEL_SIZE);

      // Perform log of absolute average detection for the visual spectrum analyzer (sa)
      sa_state[i] = 0;
      for (uint32_t i_sample = 0; i_sample < CHANNEL_SIZE; i_sample++)
         sa_state[i] += fabs(output_f32[i_sample]);
      sa_state[i] = log(sa_state[i] / CHANNEL_SIZE);

      // Apply the trim value
      sa_state[i] = trim_val * sa_state[i];
   }

   return;
}

/*
This function provides access to each individual sample that is incoming on the left channel.
The returned value will be sent to the left channel DAC.
Default behavior:
	1. Copy input to output without modification (passthrough)
	2. Estimate the number of cycles that have elapsed during the function call
*/
int16_t process_left_sample(int16_t input_sample)
{
	return input_sample;
}

/*
This function provides access to each individual sample that is incoming on the left channel.
The returned value will be sent to the right channel DAC.
Default behavior:
	1. Copy input to output without modification (passthrough)
	2. Estimate the number of cycles that have elapsed during the function call
*/
int16_t process_right_sample(int16_t input_sample)
{
	return input_sample;
}

/*
This function provides another opportunity to access the frame of data
The default behavior is to leave the buffer unchanged (passthrough)
The buffer you see here will have any changes that occurred to the signal due to:
	1. the process_input_buffer function
	2. the process_left_sample and process_right_sample functions or process_input_to_output function
*/
void process_output_buffer(int16_t* output_buffer)
{
#ifdef ENABLE_VISUALIZATION
	// 1. Deinterleave the left and right channels
	// 2. Combine the two channels (by addition) into one signal
	// 3. Save the result to the fft_in buffer which will be used for the display
	// 4. The original audio buffer is left unchanged (passthrough)

	int16_t left_sample;
	int16_t right_sample;
	for (uint32_t i_sample = 0; i_sample < FRAME_SIZE/2; i_sample+=1)
	{
		left_sample = output_buffer[i_sample];
		i_sample +=1;
		right_sample = output_buffer[i_sample];
		fft_in[i_sample/2] =  (((float32_t) left_sample) + ((float32_t) right_sample))/2;
	}
	arm_rfft_fast_f32(&fft_inst, fft_in, fft_out, 0);
	arm_cmplx_mag_f32(fft_out, fft_mag, FRAME_SIZE/8);  // fft_out[0] and fft_out[1] are technically real already as DC and X[N/2}

	new_frame = 1;
#endif

	return;
}


