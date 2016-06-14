#ifndef H_STACK
#define H_STACK
	typedef struct Element Element;
	typedef struct Stack Stack;
	Stack* initialize(int);
	void deinitialize(Stack*);
    	void stackPush(Stack*, void*);
	void* stackPop(Stack*,void*);
	void printStack(Stack*);
#endif
