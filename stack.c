#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"

Stack* initialize()
{
    Stack *stack = (Stack*) malloc(sizeof(*stack));
    stack->first = NULL;
	return stack;
}

void deinitialize(Stack* stack)
{
	if (stack == NULL)
    {
        exit(EXIT_FAILURE);
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

void stackPush(Stack *stack, void *newAdress, int numberSize)
{
    Element *newNb = (Element*) malloc(sizeof(*newNb));
    if (stack == NULL || newNb == NULL)
    {
        exit(EXIT_FAILURE);
    }
	newNb->number = malloc(numberSize);
	if (newNb->number == NULL)
    {
        exit(EXIT_FAILURE);
    }
	memcpy(newNb->number,newAdress, numberSize);
    newNb->next = stack->first;
    stack->first = newNb;
}

void* firstStackPop(Stack *stack)
{
	Element *stackElement;
	void* numberAdress = NULL;
    if (stack == NULL)
    {
        exit(EXIT_FAILURE);
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



void* stackPop(Stack* myStack, int type, int timeInterval, int finalTime, int stopTime, int i)
{
	int time = finalTime;
	char *array, *array2;
	array = malloc(i*type);
	array2 = array+(i-1)*type;
	if (myStack == NULL)
    {
        exit(EXIT_FAILURE);
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
		int j =0;
		while (j<i)
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


int stackNumberCount(Stack *myStack, int timeInterval, int finalTime,int startTime,int stopTime)
{
	int i =0;
	int time = finalTime;
	/*printf("FinalTime = %d\n",finalTime);*/
	if (myStack == NULL)
    {
        exit(EXIT_FAILURE);
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
			/*printf("Time = %d   I = %d\n",time,i);*/
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

void printStack(Stack *stack) /*INT PRINT FUNCTION*/
{
	Element* current;
    if (stack == NULL)
    {
        exit(EXIT_FAILURE);
    }
    
	current = stack->first;

    while (current != NULL)
    {
        printf("	%d\n", *(int*)current->number);
        current = current->next;
    }

    /*printf("\n");*/
}
