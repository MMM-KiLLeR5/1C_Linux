// Module registration
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

// Creation of character device
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>

// Sync
#include <linux/mutex.h>
#include <linux/wait.h>

// Others
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h> // kmalloc()

//------------------------------------------------------------------------------

// Variables

#define FIFO_SIZE 256

static char fifo_buffer[FIFO_SIZE];
static int read_pos = 0;
static int write_pos = 0;
static int bytes_in_fifo = 0;
static DEFINE_MUTEX(fifo_mutex);
static DECLARE_WAIT_QUEUE_HEAD(read_queue);
static DECLARE_WAIT_QUEUE_HEAD(write_queue);

//------------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Vashkevich");
MODULE_DESCRIPTION("Implements fifo communication between processes");
MODULE_VERSION("0.01");

static int __init fifo_proc_init(void);
static void __exit fifo_proc_exit(void);

//------------------------------------------------------------------------------

// file_operations implementation

static int fifo_open(struct inode* inode, struct file* file);
static int fifo_release(struct inode* inode, struct file* file);
static ssize_t fifo_read(struct file* filp,
                         char __user* buf,
                         size_t len,
                         loff_t* off);
static ssize_t fifo_write(struct file* filp,
                          const char* buf,
                          size_t len,
                          loff_t* off);
static struct file_operations fifo_fops =
    {
        .owner      = THIS_MODULE,
        .read       = fifo_read,
        .write      = fifo_write,
        .open       = fifo_open,
        .release    = fifo_release,
    };

//------------------------------------------------------------------------------

// Implementation

static int fifo_open(struct inode* inode, struct file* file) {
  return 0;
}

static int fifo_release(struct inode* inode, struct file* file) {
  return 0;
}

static ssize_t fifo_read(struct file* filp,
                         char __user* buf,
                         size_t len,
                         loff_t* off) {
  mutex_lock(&fifo_mutex);
  while (bytes_in_fifo == 0) {
    mutex_unlock(&fifo_mutex);
    if (wait_event_interruptible(read_queue, bytes_in_fifo > 0)) {
      return -ERESTARTSYS;
    }
    if (signal_pending(current)) {
      return -ERESTARTSYS;
    }
    mutex_lock(&fifo_mutex);
  }

  if (copy_to_user(buf, &fifo_buffer[read_pos], 1)) {
    mutex_unlock(&fifo_mutex);
    return -EFAULT;
  }
  read_pos = (read_pos + 1) % FIFO_SIZE;
  bytes_in_fifo--;
  wake_up(&write_queue);
  mutex_unlock(&fifo_mutex);
  return 1;
}

static ssize_t fifo_write(struct file* filp,
                          const char* buf,
                          size_t len,
                          loff_t* off) {
  mutex_lock(&fifo_mutex);
  while (bytes_in_fifo == FIFO_SIZE) {
    mutex_unlock(&fifo_mutex);
    if (wait_event_interruptible(write_queue, bytes_in_fifo < FIFO_SIZE)) {
      return -ERESTARTSYS;
    }
    if (signal_pending(current)) {
      return -ERESTARTSYS;
    }
    mutex_lock(&fifo_mutex);
  }

  if (copy_from_user(&fifo_buffer[write_pos], buf, 1)) {
    mutex_unlock(&fifo_mutex);
    return -EFAULT;
  }
  write_pos = (write_pos + 1) % FIFO_SIZE;
  bytes_in_fifo++;
  wake_up(&read_queue);
  mutex_unlock(&fifo_mutex);
  return 1;
}

//------------------------------------------------------------------------------

// Module init/exit.

static dev_t dev_num;
static struct cdev fifo_cdev;
static struct class* fifo_class;

static int __init fifo_proc_init(void) {
  printk(KERN_INFO "Start inserting process_fifo_module...");

  // Allocating Major number
  if ((alloc_chrdev_region(&dev_num, 0, 1, "fifo_proc")) < 0) {
    pr_err("Cannot allocate major number for fifo_proc device\n");
    return -1;
  }
  pr_info("Major = %d Minor = %d \n", MAJOR(dev_num), MINOR(dev_num));

  // Creating cdev structure
  cdev_init(&fifo_cdev, &fifo_fops);

  // Adding character device to the system
  if ((cdev_add(&fifo_cdev, dev_num, 1)) < 0) {
    pr_err("Cannot add the device 'fifo_proc' to the system\n");
    goto r_class;
  }

  // Creating struct class
  fifo_class = class_create("fifo_proc");
  if (IS_ERR(fifo_class)) {
    pr_err("Cannot create the struct class for 'fifo_proc' device\n");
    goto r_class;
  }

  // Creating device
  if (IS_ERR(device_create(fifo_class, NULL, dev_num, NULL, "fifo_proc"))) {
    pr_err("Cannot create 'fifo_proc' device\n");
    goto r_device;
  }

  pr_info("process_fifo_module inserted successfully\n");
  return 0;

  // Clean in case there are errors.
  r_device:
  class_destroy(fifo_class);
  r_class:
  unregister_chrdev_region(dev_num, 1);
  return -1;
}

static void __exit fifo_proc_exit(void) {
  pr_info("Start removing process_fifo_module...\n");

  device_destroy(fifo_class, dev_num);
  class_destroy(fifo_class);
  unregister_chrdev_region(dev_num, 1);

  pr_info("process_fifo_module removed successfully\n");
}

module_init(fifo_proc_init);
module_exit(fifo_proc_exit);
