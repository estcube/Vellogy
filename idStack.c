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

IdElement* dataIdStackPush(IdStack* myIdStack, int id, void *newAdress)
{
	IdElement *idElement;
	idElement = searchIdElement(myIdStack,id);
	if (idElement != NULL)
	{
		stackPush(idElement->dataStack, newAdress, idElement->type);
	}
	(idElement->dataNumber)++;
	return idElement;
}

void* dataIdStackPop(IdStack* myIdStack, int id, int startTime, int stopTime)
{
	if (myIdStack == NULL)
    {
        exit(EXIT_FAILURE);
    }
	IdElement *idElement;
	int finalTime = 0;
	int i=0;
	idElement = searchIdElement(myIdStack,id);
	if (idElement != NULL)
	{
		finalTime = (idElement->startTime) + (idElement->dataNumber-1)*(idElement->timeInterval);
		i = stackNumberCount(idElement->dataStack, idElement->timeInterval, finalTime,startTime,stopTime);
		if (i > 0)
		{
			idElement->dataNumber -= i;
			return (stackPop(idElement->dataStack, idElement->type,idElement->timeInterval, finalTime, stopTime,i));
		}
	}
	return NULL;
}

IdElement* searchIdElement(IdStack *myIdStack, int id)
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

IdElement* idStackPush(IdStack *myIdStack, int newId, int newType,int newStartTime, int newTimeInterval)
{
    IdElement *idElement = (IdElement*) malloc(sizeof(*idElement));
    if (myIdStack == NULL || idElement == NULL)
    {
        exit(EXIT_FAILURE);
    }
	idElement->dataNumber=0;
    idElement->id = newId;
    idElement->type = newType;
    idElement->startTime = newStartTime;
    idElement->timeInterval = newTimeInterval;
    idElement->next = myIdStack->first;
    myIdStack->first = idElement;
	idElement->dataStack = initialize();
	return idElement;
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

int idStackPop(IdStack *myIdStack, int id)
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
        printf("ID:%d Type:%d StartTime:%d TimeInterval:%d Number of Element : %d\n", current->id,current->type,current->startTime,current->timeInterval,current->dataNumber);
		/*printStack(current->dataStack);*/
        current = current->next;
    }

    printf("\n");
}

