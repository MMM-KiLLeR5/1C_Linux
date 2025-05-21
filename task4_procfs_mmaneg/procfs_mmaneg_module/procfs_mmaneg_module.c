// Module registration
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

// Creation of character device
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/fs.h>

// Procfs
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/mm.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <linux/mm_types.h>
#include <asm/page_64.h>
//#include <arch/x86/include/asm/io.h>
//#include <asm/io.h>
#include <asm/pgtable.h>

//#define page_to_phys(page)    ((dma_addr_t)page_to_pfn(page) << PAGE_SHIFT)

//------------------------------------------------------------------------------

// Variables

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "mmaneg"

static struct proc_dir_entry *proc_file;
static char mmaneg_buf[PROCFS_MAX_SIZE + 1];

//------------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Vashkevich");
MODULE_DESCRIPTION("Implements /proc/mmaneg for va->pa mapping");
MODULE_VERSION("0.01");

static int __init mmaneg_init(void);
static void __exit mmaneg_exit(void);

//------------------------------------------------------------------------------

// file_operations implementation

static int mmaneg_open(struct inode *inode, struct file *file);
static ssize_t mmaneg_write(struct file *file, const char __user *buf, size_t len, loff_t *pos);

static struct proc_ops mmaneg_fops =
    {
        .proc_open = mmaneg_open,
        .proc_read = seq_read,
        .proc_write = mmaneg_write,
        .proc_lseek = seq_lseek,
        .proc_release = single_release,
    };

//------------------------------------------------------------------------------

// Implementation

static int listvma_show(void) {
  struct task_struct *task = current;
  struct mm_struct *mm = get_task_mm(task);
  struct vm_area_struct *vma;

//  down_read(&current->mm->mmap_sem)
  mmap_read_lock(mm);

  VMA_ITERATOR(iter, mm, 0);
  pr_info("VMA for process %d:\n", task->pid);
  for_each_vma(iter, vma) {
    pr_info("Start: 0x%lx, End: 0x%lx\n", vma->vm_start, vma->vm_end);
  }

  mmap_read_unlock(mm);
  return 0;
}

static int findpage_show(unsigned long addr) {
  struct task_struct *task = current;
  struct mm_struct *mm = get_task_mm(task);
  struct vm_area_struct *vma;
  struct page *page;

  mmap_read_lock(mm);

  vma = find_vma(mm, addr);
  if (!vma) {
    pr_info("Address not found in any VMA\n");
  } else {
    page = follow_page(vma, addr, FOLL_GET);
    if (page) {
//      pr_info("Virtual Address: 0x%lu, Physical Address: 0x%lu\n", addr, page_to_pfn(page));
      pr_info("Virtual Address: 0x%lu, Physical Address: 0x%lu\n", addr, page_to_phys(page));
      put_page(page);
    } else {
      pr_info("Page not found\n");
    }
  }

  mmap_read_unlock(mm);
  return 0;
}

static int writeval_store(unsigned long addr, unsigned long val) {
  struct task_struct *task = current;
  struct mm_struct *mm = get_task_mm(task);
  struct vm_area_struct *vma;
  unsigned long *ptr;

  mmap_write_lock(mm);

  vma = find_vma(mm, addr);
  if (!vma) {
    pr_info("Address not found in any VMA\n");
  } else {
    ptr = (unsigned long *)(addr);
    *ptr = val;
    pr_info("Value at address 0x%lx set to %lu\n", addr, val);
  }

  mmap_write_unlock(mm);
  return 0;
}

static int mmaneg_open(struct inode *inode, struct file *file) {
  return 0;
//  return single_open(file, mmaneg_show, NULL);
}

static ssize_t mmaneg_write(struct file *file, const char __user *buf, size_t len, loff_t *pos) {
  if (len > PROCFS_MAX_SIZE) {
    return -ENOMEM;
  }
  if (copy_from_user(mmaneg_buf, buf, len)) {
    return -EFAULT;
  }
  mmaneg_buf[len] = '\0';
  size_t len_mmaneg_buf = strlen(mmaneg_buf);
  if (len_mmaneg_buf < 7) {
    goto unrecognized_command;
  }

  if (strncmp(mmaneg_buf, "listvma", 7) == 0) {
    int res = listvma_show();
    return (res < 0) ? (ssize_t)res : (ssize_t)len;
  }
  if (len_mmaneg_buf == 7) {
    goto unrecognized_command;
  }
  if (strncmp(mmaneg_buf, "findpage", 8) == 0) {
    unsigned long addr = 0;
    if (sscanf(mmaneg_buf + 9, "%lu", &addr) != 1) {
      goto unrecognized_arguments;
    }
    int res = findpage_show(addr);
    return (res < 0) ? (ssize_t)res : (ssize_t)len;
  }
  if (strncmp(mmaneg_buf, "writeval", 8) == 0) {
    unsigned long addr = 0;
    unsigned long val = 0;
    if (sscanf(mmaneg_buf + 9, "%lu %lu", &addr, &val) != 2) {
      goto unrecognized_arguments;
    }
    int res = writeval_store(addr, val);
    return (res < 0) ? (ssize_t)res : (ssize_t)len;
  }

  unrecognized_command:
  pr_info("Invalid command: %s\n", mmaneg_buf);
  return -1;

  unrecognized_arguments:
  pr_info("Invalid arguments: %s\n", mmaneg_buf);
  return -2;
}

//------------------------------------------------------------------------------

// Module init/exit.

static int __init mmaneg_init(void) {
  pr_info("Inserting procfs_mmaneg_module...\n");

  proc_file = proc_create(PROCFS_NAME, 0644, NULL, &mmaneg_fops);
  if (!proc_file) {
    return -ENOMEM;
  }

  pr_info("procfs_mmaneg_module inserted successfully!\n");
  return 0;
}

static void __exit mmaneg_exit(void) {
  pr_info("Start removing procfs_mmaneg_module...\n");

  remove_proc_entry(PROCFS_NAME, NULL);

  pr_info("procfs_mmaneg_module removed successfully!\n");
}

module_init(mmaneg_init);
module_exit(mmaneg_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your name");
MODULE_DESCRIPTION("mmaneg module");
