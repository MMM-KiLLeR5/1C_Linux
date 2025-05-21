// Other.
#include <linux/slab.h> // kmalloc()

// Local.
#include "phone_book_api.h"
#include "phone_book_impl.h"

long get_pb_user(const char* surname, struct user_data_t* output_data) {
  if (output_data == NULL) {
    return -2;
  }

  user_data_impl* res = user_data_rbtree_search(&handbook.tree, surname);
  if (res == NULL) {
    return -1;
  }

  if (user_data_deep_copy(res->user, output_data, User) < 0) {
    return -3;
  }

  return 0;
}

long add_pb_user(struct user_data_t* input_data) {
  user_data_t* user = kernel_kmalloc(sizeof(user_data_t));
  if (user_data_deep_copy(input_data, user, Kernel) < 0) {
    goto deep_copy_failed;
  }

  user_data_impl* ud_impl = (user_data_impl*) kernel_kmalloc(sizeof(user_data_impl));
  if (user_data_deep_copy(input_data, user, Kernel) < 0) {
    goto user_data_impl_failed;
  }

  ud_impl->user = user;
  long res = user_data_rbtree_insert(&handbook.tree, ud_impl);
  if (res == -1) {
    goto insert_failed;
  }

  return res;

  insert_failed:
  kfree(ud_impl);

  user_data_impl_failed:
  user_data_deep_free(user, Kernel);

  deep_copy_failed:
  kfree(user);

  return -1;
}

long del_pb_user(const char* surname) {
  user_data_impl* res = user_data_rbtree_search(&handbook.tree, surname);
  if (res == NULL) {
    return -1;
  }
  rb_erase(&res->node, &handbook.tree);
  user_data_deep_free(res->user, Kernel);
  kfree(res->user);
  kfree(res);
  return 0;
}
