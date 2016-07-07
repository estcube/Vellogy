#include "stack.h"

#ifndef H_IDSTACK
#define H_IDSTACK
	typedef struct IdElement IdElement;
	typedef struct IdStack IdStack;
	enum Data_type
	{
		UINT8_T,INT8_T, UINT16_T,INT16_T,UINT32_T,INT32_T,UINT64_T,INT64_T,FLOAT,DOUBLE, LDOUBLE,CHAR
	};
	typedef enum Data_type Data_type;
	enum Signal_type
	{
		VOLTAGE,TEMP,OTHER_TYPE
	};
	typedef enum Signal_type Signal_type;
	enum Id_type
	{
		MCU_CURR,MCU_TEMP,RTC,RAM,TEST1,TEST2,TEST3,TEST4
	};
	typedef enum Id_type Id_type;
	struct IdElement
	{
		Id_type id;
		Data_type dataType;
		Signal_type signalType;
		unsigned int startTime; //4
		unsigned int timeInterval; //4
		unsigned int dataNumber;  //4
		Stack *dataStack; //8
		IdElement *next; //8
	};

	struct IdStack
	{
		IdElement *first;
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
