#include "phone_book_impl.h"

handbook_t handbook = {
    .tree = RB_ROOT,
};

void init_handbook(void) {
  handbook.tree = RB_ROOT;
}

//------------------------------------------------------------------------------

// Malloc.

void* kernel_kmalloc(size_t size) {
  return kmalloc(size, GFP_KERNEL);
}

long user_data_copy_field(
    const char* user_field,
    const char** out_field,
    void* malloc_func(size_t)
) {
  if (out_field == NULL) {
    return -2;
  }

  size_t len = strlen(user_field);
  char* buf = malloc_func(len + 1);
  if (buf == NULL) {
    return -1; // bad alloc
  }

  memcpy(buf, user_field, len);
  buf[len] = '\0';
  *out_field = buf;
  return 0;
}

long user_data_deep_copy_aux(
    const user_data_t* user,
    user_data_t* out,
    void* malloc_func(size_t),
    void free_func(const void*)
) {
  if (out == NULL) {
    return -2;
  }

  if (user_data_copy_field(user->name, &out->name, malloc_func) < 0) {
    goto bad_alloc_name;
  }
  if (user_data_copy_field(user->surname, &out->surname, malloc_func) < 0) {
    goto bad_alloc_surname;
  }
  out->age = user->age;
  if (user_data_copy_field(user->phone, &out->phone, malloc_func) < 0) {
    goto bad_alloc_phone;
  }
  if (user_data_copy_field(user->email, &out->email, malloc_func) < 0) {
    goto bad_alloc_email;
  }

  return 0;

  bad_alloc_email:
  free_func(out->phone);

  bad_alloc_phone:
  free_func(out->surname);

  bad_alloc_surname:
  free_func(out->name);

  bad_alloc_name:
  free_func(out);

  return -1;
}

long user_data_deep_copy(
    const user_data_t* user,
    user_data_t* out,
    MallocType mtype
) {
//  if (mtype == User) {
//    return user_data_deep_copy_aux(user, out, malloc, (void (*)(const void*)) free);
//  }
  return user_data_deep_copy_aux(user, out, kernel_kmalloc, kfree);
}

//------------------------------------------------------------------------------

// Free.

void user_data_deep_free_aux(user_data_t* user,
                             void free_func(const void*)) {
  free_func(user->name);
  free_func(user->surname);
  free_func(user->phone);
  free_func(user->email);
}

void user_data_deep_free(user_data_t* user, MallocType mtype) {
//  if (mtype == User) {
//    user_data_deep_free_aux(user, (void (*)(const void*)) free);
//  }
  user_data_deep_free_aux(user, kfree);
}

struct user_data_impl* user_data_rbtree_search(struct rb_root* root,
                                               const char* surname) {
  struct rb_node* node = root->rb_node;

  while (node) {
    struct user_data_impl* data = container_of(node, user_data_impl, node);
    int result;

    result = strcmp(surname, data->user->surname);

    if (result < 0)
      node = node->rb_left;
    else if (result > 0)
      node = node->rb_right;
    else
      return data;
  }
  return NULL;
}

// data must be allocated on heap.
long user_data_rbtree_insert(struct rb_root* root, user_data_impl* data) {
  struct rb_node** new = &(root->rb_node), * parent = NULL;

  /* Figure out where to put new node */
  while (*new) {
    user_data_impl* cur = container_of(*new, user_data_impl, node);
    int result = strcmp(data->user->surname, cur->user->surname);

    parent = *new;
    if (result < 0)
      new = &((*new)->rb_left);
    else if (result > 0)
      new = &((*new)->rb_right);
    else
      return -1;
  }

  /* Add new node and rebalance tree. */
  rb_link_node(&data->node, parent, new);
  rb_insert_color(&data->node, root);

  return 0;
}
