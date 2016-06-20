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
		int dataNumber;
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
    IdElement* idStackPush(IdStack*,int,int,int,int);
	int firstIdStackPop(IdStack*);
	int idStackPop(IdStack*,int);
	void printIdStack(IdStack*);
	int getStartTime(IdStack*);
	int getTimeInterval(IdStack*);
	IdElement* dataIdStackPush(IdStack*, int, void*);
	void dataIdStackPop(IdStack*, int, int, int);
#endif
