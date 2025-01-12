#pragma once

#include <stdint.h>

void assert_msg(bool test, const char* message);

//void assert_msg(bool test, std::string message);

uint64_t bin_file_open(const char* file_dir, char** raw_file);

uint8_t str_file_open(   const char* file_dir, char** result_buffer    );