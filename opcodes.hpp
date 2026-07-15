#pragma once

#include <cstdint>

namespace Opcode {
    constexpr uint8_t x(uint16_t opcode) noexcept {
        return (opcode & 0x0F00u) >> 8u;
    }
    
    constexpr uint8_t y(uint16_t opcode) noexcept {
        return (opcode & 0x00F0u) >> 4u;
    }

    //
    constexpr uint8_t n(uint16_t opcode) noexcept {
        return (opcode & 0x000Fu);
    }

    //byte
    constexpr uint8_t kk(uint16_t opcode) noexcept {
        return (opcode & 0x00FFu);
    }

    //address
    constexpr uint16_t nnn(uint16_t opcode) noexcept {
        return (opcode & 0x0FFFu);
    }

};
