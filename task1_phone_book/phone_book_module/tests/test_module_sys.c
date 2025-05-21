#include <stddef.h>
#include <unistd.h>

//457	common	get_pb_user		sys_get_pb_user
//458	common	add_pb_user		sys_add_pb_user
//459	common	del_pb_user		sys_del_pb_user

struct user_data_t {
  const char* name;
  const char* surname;
  size_t age;
  const char* phone;
  const char* email;
} typedef user_data_t;

int main() {

  user_data_t user = {
      .name = "Mike",
      .surname = "Abbot",
      .age = 18,
      .phone = "+7999887766",
      .email = "mike.abbot@gmail.com",
  };

//  sys_add_pb_user(&user);
  syscall(458, &user);

//  sys_get_pb_user("Abbot", user);
  syscall(457, "Abbot", user);

//  sys_del_pb_user("Abbot");
  syscall(459, "Abbot");
}
