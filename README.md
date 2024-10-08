## Dynamic circular Queue with IOctl calls support.

### Building
`$ make`

to build the kernel module (mydriver.c)

`$ gcc config.c -o config`

`$ gcc fill.c -o fill`

`$ gcc read.c -o read`

to build userspace programs.

### Tested on `Linux Kernel 6.10.7`

