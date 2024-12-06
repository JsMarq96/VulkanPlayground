#pragma once

#include <cstdint>

/**
 * First implementation of a FILO Stack
 * the storage is maanged in differnt arenas, to avoid multiple allocs and deallocs
 * in a single frame.
 * Kinda naive, only dealloc stacks on a diffence of more than 1 from the current pointer
 */

template<typename T, uint16_t ARENA_SIZE>
struct sStack {
    struct sTinyStack {
        T data[ARENA_SIZE];
    };

    uint16_t stack_count = 0u;
    sTinyStack **stack_lists = nullptr;

    uint32_t top_pointer = 0u;
    uint16_t curr_stack_pointer = 0u;

    uint64_t element_count = 0u;

    void init() {
        stack_count = 1u;
        stack_lists = (sTinyStack**) malloc(sizeof(sTinyStack*));

        stack_lists[0u] = (sTinyStack*) malloc(sizeof(sTinyStack));
    }

    void clean() {
        for(uint32_t i = 0u; i < stack_count; i++) {
            free(stack_lists[i]);
        }

        free(stack_lists);

        element_count = 0u;
    };

    inline uint32_t size() const {
        return curr_stack_pointer * ARENA_SIZE + top_pointer;
    }

    inline void push(const T& to_add) {
        if (ARENA_SIZE >= top_pointer) {
            // Add new tiny stack
            stack_count++;
            curr_stack_pointer = stack_count;

            stack_lists = (sTinyStack**) realloc(stack_lists, stack_count * sizeof(sTinyStack*));

            const uint32_t arena_idx = (stack_count-1u) << 16u;
            stack_lists[stack_count-1u] = (sTinyStack*) malloc(sizeof(sTinyStack));
        }

        const uint32_t id_to_push = top_pointer++;

        element_count++;

        stack_lists[curr_stack_pointer][id_to_push] = to_add;
    }

    inline T& peek() {
        return stack_lists[curr_stack_pointer][top_pointer];
    };

    inline T pop() {
        T& curr_top = stack_lists[curr_stack_pointer][top_pointer];

        if (top_pointer == 0u) {
            if (curr_stack_pointer > 0) {
                top_pointer = ARENA_SIZE;
                curr_stack_pointer--;

                // Only keep one mini stack as a margin
                // if the difference is 2 or more, remove one
                if (stack_count - curr_stack_pointer > 2u) {
                    free(stack_lists[stack_count]);
                    stack_count--;
                    stack_lists = (sTinyStack**) realloc(stack_lists, stack_count * sizeof(sTinyStack*));
                }
            }
        } else {
            top_pointer--;
        }

        element_count--;

        return curr_top;
    }
};