#include "user.h"


int main() {

    struct data *d = malloc(sizeof(struct data));
    d->length = 3;
    d->data = malloc(d->length);

    memcpy(d->data, "xyz", 3);

    int fd = open(DEVICE_PATH, O_RDWR);
    if(fd < 0) {
        printf("Couldn't open device file\n");
        return -1;
    }

    int ret = ioctl(fd, PUSH_DATA, d);
    if(ret < 0) {
        printf("PUSH_DATA failed\n");
        return -1;
    }

    close(fd);
    free(d->data);
    free(d);

}