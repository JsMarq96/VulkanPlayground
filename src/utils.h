#pragma once

#include <spdlog/spdlog.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>

inline void assert_msg(bool test, const char* message) {
    if (!test) {
        spdlog::critical("Assertion failed: {}", message);
        assert(false);
    }
}

inline void assert_msg(bool test, std::string message) {
    if (!test) {
        spdlog::critical("Assertion failed: {}", message);
        assert(false);
    }
}

inline uint64_t bin_file_open(const char* file_dir, 
                              char** raw_file) {
    FILE *b_file = fopen(file_dir, "rb");

    assert_msg(b_file != nullptr, "Error opening bianry file");

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