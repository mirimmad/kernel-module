## Dynamic circular Queue with IOctl calls support.

### Building
`$ make`

to build the kernel module (mydriver.c)

`$ gcc config.c -o config`

`$ gcc fill.c -o fill`

`$ gcc read.c -o read`

to build userspace programs.

### Usage
After building the Kernel module and userspace programs

Load the Kernel module:

`$ sudo insmod mydriver.ko`

Run the configuration program, it initialises the dynamic circular queue in the Kernel space with specified capacity:

`$ ./config`

Push some data on to the queue:

`$ ./fill`

Running this program multiple times adds more elements to the queue, and fails on overflow.

Pop data from the queue:

`$ ./read`

Running this program should print the data popped from the queue. It fails in case of underflow.

### Tested on `Linux Kernel 6.10.7`

