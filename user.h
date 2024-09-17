#ifndef MY_USER_H
#define MY_USER_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h> 
#include "common.h"
#include <stdlib.h>
#include <string.h>

#define SET_SIZE_OF_QUEUE _IOW(MAJOR_NUM, 1, int * )
#define PUSH_DATA _IOW(MAJOR_NUM, 2, struct data * ) 
#define POP_DATA _IOWR(MAJOR_NUM, 3, struct data * ) 

#endif