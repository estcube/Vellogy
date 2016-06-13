#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

int main()

{
    Stack* myStack;
	myStack = initialize();


    toStack(myStack, 4);
    toStack(myStack, 8);
    toStack(myStack, 15);
    toStack(myStack, 16);
    toStack(myStack, 23);
    toStack(myStack, 42);

    printf("\nStack state :\n");
    printStack(myStack);

	/*printf("Je depile %d\n", unstack(myStack));

    printf("Je depile %d\n", unstack(myStack));


    printf("\nStack state :\n");

    printStack(myStack);*/

/*unstack(myStack);
unstack(myStack);
unstack(myStack);
unstack(myStack);
unstack(myStack);
unstack(myStack);
unstack(myStack);
unstack(myStack);*/
deinitialize(myStack);
    return 0;
}
