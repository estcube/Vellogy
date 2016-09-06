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

#include <stdio.h>
#include <stdlib.h>
#include "fftStack.h"
#include "fftFreq.h"


/**
 * \fn FftStack* fftInitialize()
 * \brief Function used to initialize a FftStack instance.
 *
 * This function have to be used before everything else fft function to initialise an instance where will be stored compressed Elements
 *
 * \return the initialized fftStack instance.
 */

FftStack* fftInitialize()
{
    FftStack *fftStack = (FftStack*) malloc(sizeof(*fftStack));
	if (fftStack == NULL)
    {
        perror("Error : Memory allocation for fftstack impossible");
		return NULL;
    }
    fftStack->first = NULL;
	return fftStack;
}

/**
 * \fn void fftDeinitialize(FftStack* myFftStack)
 * \brief Function used to deinitialize a FftStack instance.
 *
 * This function have to be used after everything else fft function to deinitialise an FftStack instance.
 *
 * \param myFftStack FftStack instance which have to be deinitialized.
 */

void fftDeinitialize(FftStack* myFftStack)
{
	if (myFftStack == NULL)
	{
		perror("Error : the FftStack should be initialied before\n");
		return;
	}
	else
    {
		while(myFftStack->first != NULL)
		{
			deinitializeFirstFftElement(myFftStack);
		}
	}
	free(myFftStack);
}


/**
 * \fn FftElement* fftPush(FftStack* myFftStack, IdStack* myIdStack, Id_type id,unsigned int stopTime)
 * \brief Transform and transfer a selected array of data into the compressed data architecture.
 *
 * \param myFftStack FftStack instance in which we want to store the compressed data.
 * \param id Type of the ID we are looking for (defined in the Id_type enum).
 * \param stopTime unsigned int corresponding to the wanted stoping time of data (Must be higher than the startTime of data and lower than the biggest time value). The stopTime migh be unreached during the storage if a bloc can't be completelly filled.
 *
 * \return pointer to the FftElement in which the data is stored.
 */

FftElement* fftPush(FftStack* myFftStack, IdStack* myIdStack, Id_type id,unsigned int stopTime)
{
	unsigned int nbElement = 0;
	unsigned timeInterval =0;
	if (myFftStack == NULL)
	{
		perror("Error : the FftStack should be initialied before\n");
		return NULL;
	}
	IdElement* idElement;
	idElement = searchIdElement(myIdStack,id);
	if (idElement == NULL)
	{
		perror("Error : The Element corresponding to the ID is inexistant\n");
		return NULL;
	}
	unsigned int startTime = idElement->startTime;
	if (startTime > stopTime)
	{
		perror("Error : stopTime should be higher than startTime\n");
		return NULL;
	}
	
	timeInterval = idElement->timeInterval;
	unsigned int finalTime = (idElement->startTime) + (idElement->dataNumber-1)*(idElement->timeInterval);
	nbElement = stackNumberCount(idElement->dataStack, idElement->timeInterval, finalTime,startTime,stopTime);


	FftElement* myFftElement;
	myFftElement = searchFftElement(myFftStack, id);
	if (myFftElement == NULL)
	{
		perror("Error : The FftElement should be initialized before\n");
		return NULL;
	}
	myFftElement->id = id;
	myFftElement->timeInterval = timeInterval;
	unsigned int blocSize = myFftElement->blocSize;
	if (blocSize > nbElement)
	{
		perror("Error : Unable to store compressed data, the number of elements is lower than the size of blocs\n");
		return NULL;
	}
	else {
		unsigned int nbBlocs = (unsigned int)(nbElement/(myFftElement->blocSize));

		FftDataStack* myFftDataStack = myFftElement -> fftDataStack;
		if ( myFftElement -> fftDataStack != NULL){
			while (myFftDataStack->next != NULL){
				myFftDataStack = myFftDataStack->next;
			}
		}
		float* dataPointer;
		for(unsigned int i = 0 ; i<nbBlocs;i++)
		{
			dataPointer = dataIdStackPop(myIdStack,id,startTime +((i+1)*blocSize-1)*timeInterval);
			if (myFftDataStack == NULL){
				myFftElement -> fftDataStack = (FftDataStack*) malloc(sizeof(*myFftDataStack));
				if (myFftElement -> fftDataStack == NULL)
				{
					perror("Error : Memory allocation for myFftElement -> fftDataStack impossible");
					return NULL;
				}
				myFftElement->startTime = startTime;
				myFftDataStack = myFftElement -> fftDataStack;
			}
			else{
				myFftDataStack -> next = (FftDataStack*) malloc(sizeof(*myFftDataStack));
				if (myFftDataStack -> next == NULL)
					{
						perror("Error : Memory allocation for myFftDataStack -> next impossible");
						return NULL;
					}
				myFftDataStack = myFftDataStack -> next;
			}
			if (dataPointer == NULL){
				perror("Error : Unable to Pop uncompressed Data");
				return NULL;
			}
			myFftDataStack -> pointerHigh = fftHigh(dataPointer,blocSize);
			myFftDataStack -> pointerLow = fftLow(dataPointer,blocSize);
			free(dataPointer);
			myFftElement->dataNumber += 1;
		}
		myFftDataStack -> next = NULL;


	}
	return myFftElement;
}

/**
 * \fn float* fftPop(FftStack* myFftStack, Id_type id,unsigned int startTime, unsigned int stopTime, Erase_mode erase, Fft_type fftType)
 * \brief Transform and transfer a selected array of data into the compressed data architecture.
 *
 * \param myFftStack FftStack instance from which we want to popped the compressed data.
 * \param id Type of the ID we are looking for (defined in the Id_type enum).
 * \param startTime unsigned int corresponding to the wanted starting time of data (Must be higher than the startTime of data and lower than the biggest time value). It's possible that data are taken before startTime in case which it isn't at the beginning of a bloc. During an ERASE, this parameter is useless, the first one will be choosen.
 * \param stopTime unsigned int corresponding to the wanted stoping time of data (Must be higher than the startTime of data and lower than the biggest time value). The stopTime migh be unreached during the storage if a bloc can't be completelly filled.
 * \param erase if we want to erase the popped data (ERASE/KEEP) (defined in the Erase_mode enum).
 * \param fftType Type of fft to perform on the data which will be send (ALL/LOW/HIGH) (defined in the Fft_type enum).
 *
 * \return pointer to the array in which the compress data is stored with first the type of FFT, the number of blocs, the size of blocs and then the startTime.
 */


float* fftPop(FftStack* myFftStack, Id_type id,unsigned int startTime, unsigned int stopTime, Erase_mode erase, Fft_type fftType)
{
	
	int nbParam = 5;  //Number of parameter before the data.
	if (myFftStack == NULL)
	{
		perror("Error : the FftStack should be initialied before");
		return NULL;
	}
	FftElement* fftElement;
	fftElement = searchFftElement(myFftStack, id);
	if (fftElement == NULL)
	{
		perror("Error : The FftElement should be initialized before");
		return NULL;
	}
	if (erase == ERASE)
	{
		startTime = fftElement->startTime;
	}
	int nbBlocs = blocNumberCount(fftElement,startTime,stopTime);
	if (nbBlocs < 1)
	{
		perror("Error : Number of blocs must be higher than 0");
		return NULL;
	}
	//
	unsigned int time = fftElement->startTime;
	FftDataStack *fftDataStack = fftElement->fftDataStack;
	if(fftDataStack == NULL)
	{
		perror("Error : No data in FftDataStack");
		return NULL;
	}
	while(time + fftElement->timeInterval * (fftElement->blocSize-1) < startTime){
		fftDataStack = fftDataStack->next;
		time += fftElement->timeInterval * fftElement->blocSize;
	}
	//
	float* array = NULL;
	unsigned int totalSize;
	if (fftType == ALL)
	{
		totalSize = (fftElement->sizeCompressedL+fftElement->sizeCompressedH)*nbBlocs+nbParam;
		array = (float*) malloc(totalSize*sizeof(float));
		if (array == NULL)
		{
			perror("Error : Allocation array for compressed data impossible");
			return NULL;
		}
		float* array2 = array+nbParam;
		for(int j = 0;j<nbBlocs;j++){
			for(unsigned int i=0;i<fftElement->sizeCompressedL;i++)
			{
				array2[i+j*(fftElement->sizeCompressedL+fftElement->sizeCompressedH)] = fftDataStack->pointerLow[i];
			}
			int g =0;
			for(unsigned int i=fftElement->sizeCompressedL;i<fftElement->sizeCompressedL+fftElement->sizeCompressedH;i++)
			{
				array2[i+j*(fftElement->sizeCompressedL+fftElement->sizeCompressedH)] = fftDataStack->pointerHigh[g];
				g++;
			}
			fftDataStack = fftDataStack->next;
		}
	}
	if (fftType == LOW)
	{
		totalSize = fftElement->sizeCompressedL*nbBlocs+nbParam;
		array = (float*)malloc(totalSize*sizeof(float));
		if (array == NULL)
		{
			perror("Error : Memory allocation impossible for Low frequency array\n");
			return NULL;
		}
		float* array2 = array+nbParam;
		for(int j = 0;j<nbBlocs;j++){
			for(unsigned int i=0;i<fftElement->sizeCompressedL;i++)
			{
				array2[i+j*fftElement->sizeCompressedL] = fftDataStack->pointerLow[i];
			}
			fftDataStack = fftDataStack->next;
		}
	}
	if (fftType == HIGH)
	{
		totalSize = fftElement->sizeCompressedH*nbBlocs+nbParam;
		array = (float*)malloc(totalSize*sizeof(float));
		if (array == NULL)
		{
			perror("Error : Memory allocation impossible for High frequency array\n");
			return NULL;
		}
		float* array2 = array+nbParam;
		for(int j = 0;j<nbBlocs;j++){
			for(unsigned int i=0;i<fftElement->sizeCompressedH;i++)
			{
				array2[i+j*fftElement->sizeCompressedH] = fftDataStack->pointerHigh[i];
			}
			fftDataStack = fftDataStack->next;
		}
	}

	array[0] = (float)fftType;
	array[1] = (float)nbBlocs;
	array[2] = (float)fftElement->blocSize;
	array[3] = (float)time;
	array[4] = (float)(fftElement->timeInterval);
	if (erase == ERASE)
	{
		FftDataStack *fftDataStack = fftElement->fftDataStack;
		FftDataStack *previousFftDataStack = fftElement->fftDataStack;
		for(int i = 0; i < nbBlocs;i++)
		{
			free(fftDataStack->pointerLow);
			free(fftDataStack->pointerHigh);
			fftDataStack = fftDataStack->next;
			free(previousFftDataStack);
			previousFftDataStack = fftDataStack;
		}
		fftElement->dataNumber -= nbBlocs;
		fftElement->fftDataStack = fftDataStack;
		fftElement->startTime += nbBlocs * fftElement->blocSize * fftElement->timeInterval;
	}
	return array;
}

/**
 * \fn float* float* ifft(float* array)
 * \brief Uncompress and print the data sent by fftPop.
 *
 * \param array Array of compressed data compressed sent by fftPop.
 *
 * \return NULL.
 */

float* ifft(float* array){
	//array[0] = Fft_Type
	//array[1] = nbBlocs
	//array[2] = sizeBlocs
	//array[3] = startTime
	//array[4] = timeInterval

	Fft_type fftType = (int)array[0];
	unsigned int nbBlocs = ((unsigned int)array[1]);
	unsigned int sizeBlocs =((unsigned int)array[2]);
	unsigned int startTime = ((unsigned int)array[3]);
	unsigned int timeInterval= ((unsigned int)array[4]);
	unsigned int nbParam = 5;
	unsigned int totalSize = nbBlocs*sizeBlocs;
	float* array2 = array+nbParam;
	float *tabTemp,*tabTemp2;
	float* tabExit = (float*)malloc(totalSize*sizeof(float));
	unsigned int sizeCompressedH, sizeCompressedL;

	if (array == NULL)
	{
		perror("Error : Memory allocation impossible for Exit array\n");
		return NULL;
	}
	if (fftType == LOW){
		sizeCompressedL = ((sizeBlocs+2)/2)+((sizeBlocs+2)/2)%2;
		for(unsigned int i = 0; i<nbBlocs;i++){
			tabTemp = ifftLow(array2 + i*sizeCompressedL,sizeBlocs);
			for(unsigned int j = 0; j<sizeBlocs;j++){
				tabExit[j+i*nbBlocs] = tabTemp[j];
			}
			free(tabTemp);
		}
	}
	if (fftType == HIGH){
		sizeCompressedH = ((sizeBlocs)/2)+((sizeBlocs)/2)%2;
		for(unsigned int i = 0; i<nbBlocs;i++){
			tabTemp = ifftHigh(array2 + i*sizeCompressedH,sizeBlocs);
			for(unsigned int j = 0; j<sizeBlocs;j++){
				tabExit[j+i*nbBlocs] = tabTemp[j];
			}
			free(tabTemp);
		}
	}
	if (fftType == ALL){
		sizeCompressedL = ((sizeBlocs+2)/2)+((sizeBlocs+2)/2)%2;
		sizeCompressedH = ((sizeBlocs)/2)+((sizeBlocs)/2)%2;
		for(unsigned int i = 0; i<nbBlocs;i++){
			tabTemp = ifftLow(array2 + i*(sizeCompressedL+sizeCompressedH),sizeBlocs);
			tabTemp2= ifftHigh(array2 +i*(sizeCompressedL+sizeCompressedH) + sizeCompressedL,sizeBlocs);
			for(unsigned int j = 0; j<sizeBlocs;j++){
				tabExit[j+i*sizeBlocs] = tabTemp[j] + tabTemp2[j];
			}
			free(tabTemp);
			free(tabTemp2);
		}
	}
	unsigned int time = startTime;
	printf("\nTIME\tVALUE\n");
	for(unsigned int i = 0; i<totalSize;i++){
		printf("%d\t%f\n",time,tabExit[i]);
		time += timeInterval;
	}
	free(array);
	free(tabExit);
	//return array2;
	return NULL;
}


/**
 * \fn FftElement* initializeFftElement(FftStack *myFftStack, Id_type id, unsigned int blocSize)
 * \brief Function used to initialize a FftElement instance.
 *
 * This function have to be used after fftInitialize() but before the storage of FftElements
 *
 * \param myFftStack FftStack instance in which we want to initialize a new FftElement.
 * \param id Type of the ID to initialize (defined in the Id_type enum).
 * \param blocSize Size of each compressed bloc. This parameter should be choosen be carrefuly : Indeed, a bloc couldn't be split and have to be completely full before the send. It must be a multiple of 2; But a bigger value implies a bigger compression.
 * \return the initialized fftElement instance.
 */


FftElement* initializeFftElement(FftStack *myFftStack, Id_type id, unsigned int blocSize)
{
	if (blocSize < 1) {
		perror("Error : Size of blocs should be higher than 0\n");
		return NULL;
	}
	FftElement* fftElement = (FftElement*) malloc(sizeof(*fftElement));
	if (fftElement == NULL)
	{
		perror("Error : Memory allocation for fftElement impossible");
		return NULL;
	}
	if (myFftStack == NULL)
    {
		perror("Error : myFftStack uninitialized");
        return NULL;
    }
	fftElement -> id = id;
	fftElement -> blocSize = blocSize; //>=1
	fftElement -> sizeCompressedH = ((blocSize)/2)+((blocSize)/2)%2;
	fftElement -> sizeCompressedL = ((blocSize+2)/2)+((blocSize+2)/2)%2;
	fftElement ->dataNumber = 0;  //4    number of blocs of data in the FftElement.
	fftElement ->fftDataStack = NULL;
	fftElement ->next = NULL;
	fftElement -> next = myFftStack -> first;
	myFftStack->first = fftElement;
	return fftElement;
}

/**
 * \fn int deinitializeFftElement(FftStack *myFftStack, Id_type id)
 * \brief Function used to deinitialize a FftElement corresponding to a specific ID.
 *
 * \param myFftStack FftStack instance we want to deinitialize a specific FftElement.
 * \param id ID of the FftElement we want to deinitialize (defined in the enum Id_type)
 * \return id if it SUCCESSED, -1 if it FAILED.
 */

int deinitializeFftElement(FftStack *myFftStack, Id_type id)
{
	FftElement* fftElement = searchFftElement(myFftStack,id);
	if (myFftStack == NULL)
	{
		perror("Error : the FftStack should be initialied before\n");
		return -1;
	}
	if (fftElement == NULL) {
		perror("Error : Deinitialization impossible : fftElement inexistant\n");
		return -1;
	}
	FftElement* previousFftElement = myFftStack -> first;
	FftElement* currentFftElement = myFftStack -> first;
	while(currentFftElement != fftElement) {
		previousFftElement = currentFftElement;
		currentFftElement = currentFftElement -> next;
	}
	previousFftElement -> next = currentFftElement -> next;
	//deinitialize
	FftDataStack* myFftDataStack = currentFftElement -> fftDataStack;
	FftDataStack* oldFftDataStack = myFftDataStack;
	if(myFftDataStack != NULL)
	{
		while (myFftDataStack->next != NULL) {
			free(myFftDataStack -> pointerHigh);
			free(myFftDataStack -> pointerLow);
			oldFftDataStack = myFftDataStack;
			myFftDataStack = myFftDataStack -> next;
			free(oldFftDataStack);
		}
		free(myFftDataStack -> pointerHigh);
		free(myFftDataStack -> pointerLow);
		free(myFftDataStack);
	}
	if (myFftStack->first == currentFftElement)
		myFftStack->first = currentFftElement->next;
	free(currentFftElement);
	return id;
}

/**
 * \fn int deinitializeFirstFftElement(FftStack *myFftStack)
 * \brief Function used to deinitialize the first FftElement.
 *
 * \param myFftStack FftStack instance we want to Pop the first FftElement.
 * \return Int value of the Popped ID.
 */

int deinitializeFirstFftElement(FftStack *myFftStack)
{
	FftElement* fftElement;
	int id = -1;
	if (myFftStack == NULL)
	{
		perror("Error : the FftStack should be initialied before\n");
		return -1;
	}
	fftElement = myFftStack->first;
	if (myFftStack != NULL && myFftStack->first != NULL)
    {
        id = fftElement->id;
        myFftStack->first = fftElement->next;


	FftDataStack* myFftDataStack = fftElement -> fftDataStack;
	FftDataStack* oldFftDataStack = myFftDataStack;
	if(myFftDataStack != NULL)
	{
		while (myFftDataStack->next != NULL) {
			free(myFftDataStack -> pointerHigh);
			free(myFftDataStack -> pointerLow);
			oldFftDataStack = myFftDataStack;
			myFftDataStack = myFftDataStack -> next;
			free(oldFftDataStack);
		}
		free(myFftDataStack -> pointerHigh);
		free(myFftDataStack -> pointerLow);;
		free(myFftDataStack);
	}
        free(fftElement);
    }
	return id;
}

/**
 * \fn FftElement* searchFftElement(FftStack *myFftStack, Id_type id)
 * \brief Search the pointer to the FftElement corresponding to the ID.
 *
 * \param myFftStack FftStack instance in which we want to search the FftElement.
 * \param id Type of the ID we are looking for (defined in the Id_type enum).
 * \return pointer to the fftElement corresponding to the id. NULL if it doesn't exist.
 */

FftElement* searchFftElement(FftStack *myFftStack, Id_type id)
{
	FftElement *fftElement = NULL;
	if (myFftStack == NULL)
	{
		perror("Error : the FftStack should be initialied before\n");
		return NULL;
	}
	fftElement = myFftStack->first;
	if (myFftStack != NULL && myFftStack->first != NULL)
    {
		while(fftElement != NULL && fftElement->id != id)
		{
			fftElement = fftElement ->next;
		}
		return fftElement;
    }
	return NULL;
}


/**
 * \fn int blocNumberCount(FftElement *fftElement, unsigned int startTime,unsigned int stopTime)
 * \brief Function used to count the number of blocs satisfying the time condition.
 *
 * \param fftElement FftElement instance in which bloc(s) will be analysed.
 * \param startTime unsigned int corresponding to the wanted starting time of data.
 * \param stopTime unsigned int corresponding to the wanted stoping time of data.
 * \return int value corresponding to the number of values satisfying the time conditions (include the whole bloc if one of the values correspond to the time condition). -1 if none value corresponding or if the startTime is too low or the stopTime is too high.
 */

int blocNumberCount(FftElement *fftElement, unsigned int startTime,unsigned int stopTime)
{
	int i =0;
	if (fftElement == NULL)
    {
        perror("Error : fftElement uninitialized");
		return -1;
    }
	unsigned int finalTime = fftElement->startTime + (fftElement->dataNumber * fftElement->blocSize -1)* fftElement->timeInterval ;
	if(stopTime>finalTime)
	{
		perror("Error : stopTime higher than the last time of data");
		return -1;
    }
	if(startTime<fftElement->startTime)
	{
		perror("Error : startTime lower than the first time of data");
		return -1;
	}
	unsigned int time = fftElement->startTime;
	FftDataStack *fftDataStack = fftElement->fftDataStack;
	if(fftDataStack == NULL)
	{
		perror("Error : No data in FftDataStack");
		return -1;
	}
	while(time + fftElement->timeInterval * (fftElement->blocSize-1) < startTime){
		fftDataStack = fftDataStack->next;
		time += fftElement->timeInterval * fftElement->blocSize;
	}
	while(time<=stopTime){
		fftDataStack = fftDataStack->next;
		time += fftElement->timeInterval * fftElement->blocSize;
		i++;
	}
	return i;
}

/**
 * \fn void printFftStack(FftStack *fftStack)
 * \brief Function used to print the state of the FftStack.
 *
 * \param fftStack FftStack instance we want to print the state.
 */

void printFftStack(FftStack *fftStack)
{
	FftElement* current;
    if (fftStack == NULL)
    {
        perror("Error : FftStack must be initialized\n");
		return;
    }
    
	current = fftStack->first;
	if (fftStack->first == NULL)
    {
		perror("Error : Nothing to print\n");
        return;
    }
	printf("\nCOMPRESSED ARCHITECTURE\n");

    while (current != NULL)
    {
        printf("ID:%d\tStartTime:%d\tBlocSize:%d\tsizeCompressedLow:%d\tsizeCompressedHigh:%d\tTimeInterval:%d\tNumber of Element : %d\n", current->id,current->startTime,current->blocSize,current->sizeCompressedL,current->sizeCompressedH,current->timeInterval,current->dataNumber);
		//printFftDataStack(current);
        current = current->next;
    }

    printf("\n");
}

/**
 * \fn void printFftDataStack(FftElement *fftElement)k)
 * \brief Function used by 'printFftStack' to print the compressed data into a FftStack
 *
 * \param fftElement FftElement instance we want to print the state.
 */

void printFftDataStack(FftElement *fftElement)
{
	FftDataStack* current;
    if (fftElement == NULL)
    {
        perror("Error : FftStack must be initialized\n");
		return;
    }
	
    
	current = fftElement->fftDataStack;
	if (current == NULL)
    {
        return;
    }

    while (current != NULL)
    {
		for(unsigned int i = 0; i<(fftElement ->sizeCompressedL);i++)
		{
        	printf("	%f\tLow\n", current->pointerLow[i]);
		}
		for(unsigned int i = 0; i<(fftElement ->sizeCompressedH);i++)
		{
        	printf("	%f\tHigh\n", current->pointerHigh[i]);
		}
		
        current = current->next;
    }

    printf("\n");
}
