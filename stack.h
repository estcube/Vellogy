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
		Element *first;
	};
	Stack* initialize();
	void deinitialize(Stack*);
    void stackPush(Stack*, void*, int);
	void* firstStackPop(Stack*);
	int stackNumberCount(Stack*, unsigned int,unsigned int,unsigned int,unsigned int);
	void* stackPop(Stack*,int,unsigned int,unsigned int,unsigned int,int);
	void printStack(Stack*);
#endif
