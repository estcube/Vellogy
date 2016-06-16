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
    void stackPush(Stack*, void*, int numberSize);
	void* stackPop(Stack*,void*);
	void printStack(Stack*);
#endif
