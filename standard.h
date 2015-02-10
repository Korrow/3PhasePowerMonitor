/********************************************************************
* standard.h
* 
* This contains typedefs and macros that are used throughout the
* project. 
*  
* 
* Author: Keegan Morrow
* Date: 6/6/2013
* Project: 3 Phase Power monitor 
*  
********************************************************************/

#ifndef STANDARD_H_
#define STANDARD_H_

typedef unsigned char  INT8U;
typedef signed char  INT8S;
typedef unsigned short  INT16U;
typedef signed short  INT16S;
typedef unsigned long  INT32U;
typedef signed long  INT32S;
typedef signed long long int INT64S;
typedef unsigned long long int INT64U;

//*****************************************************************************
//
// Macros for hardware access
//
//*****************************************************************************
#define HWREG(x)                                                              \
        (*((volatile unsigned int *)(x)))
#define HWREGB(x)                                                             \
        (*((volatile unsigned char *)(x)))


#define BIT_SET(Location,Mask) ((Location) |= (Mask))
#define BIT_CLR(Location,Mask) ((Location) &= ~(Mask))
#endif /* STANDARD_H_ */
