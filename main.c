#include <stdio.h>
#include <stdlib.h>
#include "idStack.h"
#include "stack.h"

int main()

{
    IdStack* myIdStack;
	myIdStack = idInitialize();


    idStackPush(myIdStack, 1,4,3,4);
    idStackPush(myIdStack, 5,4,7,8);
    idStackPush(myIdStack, 9,4,11,12);
    idStackPush(myIdStack, 13,4,15,16);
    idStackPush(myIdStack, 17,4,19,20);
    idStackPush(myIdStack, 21,4,23,24);

    printf("\nStack state :\n");
	printIdStack(myIdStack);
	int* intP = malloc(sizeof(int));
	*intP = 28;
	dataIdStackPush(myIdStack,13,intP);
	dataIdStackPush(myIdStack,13,intP);
	dataIdStackPush(myIdStack,13,intP);
	dataIdStackPush(myIdStack,13,intP);
	dataIdStackPush(myIdStack,5,intP);
	dataIdStackPush(myIdStack,1,intP);
	dataIdStackPush(myIdStack,5,intP);
	dataIdStackPush(myIdStack,1,intP);
	printIdStack(myIdStack);
	idDeinitialize(myIdStack);
	free(intP);
    return 0;
}

