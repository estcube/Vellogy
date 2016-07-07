#include <stdio.h>
#include <stdlib.h>
#include "idStack.h"
#include "stack.h"

IdStack* idInitialize()
{
    IdStack *idStack = (IdStack*) malloc(sizeof(*idStack));
    idStack->first = NULL;
	return idStack;
}

void idDeinitialize(IdStack* myIdStack)
{
	if (myIdStack == NULL)
    {
        exit(EXIT_FAILURE);
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

void* dataIdStackPop(IdStack* myIdStack, Id_type id,unsigned int startTime,unsigned int stopTime)
{
	IdElement *idElement;
	unsigned int finalTime = 0;
	int i=0;
	if (myIdStack == NULL)
    {
        exit(EXIT_FAILURE);
    }

	idElement = searchIdElement(myIdStack,id);
	if (idElement != NULL)
	{
		finalTime = (idElement->startTime) + (idElement->dataNumber-1)*(idElement->timeInterval);
		i = stackNumberCount(idElement->dataStack, idElement->timeInterval, finalTime,startTime,stopTime);
		if (i > 0)
		{
			idElement->dataNumber -= i;
			return (stackPop(idElement->dataStack, sizeDataType(idElement->dataType),idElement->timeInterval, finalTime, stopTime,i));
		}
	}
	return NULL;
}

IdElement* searchIdElement(IdStack *myIdStack, Id_type id)
{
	IdElement *stackElement = NULL;
	if (myIdStack == NULL)
    {
        exit(EXIT_FAILURE);
    }
	stackElement = myIdStack->first;
	if (myIdStack != NULL && myIdStack->first != NULL)
    {
		while(stackElement != NULL && stackElement->id != id)
		{
			stackElement = stackElement ->next;
		}
    }
	return stackElement;
}

int getStartTime(IdStack *myIdStack)
{
	if (myIdStack != NULL)
		if (myIdStack != NULL)
			return (myIdStack->first->startTime);
	return -1;
}

int getTimeInterval(IdStack *myIdStack)
{
	if (myIdStack != NULL)
		if (myIdStack != NULL)
			return (myIdStack->first->timeInterval);
	return -1;
}

IdElement* idStackPush(IdStack *myIdStack, Id_type newId,Signal_type newSignalType, Data_type newDataType,unsigned int newStartTime, unsigned int newTimeInterval)
{
    IdElement *idElement = (IdElement*) malloc(sizeof(*idElement));
    if (myIdStack == NULL || idElement == NULL)
    {
        exit(EXIT_FAILURE);
    }
	if (sizeDataType(newDataType) < 0)
	{
        exit(EXIT_FAILURE);
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

int firstIdStackPop(IdStack *myIdStack)
{
	IdElement *stackElement;
	int id = -1;
    if (myIdStack == NULL)
    {
        exit(EXIT_FAILURE);
    }

    stackElement = myIdStack->first;

    if (myIdStack != NULL && myIdStack->first != NULL)
    {
        id = stackElement->id;
        myIdStack->first = stackElement->next;
		deinitialize(stackElement->dataStack);
        free(stackElement);
    }
    return id;
}

int idStackPop(IdStack *myIdStack, Id_type id)
{
	IdElement *stackElement = NULL;
	IdElement *oldStackElement = NULL;
    if (myIdStack == NULL)
    {
        exit(EXIT_FAILURE);
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

void printIdStack(IdStack *stack)
{
	IdElement* current;
    if (stack == NULL)
    {
        exit(EXIT_FAILURE);
    }
    
	current = stack->first;

	printf("\n");

    while (current != NULL)
    {
        printf("ID:%d	SignalType:%d	TypeSize:%d	StartTime:%d	TimeInterval:%d		Number of Element : %d\n", current->id,current->signalType,sizeDataType(current->dataType),current->startTime,current->timeInterval,current->dataNumber);
		/*printStack(current->dataStack);*/
        current = current->next;
    }

    printf("\n");
}

