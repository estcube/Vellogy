/**
 * \file idStack.c
 * \brief IdStack Functions
 * \author Quentin.C
 * \version 0.5
 * \date August 9th 2016
 *
 * Functions used to initialize, deinitialize, Push and Pop IdStacks and Stacks
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "idStack.h"

/**
 * \fn IdStack* idInitialize()
 * \brief Function used to initialize a idStack instance.
 *
 * This function have to be used before everything else function to initialise an instance where will be stored IdElements
 *
 * \return the initialized idStack instance.
 */

IdStack* idInitialize()
{
    IdStack *idStack = (IdStack*) malloc(sizeof(*idStack));
  if (idStack == NULL)
    {
        perror("Error : Memory allocation for idStack impossible");
    return NULL;
    }
    idStack->first = NULL;
  return idStack;
}

/**
 * \fn void idDeinitialize(IdStack* myIdStack)
 * \brief Function used to deinitialize a idStack instance.
 *
 * This function have to be used after everything else function to deinitialise an IdStack instance.
 *
 * \param myIdStack IdStack instance which have to be deinitialized.
 */
void idDeinitialize(IdStack* myIdStack)
{
  if (myIdStack == NULL)
    {
    perror("Error : myIdStack uninitialized");
        return;
    }
  else
    {
    while(myIdStack->first != NULL)
    {
      firstIdStackPop(myIdStack);
    }
  }
  free(myIdStack);
}

/**
 * \fn IdElement* dataIdStackPush(IdStack* myIdStack, Id_type id, void *newAdress)
 * \brief Push a new data into the idElement corresponding to the id.
 *
 * \param myIdStack IdStack instance in which we want to search the IdElement where to put the new data.
 * \param id Type of the ID we are looking for (defined in the Id_type enum).
 * \param newAdress Pointer on the data we want to push into the IdElement.
 * \return pointer to the idElement in which was pushed the new data.
 */

IdElement* dataIdStackPush(IdStack* myIdStack, Id_type id, void *newAdress)
{
  IdElement *idElement;
  idElement = searchIdElement(myIdStack,id);
  if (idElement != NULL)
  {
    stackPush(idElement->dataStack, newAdress, sizeDataType(idElement->dataType));
  }
  (idElement->dataNumber)++;
  return idElement;
}

/**
 * \fn void* dataIdStackPop(IdStack* myIdStack, Id_type id,unsigned int startTime,unsigned int stopTime)
 * \brief Push an array of data between the startTime of data and StopTime (included) corresponding to the id.
 *
 * \param myIdStack IdStack instance in which we want to search the IdElement to pop data from.
 * \param id Type of the ID we are looking for (defined in the Id_type enum).

 * \param stopTime unsigned int corresponding to the wanted stoping time of data (Must be higher than the startTime of data and lower than the biggest time value)
 * \return pointer to the first element of the data array. NULL if the id doesn't exist or the stopTime is lower than startTime or higher than the highest time value.
 */

void* dataIdStackPop(IdStack* myIdStack, Id_type id,unsigned int stopTime)
{
  IdElement *idElement;
  unsigned int finalTime = 0;
  int dataNumber=0;
  if (myIdStack == NULL)
    {
        perror("Error : myIdStack uninitialized");
    return NULL;
    }

  idElement = searchIdElement(myIdStack,id);
  if (idElement != NULL)
  {
    unsigned int startTime = idElement->startTime;
    if (startTime != idElement->startTime && stopTime != idElement->startTime+idElement->timeInterval*idElement->dataNumber){
      perror("startTime and stopTime can't be different from the extremals values at the same time");
      return NULL;
    }
    finalTime = (idElement->startTime) + (idElement->dataNumber-1)*(idElement->timeInterval);
    dataNumber = stackNumberCount(idElement->dataStack, idElement->timeInterval, finalTime,startTime,stopTime);
    if (dataNumber > 0)
    {
      idElement->dataNumber -= dataNumber;
      if(startTime == idElement->startTime)
        idElement->startTime = stopTime + idElement->timeInterval;
      return (stackPop(idElement->dataStack, sizeDataType(idElement->dataType),idElement->timeInterval, finalTime, stopTime,dataNumber));
    }
  }
  return NULL;
}

/**
 * \fn IdElement* searchIdElement(IdStack *myIdStack, Id_type id)
 * \brief Search the pointer to the IdElement corresponding to the ID.
 *
 * \param myIdStack IdStack instance in which we want to search the IdElement.
 * \param id Type of the ID we are looking for (defined in the Id_type enum).
 * \return pointer to the idElement corresponding to the id. NULL if it doesn't exist.
 */


IdElement* searchIdElement(IdStack *myIdStack, Id_type id)
{
  IdElement *idElement = NULL;
  if (myIdStack == NULL)
    {
        perror("Error : myIdStack uninitialized");
    return NULL;
    }
  idElement = myIdStack->first;
  if (myIdStack != NULL && myIdStack->first != NULL)
    {
    while(idElement != NULL && idElement->id != id)
    {
      idElement = idElement ->next;
    }
    return idElement;
    }
  return NULL;
}

/**
 * \fn int getStartTime(IdStack *myIdStack)
 * \brief Return the start time of the first IdElement of the IdStack.
 *
 * \param myIdStack IdStack instance which we want the first IdElement start time.
 * \return int value of the first IdElement start time. -1 if the instance doesn't exist.
 */

int getStartTime(IdStack *myIdStack)
{
  if (myIdStack != NULL)
    if (myIdStack != NULL)
      return (myIdStack->first->startTime);
  return -1;
}

/**
 * \fn int getTimeInterval(IdStack *myIdStack)
 * \brief Return the time of the first IdElement of the IdStack.
 *
 * \param myIdStack IdStack instance which we want the first IdElement time interval.
 * \return int value of the first IdElement time interval. -1 if the instance doesn't exist.
 */

int getTimeInterval(IdStack *myIdStack)
{
  if (myIdStack != NULL)
    if (myIdStack->first != NULL)
      return (myIdStack->first->timeInterval);
  return -1;
}


/**
 * \fn IdElement* idStackPush(IdStack *myIdStack, Id_type newId,Signal_type newSignalType, Data_type newDataType,unsigned int newStartTime, unsigned int newTimeInterval)
 * \brief Function used to add and configure a new IdElement
 *
 * This function have to be used after idInitialize().
 *
 * \param myIdStack IdStack instance in which an IdElement will be added.
 * \param newId Type of the new ID (defined in the enum Id_type).
 * \param newSignalType Type of the new ID Signal (defined in the enum Signal_type).
 * \param newDataType Type of the data (defined in the enum Data_type).
 * \param newStartTime Start time of the data.
 * \param newTimeInterval Time interval between each data value.
 * \return pointer on the new IdElement.
 */

IdElement* idStackPush(IdStack *myIdStack, Id_type newId,Signal_type newSignalType, Data_type newDataType,unsigned int newStartTime, unsigned int newTimeInterval)
{
    IdElement *idElement = (IdElement*) malloc(sizeof(*idElement));
  if (idElement == NULL)
    {
        perror("Error : Memory allocation for idElement impossible");
    return NULL;
    }
    if (myIdStack == NULL)
    {
        perror("Error : myIdStack uninitialized");
    return NULL;
    }
  if (idElement == NULL)
    {
        perror("Error : idElement uninitialized");
    return NULL;
    }
  if (sizeDataType(newDataType) <= 0)
  {
        perror("Error : SizeDataType should be high than 0");
    return NULL;
    }
  idElement->dataNumber=0;
    idElement->id = newId;
  idElement->signalType = newSignalType;
    idElement->dataType = newDataType;
    idElement->startTime = newStartTime;
    idElement->timeInterval = newTimeInterval;
    idElement->next = myIdStack->first;
    myIdStack->first = idElement;
  idElement->dataStack = initialize();
  return idElement;
}

/**
 * \fn int sizeDataType(Data_type dataType)
 * \brief Return the size of the parameter dataType
 *
 * \param dataType Type of the data (defined in the enum Data_type).
 * \return Size of the dataType. -1 if the dataType doesn't exist.
 */

int sizeDataType(Data_type dataType){
  switch (dataType) {
    case UINT8_T :
    case INT8_T :
    case CHAR :
      return 1;
     break;
    case UINT16_T :
    case INT16_T :
      return 2;
     break;
    case UINT32_T :
    case INT32_T :
    case FLOAT :
      return 4;
     break;
    case UINT64_T :
    case INT64_T :
    case DOUBLE :
      return 8;
     break;
    case LDOUBLE :
      return 10;
     break;
    default:
      return -1;
  }
}

/**
 * \fn int firstIdStackPop(IdStack *myIdStack)
 * \brief Function used to Pop the first IdElement.
 *
 * \param myIdStack IdStack instance we want to Pop the first IdElement.
 * \return Int value of the Popped ID.
 */

int firstIdStackPop(IdStack *myIdStack)
{
  IdElement *idElement;
  int id = -1;
    if (myIdStack == NULL)
    {
        perror("Error : myIdStack uninitialized");
    return -1;
    }

    idElement = myIdStack->first;

    if (myIdStack != NULL && myIdStack->first != NULL)
    {
        id = idElement->id;
        myIdStack->first = idElement->next;
    deinitialize(idElement->dataStack);
        free(idElement);
    }
    return id;
}

/**
 * \fn int idStackPop(IdStack *myIdStack, Id_type id)
 * \brief Function used to Pop a IdElement corresponding to a specific ID.
 *
 * \param myIdStack IdStack instance we want to Pop a specific IdElement.
 * \param id ID of the IdElement we want to Pop (defined in the enum Id_type)
 * \return 0 if it SUCCESSED, -1 if it FAILED.
 */

int idStackPop(IdStack *myIdStack, Id_type id)
{
  IdElement *stackElement = NULL;
  IdElement *oldStackElement = NULL;
    if (myIdStack == NULL)
    {
        perror("Error : myIdStack uninitialized");
    return -1;
    }

    stackElement = myIdStack->first;

    if (myIdStack != NULL && myIdStack->first != NULL)
    {
    while(stackElement != NULL && stackElement->id != id)
    {
      oldStackElement = stackElement;
      stackElement = stackElement ->next;
    }
    if(stackElement != NULL)
    {
      if (oldStackElement == NULL)
      {

        myIdStack->first = stackElement->next;
      }
      else
      {
        oldStackElement->next = stackElement->next;
      }
      deinitialize(stackElement->dataStack);
      free(stackElement);
      return 0;
    }

    }
    return -1;
}

/**
 * \fn void printIdStack(IdStack *idStack)
 * \brief Function used to print the state of the idStack.
 *
 * \param myIdStack IdStack instance we want to print the state.
 */

void printIdStack(IdStack *idStack)
{
  IdElement* current;
    if (idStack == NULL)
    {
        perror("Error : myIdStack uninitialized");
    return;
    }

  current = idStack->first;

  printf("\nUNCOMPRESSED ARCHITECTURE\n");

    while (current != NULL)
    {
        printf("ID:%d  SignalType:%d  TypeSize:%d  StartTime:%d  TimeInterval:%d    Number of Element : %d\n", current->id,current->signalType,sizeDataType(current->dataType),current->startTime,current->timeInterval,current->dataNumber);
    //printStack(current->dataStack);
        current = current->next;
    }

    printf("\n");
}
