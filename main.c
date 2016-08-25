#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "idStack.h"
#include "minunit.h"

#include "kiss_fft.h"
#include "kiss_fftr.h"
#include "fft_freq.h"

IdStack* myIdStack;
IdElement* idElement = NULL;
int* intP;
int aInt[6] = {2,-32503 ,2048,2147483647,-2147483648,0};
float* floatP;
float aFloat[6] = {1.2354,-2.6523,5.2,20.,0};
char* charP;
char aChar [6] = {'a','b','c','Z','z','A'};
double* doubleP;
double aDouble[6] =  {1.2354,-12365,-0.00348979,340000000000000000000000000000000000000.,-340000000000000000000000000000000000000.,0};
long double aLDouble = -0.00348979;

void test_setup() {
}

void test_teardown() {
}

MU_TEST(test_initializeIdStack) {
	mu_check((myIdStack = idInitialize()) != NULL);
}

MU_TEST(test_initializeIdElements) {
	mu_check(idStackPush(myIdStack, MCU_TEMP,TEMP,INT32_T,0,1000) != NULL);
    mu_check(idStackPush(myIdStack, MCU_CURR,VOLTAGE,FLOAT,200,50)!=NULL);
    mu_check(idStackPush(myIdStack, RTC,OTHER_TYPE,CHAR,500,25)!=NULL);
    mu_check(idStackPush(myIdStack, RAM,OTHER_TYPE,CHAR,0,100)!=NULL);
    mu_check(idStackPush(myIdStack, TEST1,OTHER_TYPE,DOUBLE,25000,1000)!=NULL);
    mu_check(idStackPush(myIdStack, TEST2,OTHER_TYPE,DOUBLE,0,500)!=NULL);
	mu_check(idStackPush(myIdStack, TEST3,OTHER_TYPE,LDOUBLE,0,500)!=NULL);
}

MU_TEST(test_pushTestInt) {
	for(int i = 0; i<6; i++){
		mu_check((dataIdStackPush(myIdStack,MCU_TEMP,&aInt[i]))!=NULL);
		mu_check(*(int*)(searchIdElement(myIdStack,MCU_TEMP)->dataStack->first->number) == aInt[i]);
	}
}

MU_TEST(test_pushTestFloat) {
	for(int i = 0; i<6; i++){
		mu_check((dataIdStackPush(myIdStack,MCU_CURR,&aFloat[i]))!=NULL);
		mu_check(*(float*)(searchIdElement(myIdStack,MCU_CURR)->dataStack->first->number) == aFloat[i]);
	}
}


MU_TEST(test_pushTestDouble) {
	for(int i = 0; i<2; i++){
		mu_check((dataIdStackPush(myIdStack,TEST1,&aDouble[i]))!=NULL);
		mu_check(*(double*)(searchIdElement(myIdStack,TEST1)->dataStack->first->number) == aDouble[i]);
	}
	for(int i = 2; i<6; i++){
		mu_check((dataIdStackPush(myIdStack,TEST2,&aDouble[i]))!=NULL);
		mu_check(*(double*)(searchIdElement(myIdStack,TEST2)->dataStack->first->number) == aDouble[i]);
	}
	mu_check(dataIdStackPush(myIdStack,TEST3,&aLDouble)!=NULL);
	mu_check(*(long double*)(searchIdElement(myIdStack,TEST3)->dataStack->first->number) == aLDouble);
}

MU_TEST(test_pushTestChar) {
	for(int i = 0; i<6; i++){
		mu_check((dataIdStackPush(myIdStack,RTC,&aChar[i]))!=NULL);
		mu_check(*(char*)(searchIdElement(myIdStack,RTC)->dataStack->first->number) == aChar[i]);
	}
}

MU_TEST(test_searchIdElement) {
	mu_check(searchIdElement(myIdStack, MCU_TEMP) != NULL);
	mu_check(searchIdElement(myIdStack, MCU_CURR) != NULL);
	mu_check(searchIdElement(myIdStack, RTC) != NULL);
	mu_check(searchIdElement(myIdStack, RAM) != NULL);
	mu_check(searchIdElement(myIdStack, TEST1) != NULL);
	mu_check(searchIdElement(myIdStack, TEST2) != NULL);
	mu_check(searchIdElement(myIdStack, TEST4) == NULL);
	mu_check(searchIdElement(myIdStack, -1) == NULL);
	mu_check(searchIdElement(myIdStack, 100) == NULL);
}

MU_TEST(test_idStackPop) {
	mu_check(idStackPop(myIdStack, TEST4) == -1);
	mu_check(idStackPop(myIdStack, -1) == -1);
	mu_check(idStackPop(myIdStack, -1) == -1);
	mu_check(idStackPop(myIdStack, TEST1) == 0);
	mu_check(searchIdElement(myIdStack, TEST1) == NULL);
}

MU_TEST(test_dataIdStackPop) {
	int* adress = NULL;
	adress = dataIdStackPop(myIdStack, 1, 0000, 5000);
	for(int i = 0; i<6; i++){
		mu_check(adress[i] == aInt[i]);
	}
	free(adress);

}

MU_TEST(test_fft) {

	float* tab3;
	float* tab4;
	tab3 = dataIdStackPop(myIdStack, MCU_CURR, 200, 450);
	tab4 = fftAll(tab3,6);
	tab4 = ifftAll(tab4,6);
	for (unsigned int i = 0; i<6; i++)
		{
			printf("%f\t%f\t%e\n",tab3[i],tab4[i] ,tab4[i] - tab3[i]);
			mu_check(tab4[i] >= aFloat[i]-0.0001 && tab4[i] <= aFloat[i]+0.0001);
		}
	free(tab3);
	free(tab4);
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
	
	MU_RUN_TEST(test_fft);


	MU_RUN_TEST(test_deinitializeTest);

}

int main()
{
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
/*
unsigned int size = 1;
    float array[] = {1.2,2.4,3.0,2.3,1.0,0,1.3,3.0,4.,3.2,2.1,1.0,0.1,1.6,50.652,3.0,4.5,8.9,-20,2.3,9.0,1.2,-20,3.0,2.3,1.0,0,1.3,3.0,4.,3.2,2.1,1.0,0.1,1.6,3.0,4.5,8.9,10.,2.3,9.0,1.2,2.4,3.0,2.3,1.0,0,1.3,3.0,4.,3.2,2.1,1.0,0.1,1.6,3.0,4.5,8.9,10.,2.3,9.0,1.2,2.4,3.0,2.3,1.0,0,1.3,3.0,4.,3.2,2.1,1.0,0.1,1.6,3.0,4.5,8.9,10.,2.3,9.0};
	float* tab;
	float* tab2;

	for (size = 20; size < 21 ; size++)
	{
		tab = fftHigh(array,size);
		//TRANSMISSION
		//TO TRANSMIT IN AN OTHER WAY : Size
		tab = ifftHigh(tab,size);

		tab2 = fftLow(array,size);
		//TRANSMISSION
		//TO TRANSMIT IN AN OTHER WAY : Size
		tab2 = ifftLow(tab2,size);
	printf("IN\tOUT_LOW\tOUT_HIGH BOTH DIFF (Size = %d)\n",size);
		for (unsigned int i = 0; i<size; i++)
		{
			printf("%f\t%f\t%f\t%f\t%e\n",array[i],tab2[i],tab[i],tab2[i] + tab[i] ,tab2[i] + tab[i]-array[i]);
		}
	kiss_fft_cleanup();
	free(tab);free(tab2);*/


	return 0;
}
