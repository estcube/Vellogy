#include "stack.h"

#ifndef H_IDSTACK
#define H_IDSTACK
	typedef struct IdElement IdElement;
	typedef struct IdStack IdStack;
	struct IdElement
	{
		int id;
		int type;
		int startTime;
		int timeInterval;
		Stack *dataStack;
		IdElement *next;
	};

	struct IdStack
	{
		IdElement *first;
	};

	IdStack* idInitialize();
	void idDeinitialize(IdStack*);
	IdElement* searchIdElement(IdStack*, int);
    void idStackPush(IdStack*,int,int,int,int);
	int firstIdStackPop(IdStack*);
	int idStackPop(IdStack*,int);
	void printIdStack(IdStack*);

	IdElement* dataIdStackPush(IdStack*, int, void*);
#endif
