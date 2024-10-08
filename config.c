#include "user.h"


int main()
{
    int fd = open(DEVICE_PATH, O_RDWR);
    if(fd < 0) {
        printf("Couldn't open device file\n");
        return -1;
    }
    int size = 100;

    int ret = ioctl(fd, SET_SIZE_OF_QUEUE, &size);
    if(ret < 0) {
        printf("SET_SIZE_OF_QUEUE failed\n");
        return -1;
    }
    close(fd);

    return 0;
}