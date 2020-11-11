#include "idStack.h"

#ifndef H_FFTSTACK
#define H_FFTSTACK

/**
 * \file fftStack.h
 * \brief FftStack Functions declarations
 * \author Quentin.C
 * \version 0.1
 * \date August 30th 2016
 *
 * Functions used to initialize, deinitialize, Push and Pop FftStacks
 *
 */

/* HOW TO USE IT

1 - Initialize FftStack with fftInitialize() Function.
2 - Initialize FftElement with initializeFftElement(...) Function.
2 - Compress data in an IdStack and store the compressed data with the fftPush(...) Function.
3 - Pop data from the FftStack into a float array which could be send, with the fftPop(...) Function.
4 - Possible to compress with other methods and send (Like Zlib)
5 - Decompress the float array with the ifft(...) Function
6 - You can delete an FftElement with deinitializeFftElement(...) Funtion (not a requirement).
7 - Deinitialize FftStack with fftDeinitialize(...) Function.
*/

/**
 * \enum Erase_mode
 * \brief Possible modes when compress data is popped.
 *
 * We can chose to erase the data in the compressed architecture or to keep it inside.
 */
	enum Erase_mode
	{
		ERASE,KEEP
	};
	typedef enum Erase_mode Erase_mode;

/**
 * \enum Fft_type
 * \brief Existing Fft types. (ALL,LOW,HIGH)
 *
 * Used to compress and decompress according to the right method.
 */
	enum Fft_type
	{
		ALL,LOW,HIGH
	};

	typedef enum Fft_type Fft_type;

	typedef struct FftStack FftStack;
	typedef struct FftElement FftElement;
	typedef struct FftDataStack FftDataStack;

/**
 * \struct FftStack
 * \brief Contain FftElement
 *
 */

	struct FftStack
	{
		FftElement *first;   //pointer to the first FftElement
	};

/**
 * \struct FftElement
 * \brief Part of FftStack. Contain FftDataStack.
 *
 */

	struct FftElement
	{
		Id_type id;
		unsigned int startTime; //4
		unsigned int blocSize; //4 Very important parameter ...
		unsigned int sizeCompressedH; //4
		unsigned int sizeCompressedL; //4
		unsigned int timeInterval; //4
		unsigned int dataNumber;  //4    number of data in the FftElement.
		FftDataStack *fftDataStack; //8   pointer to the compressed dataStack.
		FftElement *next; //8   pointer to the next FftElement.
	};

/**
 * \struct FftDataStack
 * \brief Part of FftElement. Contain the frequency compressed data by blocs.
 *
 */

	struct FftDataStack
	{
		float *pointerHigh;   //pointer to the first float data of the high frequency compressed array
		float *pointerLow;     //pointer to the first float data of the low frequency compressed array
		FftDataStack *next;
	};

	FftStack* fftInitialize();
	void fftDeinitialize(FftStack* myFftStack);
	FftElement* fftPush(FftStack* myFftStack, IdStack* myIdStack, Id_type id,unsigned int stopTime);
	FftElement* initializeFftElement(FftStack *myFftStack, Id_type id, unsigned int blocSize);
	FftElement* searchFftElement(FftStack *myFftStack, Id_type id);
	int deinitializeFftElement(FftStack *myFftStack, Id_type id);
	int deinitializeFirstFftElement(FftStack *myFftStack);
	void printFftStack(FftStack *fftStack);
	void printFftDataStack(FftElement *fftElement);
	int blocNumberCount(FftElement* myfftElement,unsigned int startTime,unsigned int stopTime);
	float* fftPop(FftStack* myFftStack, Id_type id,unsigned int startTime,unsigned int stopTime, Erase_mode erase, Fft_type fft_type);
	float* ifft(float* array);

#endif
