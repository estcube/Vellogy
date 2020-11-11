/**
 * \file fft_freq.h
 * \brief Low, high and both frequencies compression functions
 * \author Quentin.C
 * \version 0.1
 * \date August 19th 2016
 *
 *
 */

#ifndef FFT_FREQ_H
#define FFT_FREQ_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Return Low frequencies of FFT of array. Size is the number of elements of array.
 */
float* fftLow(float* array,unsigned int size);

/*
 * Return High frequencies of FFT of array. Size is the number of elements of array.
 */
float* fftHigh(float* array,unsigned int size);

/*
 * Return FFT of array. Size is the number of elements of array.
 */
float* fftAll(float* array,unsigned int size);

/*
 * Return data from the low frequencies array. Size if the number of elements of DATA array.
 */
float* ifftLow(float* array,unsigned int size);

/*
 * Return data from the High frequencies array. Size if the number of elements of DATA array.
 */
float* ifftHigh(float* array,unsigned int size);

/*
 * Return data from the frequencies array. Size if the number of elements of DATA array.
 */
float* ifftAll(float* array,unsigned int size);

#ifdef __cplusplus
}
#endif

#endif
