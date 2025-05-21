obj-y += phone_book.o
phone_book-y += \
				phone_book_api.o \
				phone_book_impl.o \

obj-m += phone_book_module.o
phone_book_module-y += \
                 	phone_book_api.o \
                 	phone_book_impl.o \
                 	phone_book_module_impl.o \

# pass it for building (default is /)
ABS_PATH_TO_VROOT ?= "/"
KERNEL_VERSION ?= "$(shell uname -r)"

# "/lib/modules/$(shell uname -r)/build"
PATH_TO_MODULE_BUILD=$(ABS_PATH_TO_VROOT)/lib/modules/$(KERNEL_VERSION)/build

all:
	make -C "$(PATH_TO_MODULE_BUILD)" M="$(PWD)" modules
clean:
	make -C "$(PATH_TO_MODULE_BUILD)" M="$(PWD)" clean
