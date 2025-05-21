// Module registration.
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

// Others.
#include <linux/time.h>
#include <linux/interrupt.h>

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Egor Vashkevich");
MODULE_DESCRIPTION("Reports how much keys was entered");
MODULE_VERSION("0.01");

//------------------------------------------------------------------------------

// Timer variables.

static struct timer_list my_timer;
static atomic_t key_count = ATOMIC_INIT(0);
static const int TIMEOUT = 60000; // 60 secs == 1 min

static void schedule_timer(void) {
  mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMEOUT));
}

void my_timer_callback(struct timer_list* timer) {
  int number = atomic_fetch_and(0, &key_count);
  pr_info("Characters typed in the last minute: %d\n", number);
  schedule_timer();
}

irqreturn_t irq_handler(int irq, void* dev_id) {
  atomic_inc(&key_count);
  return IRQ_HANDLED;
}

// IRQ for PS/2 keyboard https://en.wikipedia.org/wiki/Interrupt_request#Master_PIC
static const int PS2_IRQ = 1;

static int __init key_counter_init(void) {
  pr_info("Initialising key_counter_module...\n");

  timer_setup(&my_timer, my_timer_callback, 0);
  schedule_timer();

  int err = request_irq(PS2_IRQ,
                        irq_handler,
                        IRQF_SHARED,
                        "Key counter",
                        (void*) irq_handler);
  if (err) {
    pr_err("Failed to register IRQ handler.\n"
           "Error code: %d\n.", err);
    return -EIO;
  }

  pr_info("key_counter_module initialized successfully!\n");
  return 0;
}

static void __exit key_counter_exit(void) {
  free_irq(PS2_IRQ, (void*) irq_handler);
  del_timer(&my_timer);
  printk(KERN_INFO "Key counter exited\n");
}

module_init(key_counter_init);
module_exit(key_counter_exit);
