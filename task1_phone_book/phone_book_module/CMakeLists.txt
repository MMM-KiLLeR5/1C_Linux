cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project("Linux Kernel Module with CLion IDE support / CMake" VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find kernel headers
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(KernelHeaders REQUIRED)


# find MODULE_LICENSE("GPL"), MODULE_AUTHOR() etc.
# thanks to "merseyviking" from stack overflow
add_definitions(-D__KERNEL__ -DMODULE)

set(COMMON_SOURCES
        phone_book_impl.h
        phone_book_impl.c
        phone_book_api.h
        phone_book_api.c
)

# this is needed in order for CLion IDE to provide syntax highlightning
# this is independent from the actual kernel object that is built
add_executable(phone_handbook_module
        # add all *.h and *.c files here that # CLion should cover
        phone_book_module_impl.c
        ${COMMON_SOURCES}
)
add_executable(phone_handbook_syscall
        phone_book.c
        ${COMMON_SOURCES}
)

# CLion IDE will find symbols from <linux/*>
target_include_directories("phone_handbook_module" PRIVATE ${KERNELHEADERS_INCLUDE_DIRS})

target_include_directories("phone_handbook_syscall" PRIVATE ${KERNELHEADERS_INCLUDE_DIRS})
