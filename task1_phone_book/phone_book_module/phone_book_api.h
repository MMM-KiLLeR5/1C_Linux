#pragma once

#include "phone_book_impl.h"

long get_pb_user(const char* surname, struct user_data_t* output_data);
long add_pb_user(struct user_data_t* input_data);
long del_pb_user(const char* surname);
