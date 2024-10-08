#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/errno.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/ioctl.h>

#include "common.h"
#define DEVICE_NAME "mydriver"
#define MAX_Q_SIZE 1024
#define SET_SIZE_OF_QUEUE _IOW(MAJOR_NUM, 1, int * ) 
#define PUSH_DATA _IOW(MAJOR_NUM, 2, struct data * ) 
#define POP_DATA _IOWR(MAJOR_NUM, 3, struct data * ) 



struct circular_queue {
    struct data **queue;
    int head;
    int size;
    int tail;
    int count;
};
static struct circular_queue circ_queue;
static DECLARE_WAIT_QUEUE_HEAD(my_queue);

static void init_queue(int size)
{
    pr_info("mydriver: init queue");

    // If a queue was already allocated, free it.
    if(circ_queue.queue)
    {
        for (int i = 0; i < circ_queue.count; i++) {
            kfree(circ_queue.queue[i]->data);
            kfree(circ_queue.queue[i]);
        }
        kfree(circ_queue.queue);
        pr_info("mydriver: Freed previously allocated queue");
    }

    circ_queue.queue = kmalloc_array(size, sizeof(struct data), GFP_KERNEL);
    circ_queue.size = size;
    circ_queue.head = 0;
    circ_queue.tail = 0;
    circ_queue.count = 0;
    pr_info("mydriver: allocated queue of size %d", size);
}

static int enqueue(struct data * new_data)
{
    pr_info("mydriver: enqueue");
    if (circ_queue.count >= circ_queue.size) {
        pr_info("mydriver: queue overflow");
        return -1;
    }

    circ_queue.queue[circ_queue.tail] = new_data;
    circ_queue.tail = (circ_queue.tail + 1) % circ_queue.size;
    circ_queue.count++;
    pr_info("mydriver: enqueue: Pushed data");
    return 0;
}

static int dequeue(struct data ** out)
{
    if (circ_queue.count <= 0) {
        pr_info("mydriver: Empty Queue");
        return -1;
    }

    *out = circ_queue.queue[circ_queue.head];
    circ_queue.head = (circ_queue.head + 1) % circ_queue.size;
    circ_queue.count--;
    return 0;
}

static int device_open (struct inode *inode, struct file * fp) 
{
    pr_info("mydriver: Device opened");
    return 0;
}

static int device_release (struct inode * inode, struct file * fp) 
{
    pr_info("mydriver: Device closed");
    return 0;
}

static long device_ioctl (struct file * fp, unsigned int ioctl_num, unsigned long ioctl_param)
{    
    pr_info("mydriver: inside ioctl handler");
    int size;
    struct data *user_data;
    struct data *queue_data;

    switch (ioctl_num) {
        case SET_SIZE_OF_QUEUE: {
            get_user(size, (int __user *)ioctl_param);
            pr_info("mydriver: SET_SIZE_OF_QUEUE: %d", size);
            if (size > MAX_Q_SIZE || size <= 0) {
                pr_info("mydriver: Invalid Queue size");
                return -EINVAL;
            }
            init_queue(size);
            break;
        }

        case PUSH_DATA: {
            pr_info("mydriver: PUSH_DATA");
            user_data = kmalloc(sizeof(struct data), GFP_KERNEL);
            //copy all of user's "data"
            copy_from_user(user_data, (struct data __user *)ioctl_param, sizeof(struct data));
            char * data_addr = user_data->data;

            //allocate mem for users "data->data"
            user_data->data = kmalloc(user_data->length, GFP_KERNEL);
            //copy into it
            copy_from_user(user_data->data, data_addr, user_data->length);
            pr_info("mydriver: PUSH_DATA: %d", user_data->length);

            if(enqueue(user_data) == -1) {
                pr_info("mydriver: PUSH failed");
                return -EFAULT;
            }

            pr_info("mydriver: PUSH successfull");
            wake_up_interruptible(&my_queue);
            break;
        }

        case POP_DATA: {
            pr_info("mydriver: POP_DATA");

            if (circ_queue.count <= 0) {
                wait_event_interruptible(my_queue, circ_queue.count > 0);
            }

            if(dequeue(&queue_data) == -1) {
                pr_info("mydriver: POP_DATA failed");
                return -EFAULT;
            }

            // copy user's data to check length 
            struct data * u_data = kmalloc(sizeof(struct data), GFP_KERNEL);
            copy_from_user(u_data, (struct data __user *)ioctl_param, sizeof(struct data));
            
            if (u_data->length != queue_data->length) {
                pr_info("mydriver: POP: Invalid length, expected %d, got %d", queue_data->length, u_data->length);
                return -EFAULT;
            }

            //copy data to user
            copy_to_user(u_data->data, queue_data->data, queue_data->length);
            pr_info("mydriver: POP successfull");
            kfree(queue_data->data);
            kfree(queue_data);
            kfree(u_data);
            break;
        }

        default: {
            pr_info("mydriver: hit default");
            return -EINVAL;
        }


    }


    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl,
};
static struct class *cls;

static int __init mydriver_init(void)
{   
    int major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_info("mydriver: couldn't register chrdev %d", major);
        return major;
    }

    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    cls = class_create(DEVICE_NAME);
    #else
    cls = class_create(THIS_MODULE, DEVICE_NAME);
    #endif

    device_create(cls, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_NAME);
    pr_info("Device created on /dev/%s", DEVICE_NAME);

    return 0;
}

static void __exit mydriver_exit(void)
{

    device_destroy(cls, MKDEV(MAJOR_NUM, 0));
    class_destroy(cls);

    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

}

module_init(mydriver_init);
module_exit(mydriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Immads Mir");
MODULE_DESCRIPTION("A dynamic circular queue with ioctl calls");