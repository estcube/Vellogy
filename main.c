#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "minunit.h"

Stack* myStack;
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

MU_TEST(test_initializeEight) {
	mu_check((myStack = initialize(8)) != NULL);
}

MU_TEST(test_initializeFour) {
	mu_check((myStack = initialize(4)) != NULL);
}

MU_TEST(test_initializeOne) {
	mu_check((myStack = initialize(1)) != NULL);
}

MU_TEST(test_pushTestInt) {
	stackPush(myStack, &aInt);
	mu_check(*(int*)(myStack->first->number) == aInt);
	stackPush(myStack, &bInt);
	mu_check(*(int*)(myStack->first->number) == bInt);
	stackPush(myStack, &cInt);
	mu_check(*(int*)(myStack->first->number) == cInt);
	stackPush(myStack, &dInt);
	mu_check(*(int*)(myStack->first->number) == dInt);
	stackPush(myStack, &eInt);
	mu_check(*(int*)(myStack->first->number) == eInt);
	stackPush(myStack, &fInt);
	mu_check(*(int*)(myStack->first->number) == fInt);
}

MU_TEST(test_pushTestFloat) {
	stackPush(myStack, &aFloat);
	mu_check(*(float*)(myStack->first->number) == aFloat);
	stackPush(myStack, &bFloat);
	mu_check(*(float*)(myStack->first->number) == bFloat);
	stackPush(myStack, &cFloat);
	mu_check(*(float*)(myStack->first->number) == cFloat);
	stackPush(myStack, &dFloat);
	mu_check(*(float*)(myStack->first->number) == dFloat);
	stackPush(myStack, &eFloat);
	mu_check(*(float*)(myStack->first->number) == eFloat);
	stackPush(myStack, &fFloat);
	mu_check(*(float*)(myStack->first->number) == fFloat);
}

MU_TEST(test_pushTestDouble) {
	stackPush(myStack, &aDouble);
	/*mu_check(*(double*)(myStack->first->number) == aDouble);
	stackPush(myStack, &bDouble);
	mu_check(*(double*)(myStack->first->number) == bDouble);
	stackPush(myStack, &cDouble);
	mu_check(*(double*)(myStack->first->number) == cDouble);
	stackPush(myStack, &dDouble);
	mu_check(*(double*)(myStack->first->number) == dDouble);
	stackPush(myStack, &eDouble);
	mu_check(*(double*)(myStack->first->number) == eDouble);
	stackPush(myStack, &fDouble);
	mu_check(*(double*)(myStack->first->number) == fDouble);*/
}

MU_TEST(test_pushTestChar) {
	stackPush(myStack, &aChar);
	mu_check(*(char*)(myStack->first->number) == aChar);
	stackPush(myStack, &bChar);
	mu_check(*(char*)(myStack->first->number) == bChar);
	stackPush(myStack, &cChar);
	mu_check(*(char*)(myStack->first->number) == cChar);
	stackPush(myStack, &dChar);
	mu_check(*(char*)(myStack->first->number) == dChar);
	stackPush(myStack, &eChar);
	mu_check(*(char*)(myStack->first->number) == eChar);
	stackPush(myStack, &fChar);
	mu_check(*(char*)(myStack->first->number) == fChar);
}

MU_TEST(test_popTestInt) {
	free(stackPop(myStack, intP));
	if (myStack->first != NULL)
		mu_check(*(int*)(myStack->first->number) == eInt);
}

MU_TEST(test_popTestFloat) {
	free(stackPop(myStack, floatP));
	if (myStack->first != NULL)
		mu_check(*(float*)(myStack->first->number) == eFloat);
}
MU_TEST(test_popTestDouble) {
	free(stackPop(myStack, doubleP));
	if (myStack->first != NULL)
		mu_check(*(double*)(myStack->first->number) == eDouble);
}

MU_TEST(test_popTestChar) {
	free(stackPop(myStack, charP));
	if (myStack->first != NULL)
		mu_check(*(char*)(myStack->first->number) == eChar);
}

MU_TEST(test_deinitializeTest) {
	deinitialize(myStack);
}

MU_TEST_SUITE(test_suite) {
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	MU_RUN_TEST(test_initializeFour);
	MU_RUN_TEST(test_pushTestInt);
	MU_RUN_TEST(test_popTestInt);
	MU_RUN_TEST(test_deinitializeTest);

	MU_RUN_TEST(test_initializeFour);
	MU_RUN_TEST(test_pushTestFloat);
	MU_RUN_TEST(test_popTestFloat);
	MU_RUN_TEST(test_deinitializeTest);

	MU_RUN_TEST(test_initializeOne);
	MU_RUN_TEST(test_pushTestChar);
	MU_RUN_TEST(test_popTestChar);
	MU_RUN_TEST(test_deinitializeTest);

	MU_RUN_TEST(test_initializeEight);
	MU_RUN_TEST(test_pushTestDouble);
	MU_RUN_TEST(test_popTestDouble);
	MU_RUN_TEST(test_popTestDouble);
	MU_RUN_TEST(test_deinitializeTest);
}

int main()
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return 0;
}
