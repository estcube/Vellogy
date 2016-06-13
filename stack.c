#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

struct Element
{
	int number;
	Element *next;
};

struct Stack
{
	Element *first;
};

Stack* initialize()
{
    Stack *stack = malloc(sizeof(*stack));
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
			unstack(stack);
		}
	}
	free(stack);
}

void toStack(Stack *stack, int newNumber)
{
    Element *newNb = malloc(sizeof(*newNb));
    if (stack == NULL || newNb == NULL)
    {
        exit(EXIT_FAILURE);
    }
    newNb->number = newNumber;
    newNb->next = stack->first;
    stack->first = newNb;
}

int unstack(Stack *stack)
{
	Element *stackElement;
	int stackNumber = 0;
    if (stack == NULL)
    {
        exit(EXIT_FAILURE);
    }

    stackElement = stack->first;

    if (stack != NULL && stack->first != NULL)
    {
        stackNumber = stackElement->number;
        stack->first = stackElement->next;
        free(stackElement);
    }
    return stackNumber;
}

void printStack(Stack *stack)
{
	Element* current;
    if (stack == NULL)
    {
        exit(EXIT_FAILURE);
    }
    
	current = stack->first;

    while (current != NULL)
    {
        printf("%d\n", current->number);
        current = current->next;
    }

    printf("\n");
}
