#pragma once

//#include <malloc.h>
//#include <stddef.h>

#include <linux/slab.h>
#include <linux/rbtree.h>

//------------------------------------------------------------------------------

// Structures.

struct user_data_t {
  const char* name;
  const char* surname;
  size_t age;
  const char* phone;
  const char* email;
} typedef user_data_t;

struct user_data_impl {
  struct rb_node node;
  user_data_t* user;
} typedef user_data_impl;

struct handbook_t {
  struct rb_root tree;
} typedef handbook_t;

extern handbook_t handbook;

enum MallocType {
  User,
  Kernel,
} typedef MallocType;

//------------------------------------------------------------------------------

// Functions.

void init_handbook(void);

//------------------------------------------------------------------------------

// Malloc.
void* kernel_kmalloc(size_t size);
// 'out' must be allocated or be on stack (not NULL).
long user_data_copy_field(
    const char* user_field,
    const char** out_field,
    void* malloc_func(size_t)
);
long user_data_deep_copy_aux(
    const user_data_t* user,
    user_data_t* out,
    void* malloc_func(size_t),
    void free_func(const void*)
);
long user_data_deep_copy(
    const user_data_t* user,
    user_data_t* out,
    MallocType mtype
);

//------------------------------------------------------------------------------

// Free.

void user_data_deep_free_aux(user_data_t* user,
                             void free_func(const void*));
void user_data_deep_free(user_data_t* user, MallocType mtype);

//------------------------------------------------------------------------------

// Rbtree.

struct user_data_impl* user_data_rbtree_search(struct rb_root* root,
                                               const char* surname);

// 'data' must be kernel-allocated.
long user_data_rbtree_insert(struct rb_root* root, user_data_impl* data);
