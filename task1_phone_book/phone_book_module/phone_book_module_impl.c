// Module registration.
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

// Creation of character device.
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>

// Local.
#include "phone_book_api.h"

#define DEVICE_BUF_SIZE 1024
#define COMMAND_MAX_LEN 5
#define NAME_MAX_LEN 30
#define SURNAME_MAX_LEN 30
#define PHONE_MAX_LEN 15
#define EMAIL_MAX_LEN 30

//------------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Vashkevich");
MODULE_DESCRIPTION("Phone Book");
MODULE_VERSION("0.01");

static int __init mipt_phone_book_init(void);
static void __exit mipt_phone_book_exit(void);

//------------------------------------------------------------------------------

// Creating character device

dev_t dev = 0;
static struct class* dev_class;
static struct cdev mipt_cdev;

//------------------------------------------------------------------------------

// file_operations implementation.

static int mipt_open(struct inode* inode, struct file* file);
static int mipt_release(struct inode* inode, struct file* file);
static ssize_t mipt_read(struct file* filp,
                         char __user* buf,
                         size_t len,
                         loff_t* off);
static ssize_t mipt_write(struct file* filp,
                          const char* buf,
                          size_t len,
                          loff_t* off);
static struct file_operations fops =
    {
        .owner      = THIS_MODULE,
        .read       = mipt_read,
        .write      = mipt_write,
        .open       = mipt_open,
        .release    = mipt_release,
    };

//------------------------------------------------------------------------------

// Constants.

// 100 is error message padding.
static char device_buf[DEVICE_BUF_SIZE + 100 + 1] = {};

static char add_cmd[] = "add";
static char get_cmd[] = "get";
static char del_cmd[] = "del";

//------------------------------------------------------------------------------

// Open & Release.

static int mipt_open(struct inode* inode, struct file* filp) {
  return 0;
}

static int mipt_release(struct inode* inode, struct file* filp) {
  return 0;
}

//------------------------------------------------------------------------------

// Read.

/*
** This function will be called when we Read the Device file
*/
static ssize_t mipt_read(struct file* filp,
                         char __user* buf,
                         size_t len,
                         loff_t* off) {
  if (len == 0) {
    return 0;
  }
  if (len > DEVICE_BUF_SIZE - *off) {
    len = DEVICE_BUF_SIZE - *off;
  }

  size_t msg_len = strlen(device_buf);
  if (*off >= msg_len) {
    return 0;
  } else if (msg_len < len) {
    len = msg_len;
  }

  // copy_to_user() == memcpy(), but make security checks.
  if (copy_to_user(buf, device_buf + *off, len) != 0) {
    return -EFAULT;
  }

  *off += (loff_t) len;
  return (loff_t) len;
}

//------------------------------------------------------------------------------

// Write.

void display_user_data(user_data_t* user) {
  sprintf(
      device_buf,
      "name: %s\n"
      "surname: %s\n"
      "age: %zu\n"
      "phone: %s\n"
      "email: %s\n",
      user->name,
      user->surname,
      user->age,
      user->phone,
      user->email
  );
}

// Write can be made gradually (therefore offset if provided to write function).
// So parsing instruction may fail and we "try" to parse it.
void try_parse_write_command(void) {
  char* cmd = kernel_kmalloc(COMMAND_MAX_LEN + 1);

  if (sscanf(device_buf, "%s", cmd) != 1) {
    goto parse_fail;
  }
  char* cur_wb = device_buf + strlen(cmd) + 1;

  if (strcmp(cmd, add_cmd) == 0) {
    char* name = kernel_kmalloc(NAME_MAX_LEN + 1);
    char* surname = kernel_kmalloc(SURNAME_MAX_LEN);
    size_t age = 0;
    char* phone = kernel_kmalloc(PHONE_MAX_LEN);
    char* email = kernel_kmalloc(EMAIL_MAX_LEN);

    if (sscanf(cur_wb,
               "%s %s %zu %s %s",
               name,
               surname,
               &age,
               phone,
               email) != 5) {
      kfree(name);
      kfree(surname);
      kfree(phone);
      kfree(email);
      goto parse_fail;
    }

    user_data_t new_user = {
      .name = name,
      .surname = surname,
      .age = age,
      .phone = phone,
      .email = email,
    };
    add_pb_user(&new_user);

    kfree(name);
    kfree(surname);
    kfree(phone);
    kfree(email);

    sprintf(device_buf, "New user inserted successfully.\n");
    goto return_with_clear;
  }
  if (strcmp(cmd, get_cmd) == 0) {
    char* surname = kernel_kmalloc(SURNAME_MAX_LEN + 1);
    if (sscanf(cur_wb, "%s", surname) != 1) {
      kfree(surname);
      goto parse_fail;
    }

    user_data_rbtree_search(&handbook.tree, surname);
    user_data_impl* ud_impl = user_data_rbtree_search(&handbook.tree, surname);

    if (ud_impl == NULL) {
      sprintf(device_buf, "User not found.\n");
      kfree(surname);
      goto return_with_clear;
    }
    kfree(surname);

    display_user_data(ud_impl->user);
    goto return_with_clear;
  }
  if (strcmp(cmd, del_cmd) == 0) {
    char* surname = kernel_kmalloc(SURNAME_MAX_LEN + 1);
    if (sscanf(cur_wb, "%s", surname) != 1) {
      kfree(surname);
      goto parse_fail;
    }

    if (del_pb_user(surname) == -1) {
      sprintf(device_buf, "User '%s' not found.\n", surname);
    }
    sprintf(device_buf, "User '%s' successfully deleted.\n", surname);

    kfree(surname);
    goto return_with_clear;
  }

  parse_fail:
  pr_info("Parse error (probably write buffer is not fully flushed).\n"
          "Current buffer: %s\n", device_buf);
  char tmp_buf[DEVICE_BUF_SIZE + 1] = {};
  strcpy(tmp_buf, device_buf);
  sprintf(device_buf, "Parse error (probably write buffer is not fully flushed).\n"
                      "Current buffer: %s\n", tmp_buf);

  return_with_clear:
  kfree(cmd);
}

/*
** This function will be called when we write the Device file
*/
static ssize_t mipt_write(struct file* filp,
                          const char __user* buf,
                          size_t len,
                          loff_t* off) {
  if (len == 0) {
    return 0;
  }
  if (*off + 1 > DEVICE_BUF_SIZE) {
    return 0;
  }
  if (*off + len + 1 > DEVICE_BUF_SIZE) {
    len = DEVICE_BUF_SIZE - *off - 1;
  }
  if (copy_from_user(device_buf + *off, buf, len) != 0) {
    return -EFAULT; // address not from user process
  }
  device_buf[*off + len] = '\0';

  try_parse_write_command();

  *off += (loff_t) len;
  return (loff_t) len;
}

//------------------------------------------------------------------------------

// Module init/exit.

static int __init mipt_phone_book_init(void) {
  printk(KERN_INFO "Start inserting kernel module...");

  // Allocating Major number
  if ((alloc_chrdev_region(&dev, 0, 1, "mipt_pb")) < 0) {
    pr_err("Cannot allocate major number for device\n");
    return -1;
  }
  pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

  // Creating cdev structure
  cdev_init(&mipt_cdev, &fops);

  // Adding character device to the system
  if ((cdev_add(&mipt_cdev, dev, 1)) < 0) {
    pr_err("Cannot add the device to the system\n");
    goto r_class;
  }

  // Creating struct class
  dev_class = class_create("mipt_pb");
  if (IS_ERR(dev_class)) {
    pr_err("Cannot create the struct class for device\n");
    goto r_class;
  }

  // Creating device
  if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "mipt_pb"))) {
    pr_err("Cannot create the Device\n");
    goto r_device;
  }

  // Init rbtree structure.
  init_handbook();

  pr_info("Kernel module inserted successfully\n");
  return 0;

  // Clean in case there are errors.
  r_device:
  class_destroy(dev_class);
  r_class:
  unregister_chrdev_region(dev, 1);
  return -1;
}

static void __exit mipt_phone_book_exit(void) {
  pr_info("Start removing kernel module...\n");

  device_destroy(dev_class, dev);
  class_destroy(dev_class);
  unregister_chrdev_region(dev, 1);

  pr_info("Kernel module removed successfully\n");
}

module_init(mipt_phone_book_init)
module_exit(mipt_phone_book_exit)
