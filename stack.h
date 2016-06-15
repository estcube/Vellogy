#ifndef H_STACK
#define H_STACK
	typedef struct Element Element;
	typedef struct Stack Stack;
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
	Stack* initialize(int);
	void deinitialize(Stack*);
    void stackPush(Stack*, void*);
	void* stackPop(Stack*,void*);
	void printStack(Stack*);
#endif
