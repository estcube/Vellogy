#include "stack.h"

#ifndef H_IDSTACK
#define H_IDSTACK

/**
 * \file idStack.h
 * \brief IdStack Functions declarations
 * \author Quentin.C
 * \version 0.5
 * \date August 9th 2016
 *
 * Functions used to initialize, deinitialize, Push and Pop IdStacks and Stacks
 *
 */

	typedef struct IdElement IdElement;
	typedef struct IdStack IdStack;
/**
 * \enum Data_type
 * \brief Existing data types.
 *
 * Used to allocate the good memory size to store the data.
 */
	enum Data_type
	{
		UINT8_T,INT8_T, UINT16_T,INT16_T,UINT32_T,INT32_T,UINT64_T,INT64_T,FLOAT,DOUBLE, LDOUBLE,CHAR
	};
	typedef enum Data_type Data_type;
/**
 * \enum Signal_type
 * \brief Existing Signal types.
 *
 */
	enum Signal_type
	{
		VOLTAGE,TEMP,OTHER_TYPE
	};
	typedef enum Signal_type Signal_type;
/**
 * \enum Id_type
 * \brief Existing Id types.
 *
 * Used to know where to push and pop data.
 */
	enum Id_type
	{
		MCU_CURR,MCU_TEMP,RTC,RAM,TEST1,TEST2,TEST3,TEST4
	};
	typedef enum Id_type Id_type;
/**
 * \struct IdElement
 * \brief Part of IdStack
 *
 */
	struct IdElement
	{
		Id_type id;
		Data_type dataType;
		Signal_type signalType;
		unsigned int startTime; //4
		unsigned int timeInterval; //4
		unsigned int dataNumber;  //4    number of data in the IdElement.
		Stack *dataStack; //8   pointer to the dataStack.
		IdElement *next; //8   pointer to the next IdElement.
	};
/**
 * \struct IdStack
 * \brief Contain IdElements
 *
 */
	struct IdStack
	{
		IdElement *first;   //pointer to the first IdElement
	};

	IdStack* idInitialize();
	void idDeinitialize(IdStack*);
	IdElement* searchIdElement(IdStack*, Id_type);
    IdElement* idStackPush(IdStack*,Id_type,Signal_type,Data_type,unsigned int,unsigned int);
	int sizeDataType(Data_type);
	int firstIdStackPop(IdStack*);
	int idStackPop(IdStack*,Id_type);
	void printIdStack(IdStack*);
	int getStartTime(IdStack*);
	int getTimeInterval(IdStack*);
	IdElement* dataIdStackPush(IdStack*, Id_type, void*);
	void* dataIdStackPop(IdStack*, Id_type,unsigned int,unsigned int);
#endif
