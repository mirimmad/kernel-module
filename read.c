#include "user.h"


int main() {

    struct data *d = malloc(sizeof(struct data));
    d->length = 3;
    d->data = malloc(d->length);


    int fd = open("/dev/immad", O_RDWR);
    if(fd < 0) {
        printf("Couldn't open device file");
        return -1;
    }

    int ret = ioctl(fd, POP_DATA, d);
    if(ret < 0) {
        printf("POP_DATA failed\n");
        return -1;
    }

    printf("received: %s\n", d->data);

    close(fd);
    free(d->data);
    free(d);    


}