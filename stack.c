#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"

Stack* initialize()
{
    Stack *stack = malloc(sizeof(*stack));
    stack->first = NULL;
	return stack;
}

void deinitialize(Stack* stack)
{
	void *adress = NULL;
	if (stack == NULL)
    {
        exit(EXIT_FAILURE);
    }
	else
    {
		while(stack->first != NULL)
		{
			free(stackPop(stack,adress));
		}
	}
	free(stack);
}

void stackPush(Stack *stack, void *newAdress, int numberSize)
{
    Element *newNb = malloc(sizeof(*newNb));
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

void* stackPop(Stack *stack, void* numberAdress)
{
	Element *stackElement;
	numberAdress = NULL;
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
