#ifndef H_STACK
#define H_STACK

/**
 * \file stack.h
 * \brief stack Functions declarations
 * \author Quentin.C
 * \version 0.5
 * \date August 9th 2016
 *
 * Functions only used by idStack.c to manipulate Stacks
 *
 */
	typedef struct Element Element;
	typedef struct Stack Stack;
/**
 * \struct Element
 * \brief Contain data
 *
 */
	struct Element
	{
		void *number;  //data
		Element *next;  //pointer to the next Element
	};
/**
 * \struct Stack
 * \brief Contain Elements
 *
 */
	struct Stack
	{
		Element *first;  //pointer to the first Element
	};
	Stack* initialize();
	void deinitialize(Stack*);
    void stackPush(Stack*, void*, int);
	void* firstStackPop(Stack*);
	int stackNumberCount(Stack*, unsigned int,unsigned int,unsigned int,unsigned int);
	void* stackPop(Stack*,int,unsigned int,unsigned int,unsigned int,int);
	void printStack(Stack*);
#endif
