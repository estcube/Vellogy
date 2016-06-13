#ifndef H_STACK
#define H_STACK
	typedef struct Element Element;
	typedef struct Stack Stack;
	Stack* initialize();
	void deinitialize(Stack*);
    void toStack(Stack*, int);
	int unstack(Stack*);
	void printStack(Stack*);
#endif
