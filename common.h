#ifndef MY_COMMON_H
#define MY_COMMON_H

#define DEVICE_PATH "/dev/mydriver"

#define MAJOR_NUM 200

struct data {
    int length;
    char *data;
};
#endif