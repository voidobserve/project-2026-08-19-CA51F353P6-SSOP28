#ifndef __USER_TYPEDEF_H__
#define __USER_TYPEDEF_H__

/*
    printf("unsigned char size: %d\n", (int)sizeof(unsigned char));	  // 打印结果 1
    printf("unsigned short size: %d\n", (int)sizeof(unsigned short)); // 打印结果 2
    printf("unsigned int size: %d\n", (int)sizeof(unsigned int));	  // 打印结果 2
    printf("unsigned long size: %d\n", (int)sizeof(unsigned long));	  // 打印结果 4
*/

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#endif
