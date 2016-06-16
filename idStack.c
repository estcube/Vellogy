#include <stdio.h>
#include <stdlib.h>
#include "idStack.h"
#include "stack.h"

IdStack* idInitialize()
{
    IdStack *stack = malloc(sizeof(*stack));
    stack->first = NULL;
	return stack;
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
	return idElement;
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

void idStackPush(IdStack *myIdStack, int newId, int newType,int newStartTime, int newTimeInterval)
{
    IdElement *idElement = malloc(sizeof(*idElement));
    if (myIdStack == NULL || idElement == NULL)
    {
        exit(EXIT_FAILURE);
    }
    idElement->id = newId;
    idElement->type = newType;
    idElement->startTime = newStartTime;
    idElement->timeInterval = newTimeInterval;
    idElement->next = myIdStack->first;
    myIdStack->first = idElement;
	idElement->dataStack = initialize();
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

    while (current != NULL)
    {
        printf("ID:%d Type:%d StartTime:%d TimeInterval:%d\n", current->id,current->type,current->startTime,current->timeInterval);
		printStack(current->dataStack);
        current = current->next;
    }

    printf("\n");
}

