#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

bool replace(uint32_t start, uint32_t size, const char* original_val, size_t original_val_sz, const char* new_val, size_t new_val_sz);

struct replacement {
    std::span<const uint8_t> orig;
    std::span<const uint8_t> repl;
};

void replaceBulk(uint32_t start, uint32_t size, std::span<const replacement> replacements);
