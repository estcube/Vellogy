/**
 * \file fft_freq.c
 * \brief Low, high and both frequencies compression functions
 * \author Quentin.C
 * \version 0.1
 * \date August 19th 2016
 *
 *
 */

#include "fft_freq.h"
#include "kiss_fft.h"
#include "kiss_fftr.h"

/**
 * \fn float* fftLow(float* array,unsigned int size)
 * \brief Compress the float array into frequency data and return an array with only Low frequencies.
 *
 * \param array Float Array to compress.
 * \param size Size of the array uncompressed.
 * \return An float array with only Low frequencies.
 */

float* fftLow(float* array,unsigned int size)
{
    //FFT LOW FREQ
    kiss_fft_cpx out_cpx[size],*cpx_buf;
    kiss_fftr_cfg fft = kiss_fftr_alloc(size*2 ,0 ,0,0);
    cpx_buf = copycpx(array,size);
    kiss_fftr(fft,(kiss_fft_scalar*)cpx_buf, out_cpx);

    int newSize = ((size+2)/2)+((size+2)/2)%2;
    float* newArray;
    newArray = (float*)malloc(newSize*sizeof(float));
    for(int i=0;i<newSize/2;i++)
    {
        newArray[i] = out_cpx[i].r;
    }
    int g = 0;
    for(int i=newSize/2;i<newSize;i++)
    {
        newArray[i] = out_cpx[g].i;
        g++;
    }

    free(fft);
    free(cpx_buf);
    return newArray;
}

/**
 * \fn float* fftHigh(float* array,unsigned int size)
 * \brief Compress the float array into frequency data and return an array with only High frequencies.
 *
 * \param array Float Array to compress.
 * \param size Size of the array uncompressed.
 * \return An float array with only High frequencies.
 */

float* fftHigh(float* array,unsigned int size)
{
    //FFT HIGH FREQ
    kiss_fft_cpx out_cpx[size],*cpx_buf;
    kiss_fftr_cfg fft = kiss_fftr_alloc(size*2 ,0 ,0,0);
    cpx_buf = copycpx(array,size);
    kiss_fftr(fft,(kiss_fft_scalar*)cpx_buf, out_cpx);  
    int newSize = ((size)/2)+((size)/2)%2;
    int stop = (size-1)/2+(size-1)%2+1;
    float *newArray;
    newArray = (float*)malloc(newSize*sizeof(float));
    int g =0;
    for(int i=stop-newSize/2;i<stop;i++)
    {
        newArray[g] = out_cpx[i].r;
        g++;
    }
    for(int i=stop-newSize/2;i<stop;i++)
    {
        newArray[g] = out_cpx[i].i;
        g++;
    }

    free(fft);
    free(cpx_buf);
    return newArray;
}

/**
 * \fn float* fftAll(float* array,unsigned int size)
 * \brief Transform the float array into frequency data and return an array with all frequencies.
 *
 * \param array Float Array to compress.
 * \param size Size of the array uncompressed.
 * \return An float array with all frequencies.
 */

float* fftAll(float* array,unsigned int size)
{
    //FFT ALL
    kiss_fft_cpx out_cpx[size],*cpx_buf;
    kiss_fftr_cfg fft = kiss_fftr_alloc(size*2 ,0 ,0,0);
    cpx_buf = copycpx(array,size);
    kiss_fftr(fft,(kiss_fft_scalar*)cpx_buf, out_cpx);  

    float* newArray;
    newArray = (float*)malloc((2*(size/2)+2)*sizeof(float));
    for(unsigned int i=0;i<size/2+1;i++)
    {
        newArray[i] = out_cpx[i].r;
    }
    int g = 0;
    for(unsigned int i=size/2+1;i<2*(size/2)+2;i++)
    {
        newArray[i] = out_cpx[g].i;
        g++;
    }

    free(fft);
    free(cpx_buf);
    return newArray;
}

/**
 * \fn float* ifftLow(float* newArray,unsigned int size)
 * \brief Transform the frequency array previously compressed by fftLow function into data array.
 *
 * \param newArray Float Array compressed by fftLow function.
 * \param size Size of the array UNcompressed.
 * \return An float arraywith Low frequency data of size "size"
 */

float* ifftLow(float* newArray,unsigned int size)
{
    //IFFT LOW FREQ
    float* dataOut;
    dataOut = (float*)malloc(size*sizeof(float));
    int isinverse = 1;
    kiss_fft_cpx out[size], new_out_cpx[size];
    kiss_fftr_cfg ifft = kiss_fftr_alloc(size*2,isinverse,0,0);
    unsigned int newSize = ((size+2)/2)+((size+2)/2)%2;
    for(unsigned int i=0;i<size+1;i++)
    {
        new_out_cpx[i].r = 0.;
        new_out_cpx[i].i = 0.;
    }
    for(unsigned int i=0;i<newSize/2;i++)
    {
        new_out_cpx[i].r = newArray[i];
        new_out_cpx[size-i].r = newArray[i];
        new_out_cpx[i].i = newArray[i+newSize/2];
        new_out_cpx[size-i].i = -newArray[i+newSize/2];  
    }
    kiss_fftri(ifft,new_out_cpx,(kiss_fft_scalar*)out );

    for(unsigned int i=0;i<size;i++)
    {
        dataOut[i] = (out[i].r)/(size*2);
    }
    free(newArray);
    free(ifft);
    return dataOut;
}

/**
 * \fn float* ifftHigh(float* newArray,unsigned int size)
 * \brief Transform the frequency array previously compressed by fftHigh function into data array.
 *
 * \param newArray Float Array compressed by fftHigh function.
 * \param size Size of the array UNcompressed.
 * \return An float array with High frequency data of size "size"
 */

float* ifftHigh(float* newArray,unsigned int size)
{
    //IFFT HIGH FREQ
    float* dataOut;
    dataOut = (float*)malloc(size*sizeof(float));
    int isinverse = 1;
    kiss_fft_cpx out[size], new_out_cpx[size];
    kiss_fftr_cfg ifft = kiss_fftr_alloc(size*2,isinverse,0,0);
    unsigned int newSize = ((size)/2)+((size)/2)%2;
    unsigned int stop = (size-1)/2+(size-1)%2;
    for(unsigned int i=0;i<size+1;i++)
    {
        new_out_cpx[i].r = 0.;
        new_out_cpx[i].i = 0.;
    }
    for(unsigned int i=0;i<newSize/2;i++)
    {
        new_out_cpx[stop-newSize/2+i+1].r = newArray[i];
        new_out_cpx[stop+newSize/2-i-1+size%2].r = newArray[i];
        new_out_cpx[stop+newSize/2-i-1+size%2].i = -newArray[i+newSize/2];
        new_out_cpx[stop-newSize/2+i+1].i = newArray[i+newSize/2];
    }
    kiss_fftri(ifft,new_out_cpx,(kiss_fft_scalar*)out );
    for(unsigned int i=0;i<size;i++)
    {
        dataOut[i] = (out[i].r)/(size*2);
    }
    free(newArray);
    free(ifft);
    return dataOut;
}

/**
 * \fn float* ifftAll(float* newArray,unsigned int size)
 * \brief Transform the frequency array previously compressed by fftAll function into data array.
 *
 * \param newArray Float Array compressed by fftAll function.
 * \param size Size of the array UNcompressed.
 * \return An float array with All frequency data of size "size"
 */

float* ifftAll(float* newArray,unsigned int size)
{
    //IFFT ALL
    float* dataOut;
    dataOut = (float*)malloc(size*sizeof(float));
    int isinverse = 1;
    kiss_fft_cpx out[size], new_out_cpx[size];
    kiss_fftr_cfg ifft = kiss_fftr_alloc(size*2,isinverse,0,0);
    
    for(unsigned int i=0;i<size/2+1;i++)
    {
        new_out_cpx[i].r = newArray[i];
        new_out_cpx[size-i].r = newArray[i];
        new_out_cpx[size-i].i = -newArray[i+size/2+1];
        new_out_cpx[i].i = newArray[i+size/2+1];

    }
    kiss_fftri(ifft,new_out_cpx,(kiss_fft_scalar*)out );

    for(unsigned int i=0;i<size;i++)
    {
        dataOut[i] = (out[i].r)/(size*2);
    }
    free(newArray);
    free(ifft);
    return dataOut;
}
