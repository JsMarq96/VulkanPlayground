#pragma once

#include <cstdint>

/**
* First implementation of an arena memory allocator
* The idea is O(1) indexing, O(1) insertions and O(1) deletions. Depending on the arena sizes cache coherency might or might not suffer
* This data strucutre is not thought for secuental access, or processing
* Arenas are not cleared up on empty TODO: should be setted via parameter on teh template?
* Max arenas 12 bits -> 4096 total arenas & 16 bits -> 65536 elementes max per arena -> total max 268435456 elements max
*  4 bits per prefixes
 */

// Type of the arena, # of elements per arena
template<typename T, uint16_t N>
struct sArenaList {
    struct sSmallArena {
        T elements[N];
    };

    uint32_t empty_index_top = 0u;
    uint32_t empty_index_buffer[N * 2u];

    uint16_t arena_count = 0u;
    sSmallArena **arena_lists = nullptr;

    void init() {
        arena_count = 1u;
        arena_lists = (sSmallArena**) malloc(sizeof(sSmallArena*));

        arena_lists[arena_count-1u] = (sSmallArena*) malloc(sizeof(sSmallArena));
    }

    inline bool add_arena() {
        arena_count++;
        arena_lists = (sSmallArena**) realloc(arena_lists, arena_count * sizeof(sSmallArena*));

        const uint32_t arena_idx = (arena_count-1u) << 16u;
        arena_lists[arena_count-1u] = (sSmallArena*) malloc(sizeof(sSmallArena));

        for(uint16_t i = 0u; i < N; i++) {
            empty_index_buffer[empty_index_top++] = arena_idx | i;
        }

        return true; // TODO asserts
    }

    inline T& get(const uint32_t idx) const {
        const uint16_t arena_idx = 0x0FFFu & (idx << 16u);
        const uint16_t in_arena_idx = (idx & 0xFFFFu);

        if (arena_count < arena_idx) {
            // Error!
        }

        return arena_lists[arena_idx]->elements[in_arena_idx];
    }

    inline uint32_t store(T &new_element) {
        if (empty_index_top == 0u)  {
            if (add_arena()) {
                // Error
            }
        }

        const uint32_t new_elem_idx = empty_index_buffer[empty_index_top];
        const uint16_t arena_idx = 0x0FFFu & (new_elem_idx << 16u);
        const uint16_t in_arena_idx = (new_elem_idx & 0xFFFFu);

        arena_lists[arena_idx]->elements[in_arena_idx] = new_element;

        return new_elem_idx;
    }

    inline void remove(const uint32_t idx) {
        empty_index_buffer[empty_index_top++] = idx;
    }
};