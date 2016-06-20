#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "idStack.h"
#include "minunit.h"

IdStack* myIdStack;
IdElement* idElement = NULL;
int* intP;
int aInt = 2;
int bInt = -32503;
int cInt = 2048;
int dInt = 2147483647;
int eInt = -2147483648;
int fInt = 0;
float* floatP;
float aFloat = 1.2354;
float bFloat = -12365;
float cFloat = -0.00348979;
float dFloat = 340000000000000000000000000000000000000.;
float eFloat = -340000000000000000000000000000000000000.;
float fFloat = 0;
char* charP;
char aChar = 'a';
char bChar = 'b';
char cChar = 'c';
char dChar = 'Z';
char eChar = 'z';
char fChar = 'A';
double* doubleP;
double aDouble = 1.2354;
double bDouble = -12365;
double cDouble = -0.00348979;
double dDouble = 340000000000000000000000000000000000000.;
double eDouble = -340000000000000000000000000000000000000.;
double fDouble = 0;

void test_setup() {
}

void test_teardown() {
}

MU_TEST(test_initializeIdStack) {
	mu_check((myIdStack = idInitialize()) != NULL);
}

MU_TEST(test_initializeIdElements) {
	mu_check(idStackPush(myIdStack, 1,4,0,1000) != NULL);
    mu_check(idStackPush(myIdStack, 5,4,200,50)!=NULL);
    mu_check(idStackPush(myIdStack, 9,1,500,25)!=NULL);
    mu_check(idStackPush(myIdStack, 13,1,0,100)!=NULL);
    mu_check(idStackPush(myIdStack, 17,8,25000,1000)!=NULL);
    mu_check(idStackPush(myIdStack, 21,8,0,500)!=NULL);
}

MU_TEST(test_pushTestInt) {
	mu_check((dataIdStackPush(myIdStack,1,&aInt))!=NULL);
	mu_check(*(int*)(searchIdElement(myIdStack,1)->dataStack->first->number) == aInt);
	mu_check(dataIdStackPush(myIdStack,1,&bInt)!=NULL);
	mu_check(*(int*)(searchIdElement(myIdStack,1)->dataStack->first->number) == bInt);
	mu_check(dataIdStackPush(myIdStack,1,&cInt)!=NULL);
	mu_check(*(int*)(searchIdElement(myIdStack,1)->dataStack->first->number) == cInt);
	mu_check(dataIdStackPush(myIdStack,1,&dInt)!=NULL);
	mu_check(*(int*)(searchIdElement(myIdStack,1)->dataStack->first->number) == dInt);
	mu_check(dataIdStackPush(myIdStack,1,&eInt)!=NULL);
	mu_check(*(int*)(searchIdElement(myIdStack,1)->dataStack->first->number) == eInt);
	mu_check(dataIdStackPush(myIdStack,1,&fInt)!=NULL);
	mu_check(*(int*)(searchIdElement(myIdStack,1)->dataStack->first->number) == fInt);
}
MU_TEST(test_pushTestFloat) {
	mu_check((dataIdStackPush(myIdStack,5,&aFloat))!=NULL);
	mu_check(*(float*)(searchIdElement(myIdStack,5)->dataStack->first->number) == aFloat);
	mu_check(dataIdStackPush(myIdStack,5,&bFloat)!=NULL);
	mu_check(*(float*)(searchIdElement(myIdStack,5)->dataStack->first->number) == bFloat);
	mu_check(dataIdStackPush(myIdStack,5,&cFloat)!=NULL);
	mu_check(*(float*)(searchIdElement(myIdStack,5)->dataStack->first->number) == cFloat);
	mu_check(dataIdStackPush(myIdStack,5,&dFloat)!=NULL);
	mu_check(*(float*)(searchIdElement(myIdStack,5)->dataStack->first->number) == dFloat);
	mu_check(dataIdStackPush(myIdStack,5,&eFloat)!=NULL);
	mu_check(*(float*)(searchIdElement(myIdStack,5)->dataStack->first->number) == eFloat);
	mu_check(dataIdStackPush(myIdStack,5,&fFloat)!=NULL);
	mu_check(*(float*)(searchIdElement(myIdStack,5)->dataStack->first->number) == fFloat);
}


MU_TEST(test_pushTestDouble) {
	mu_check((dataIdStackPush(myIdStack,17,&aDouble))!=NULL);
	mu_check(*(double*)(searchIdElement(myIdStack,17)->dataStack->first->number) == aDouble);
	mu_check(dataIdStackPush(myIdStack,17,&bDouble)!=NULL);
	mu_check(*(double*)(searchIdElement(myIdStack,17)->dataStack->first->number) == bDouble);
	mu_check(dataIdStackPush(myIdStack,21,&cDouble)!=NULL);
	mu_check(*(double*)(searchIdElement(myIdStack,21)->dataStack->first->number) == cDouble);
	mu_check(dataIdStackPush(myIdStack,21,&dDouble)!=NULL);
	mu_check(*(double*)(searchIdElement(myIdStack,21)->dataStack->first->number) == dDouble);
	mu_check(dataIdStackPush(myIdStack,21,&eDouble)!=NULL);
	mu_check(*(double*)(searchIdElement(myIdStack,21)->dataStack->first->number) == eDouble);
	mu_check(dataIdStackPush(myIdStack,21,&fDouble)!=NULL);
	mu_check(*(double*)(searchIdElement(myIdStack,21)->dataStack->first->number) == fFloat);
}

MU_TEST(test_pushTestChar) {
	mu_check((dataIdStackPush(myIdStack,9,&aChar))!=NULL);
	mu_check(*(char*)(searchIdElement(myIdStack,9)->dataStack->first->number) == aChar);
	mu_check(dataIdStackPush(myIdStack,9,&bChar)!=NULL);
	mu_check(*(char*)(searchIdElement(myIdStack,9)->dataStack->first->number) == bChar);
	mu_check(dataIdStackPush(myIdStack,13,&cChar)!=NULL);
	mu_check(*(char*)(searchIdElement(myIdStack,13)->dataStack->first->number) == cChar);
	mu_check(dataIdStackPush(myIdStack,13,&dChar)!=NULL);
	mu_check(*(char*)(searchIdElement(myIdStack,13)->dataStack->first->number) == dChar);
	mu_check(dataIdStackPush(myIdStack,13,&eChar)!=NULL);
	mu_check(*(char*)(searchIdElement(myIdStack,13)->dataStack->first->number) == eChar);
	mu_check(dataIdStackPush(myIdStack,13,&fChar)!=NULL);
	mu_check(*(char*)(searchIdElement(myIdStack,13)->dataStack->first->number) == fChar);
}

MU_TEST(test_searchIdElement) {
	mu_check(searchIdElement(myIdStack, 1) != NULL);
	mu_check(searchIdElement(myIdStack, 5) != NULL);
	mu_check(searchIdElement(myIdStack, 9) != NULL);
	mu_check(searchIdElement(myIdStack, 13) != NULL);
	mu_check(searchIdElement(myIdStack, 17) != NULL);
	mu_check(searchIdElement(myIdStack, 21) != NULL);
	mu_check(searchIdElement(myIdStack, 0) == NULL);
	mu_check(searchIdElement(myIdStack, -1) == NULL);
	mu_check(searchIdElement(myIdStack, 100) == NULL);
}

MU_TEST(test_idStackPop) {
	mu_check(idStackPop(myIdStack, 0) == -1);
	mu_check(idStackPop(myIdStack, -1) == -1);
	mu_check(idStackPop(myIdStack, -1) == -1);
	mu_check(idStackPop(myIdStack, 17) == 0);
	mu_check(searchIdElement(myIdStack, 17) == NULL);
}

MU_TEST(test_dataIdStackPop) {
	dataIdStackPop(myIdStack, 1, 3000, 5000);
	mu_check(*(int*)(searchIdElement(myIdStack,1)->dataStack->first->number) == cInt);
}

MU_TEST(test_deinitializeTest) {
	idDeinitialize(myIdStack);
}

MU_TEST_SUITE(test_suite) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
	MU_RUN_TEST(test_initializeIdStack);
	MU_RUN_TEST(test_initializeIdElements);

	MU_RUN_TEST(test_pushTestInt);
	MU_RUN_TEST(test_pushTestFloat);
	MU_RUN_TEST(test_pushTestChar);
	MU_RUN_TEST(test_pushTestDouble);

	printIdStack(myIdStack);

	MU_RUN_TEST(test_searchIdElement);
	MU_RUN_TEST(test_idStackPop);

	printIdStack(myIdStack);

	MU_RUN_TEST(test_dataIdStackPop);

	printIdStack(myIdStack);

	MU_RUN_TEST(test_deinitializeTest);
}

int main()
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return 0;
}
