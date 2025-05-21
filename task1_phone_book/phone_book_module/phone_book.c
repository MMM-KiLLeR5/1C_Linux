#include <linux/kernel.h>
#include <linux/syscalls.h>

#include "phone_book_api.h"

/**
 * @invariants
 * * @b surname must be null-terminated string \n
 * * @b output_data != NULL \n
 * * inner fields of @b output_data must be deallocated manually by user.
 *    User may use user_data_deep_free() from phone_book_impl() \n
 *
 * @param surname
 * @param output_data
 *
 * @return  0 - success \n
 *         -1 - user is not found \n
 *         -2 - incorrect output_data \n
 *         -3 - bad alloc \n
 */
//asmlinkage long sys_get_pb_user(
//    const char __user* surname,
//    user_data_t __user* output_data
//) {
//  return get_pb_user(surname, output_data);
//}

SYSCALL_DEFINE2(
    get_pb_user,
    const char __user *,
    surname,
    user_data_t __user *,
    output_data
) {
return get_pb_user(surname, output_data);
}

/**
 *
 * @param input_data
 *
 * @return  0 - success \n
 *         -1 - user already exists \n
 */
//asmlinkage long sys_add_pb_user(user_data_t __user* input_data) {
//  return add_pb_user(input_data);
//}

SYSCALL_DEFINE1(
    add_pb_user,
    user_data_t __user*,
    input_data,
){
return add_pb_user(input_data);
}

/**
 * @invariants
 * @b surname must be null-terminated string \n
 * @b user is deallocated \n
 *
 * @param surname
 *
 * @return  0 - success \n
 *         -1 - user is not found \n
 */
//asmlinkage long sys_del_pb_user(const char __user* surname) {
//  return del_pb_user(surname);
//}

SYSCALL_DEFINE1(
    del_pb_user,
    const char __user*,
    surname,
) {
return del_pb_user(surname);
}
