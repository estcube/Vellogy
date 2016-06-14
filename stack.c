#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stack.h"

struct Element
{
	void *number;
	Element *next;
};

struct Stack
{
	int numberSize;
	Element *first;
};

Stack* initialize(int numberSize)
{
    Stack *stack = malloc(sizeof(*stack));
    stack->first = NULL;
	stack->numberSize = numberSize;
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

void stackPush(Stack *stack, void *newAdress)
{
    Element *newNb = malloc(sizeof(*newNb));
    if (stack == NULL || newNb == NULL)
    {
        exit(EXIT_FAILURE);
    }
	newNb->number = malloc(sizeof(stack->numberSize));
	if (newNb->number == NULL)
    {
        exit(EXIT_FAILURE);
    }
	memcpy(newNb->number,newAdress, stack->numberSize);
    newNb->next = stack->first;
    stack->first = newNb;
}

void* stackPop(Stack *stack, void* numberAdress)
{
	Element *stackElement;
    if (stack == NULL)
    {
        exit(EXIT_FAILURE);
    }

    stackElement = stack->first;

    if (stack != NULL && stack->first != NULL)
    {
        numberAdress = stackElement->number;
        stack->first = stackElement->next;
		/*free(stackElement->number);*/
        free(stackElement);
    }
	printf(" AdresseNB : %d\n", *(int*)numberAdress);
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
        printf("%d\n", *(int*)current->number);
        current = current->next;
    }

    printf("\n");
}
