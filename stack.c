/**
 * \file stack.c
 * \brief stack Functions
 * \author Quentin.C
 * \version 0.5
 * \date August 9th 2016
 *
 * Functions only used by idStack.c to manipulate Stacks
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"

/**
 * \fn Stack* initialize()
 * \brief Function used to initialize a Stack instance.
 *
 * This function is used to initialise a Stack instance where will be stored Elements containning data.
 *
 * \return the initialized Stack instance.
 */

Stack* initialize()
{
    Stack *stack = (Stack*) malloc(sizeof(*stack));
	if (stack == NULL)
    {
        perror("Error : Memory allocation for stack impossible");
		return NULL;
    }
    stack->first = NULL;
	return stack;
}

/**
 * \fn void deinitialize(Stack* stack)
 * \brief Function used to deinitialize a Stack instance.
 *
 * This function is used to deinitialise a Stack instance.
 *
 * \param stack Stack instance which have to be deinitialized.
 */

void deinitialize(Stack* stack)
{
	if (stack == NULL)
    {
        perror("Error : Stack uninitialized");
		return;
    }
	else
    {
		while(stack->first != NULL)
		{
			free(firstStackPop(stack));
		}
	}
	free(stack);
}

/**
 * \fn void stackPush(Stack *stack, void *newAdress, int numberSize)
 * \brief Function used to add and configure a new Element with it data into a Stack.
 *
 * This function have to be used after initialize().
 *
 * \param stack Stack instance in which an Element will be added.
 * \param newAdress Pointer on the data we want to push into the Stack.
 * \param numberSize int value of the size of data.
 */

void stackPush(Stack *stack, void *newAdress, int numberSize)
{
    Element *newNb = (Element*) malloc(sizeof(*newNb));
	if (newNb == NULL)
	{
		perror("Error : Allocation of newNb Impossible");	
		return;
	}
    if (stack == NULL)
    {
        perror("Error : Stack uninitialized");
		return;
    }
	newNb->number = malloc(numberSize);
	if (newNb->number == NULL)
    {
        perror("Error : Number pointer invalid");
		return;
    }
	memcpy(newNb->number,newAdress, numberSize);
    newNb->next = stack->first;
    stack->first = newNb;
}

/**
 * \fn void* firstStackPop(Stack *stack)
 * \brief Function used to Pop the first Element.
 *
 * \param stack Stack instance we want to Pop the first Element.
 * \return pointer to the data of the first Element.
 */

void* firstStackPop(Stack *stack)
{
	Element *stackElement;
	void* numberAdress = NULL;
    if (stack == NULL)
    {
        perror("Error : Stack uninitialized");
		return NULL;
    }

    stackElement = stack->first;
    if (stack != NULL && stack->first != NULL)
    {
        numberAdress = stackElement->number;
        stack->first = stackElement->next;
        free(stackElement);
    }
	/*Don't forget to free numberAdress After*/
    return numberAdress;
}

/**
 * \fn void* stackPop(Stack* myStack, int type, unsigned int timeInterval,unsigned int finalTime,unsigned int stopTime, int i)
 * \brief Function used to pop Elements between two time values.
 *
 * \param myStack Stack instance in which Element(s) will be popped.
 * \param type Size of a data value.
 * \param timeInterval Time interval between each data value.
 * \param finalTime time of the last data value.
 * \param stopTime Last time of the wanted data value.
 * \param dataNumber number of values corresponding to the wanted time.
 * \return pointer to the first element of the data array.
 */

void* stackPop(Stack* myStack, int type, unsigned int timeInterval,unsigned int finalTime,unsigned int stopTime, int dataNumber)
{
	unsigned int time = finalTime;
	char *array, *array2;
	int j =0;
	array = malloc(dataNumber*type);
	if (array == NULL)
    {
        perror("Error : Memory allocation for array impossible");
		return NULL;
    }
	array2 = array+(dataNumber-1)*type;
	if (myStack == NULL)
    {
        perror("Error : Stack uninitialized");
		return NULL;
    }
	Element *element = NULL;
	Element *oldElement = NULL;
	Element *firstElement = NULL;
	if (myStack != NULL)
    {
		
		element = myStack->first;
		while (stopTime < time)
		{
			oldElement = element;
			element = element->next;
			time -= timeInterval;
		}
		firstElement = oldElement;
		while (j<dataNumber)
		{
			memcpy(array2,element->number,type);
			oldElement = element;
			element = element->next;
			free(oldElement->number);
			free(oldElement);
			j++;
			array2 -= type;
		}
			if (firstElement==NULL)
				myStack->first = element;
			else
				firstElement->next=element;
	}
	/*Don't forget to free array Then*/
	return array;
}

/**
 * \fn int stackNumberCount(Stack *myStack, unsigned int timeInterval, unsigned int finalTime,unsigned int startTime,unsigned int stopTime)
 * \brief Function used to count the number of data satisfying the time condition.
 *
 * \param myStack Stack instance in which Element(s) will be analysed.
 * \param timeInterval Time interval between each data value.
 * \param finalTime time of the last data value.
 * \param startTime unsigned int corresponding to the wanted starting time of data.
 * \param stopTime unsigned int corresponding to the wanted stoping time of data.
 * \return int value corresponding to the number of values satisfying the time conditions. -1 if none value corresponding or if the startTime is too low or the stopTime is too high.
 */

int stackNumberCount(Stack *myStack, unsigned int timeInterval, unsigned int finalTime,unsigned int startTime,unsigned int stopTime)
{
	int i =0;
	unsigned int time = finalTime;
	if (myStack == NULL)
    {
        perror("Error : Stack uninitialized");
		return -1;
    }
	if(stopTime > finalTime || stopTime<startTime)
	{
		return -1;
	}
	Element *element;
	if (myStack != NULL)
    {
		element = myStack->first;
		while (stopTime < time)
		{
			if (element->next == NULL)
			{
				return -1;
			}
			element = element->next;
			time -= timeInterval;
		}
		while (startTime <= time)
		{
			i++;
			if (element->next == NULL)
			{
				if (startTime >= time)
				{
					return i;
				}
				return -1;
			}
			time -= timeInterval;
			element = element->next;
		}
	}
	return i;
}

/**
 * \fn void printStack(Stack *stack)
 * \brief Function used to print data in a Stack belonging to a idStack of specific datatype. (Here Float)
 *
 * Can be used in printIdStack(IdStack *idStack) but it's not working if there is not float data.
 *
 * \param stack Stack instance we want to print the float data.
 */

void printStack(Stack *stack) /*FLOAT PRINT FUNCTION*/
{
	Element* current;
    if (stack == NULL)
    {
        perror("Error : Stack uninitialized");
		return;
    }
    
	current = stack->first;

    while (current != NULL)
    {
        printf("	%f\n", *(float*)current->number);
        current = current->next;
    }

    /*printf("\n");*/
}
