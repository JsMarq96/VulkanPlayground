#include "utils.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <spdlog/spdlog.h>

void assert_msg(bool test, const char* message) {
    if (!test) {
        spdlog::critical("Assertion failed: {}", message);
        assert(false);
    }
}

void assert_msg(bool test, std::string message) {
    if (!test) {
        spdlog::critical("Assertion failed: {}", message);
        assert(false);
    }
}

uint64_t bin_file_open(const char* file_dir, 
                              char** raw_file) {
    FILE *b_file = fopen(file_dir, "rb");

    assert_msg(b_file != nullptr, "Error opening binary file");

    fseek(b_file, 0, SEEK_END);
    uint64_t file_size = ftell(b_file);
    fseek(b_file, 0, SEEK_SET);

    char* raw_data = (char*) malloc(file_size + 1u);
    fread(raw_data, file_size, 1, b_file);

    fclose(b_file);

    raw_data[file_size] = 0u;

    *(raw_file) = raw_data;

    return file_size;
}

uint8_t str_file_open(   const char* file_dir,
                                char** result_buffer    ) {
    const uint64_t bin_file_size = bin_file_open(file_dir, result_buffer);

    // TODO: should remove the null terminator from the size??
    return bin_file_size / sizeof(char);
}