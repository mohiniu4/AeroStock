#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace AeroStock::Common
{
    // Provides checksum helpers used to protect packet data integrity.
    class Checksum final
    {
    public:
        Checksum() = delete;

        // Calculates a checksum from a raw byte buffer.
        [[nodiscard]] static std::uint32_t Calculate(
            const std::uint8_t* data,
            std::size_t length) noexcept;

        // Calculates a checksum from a byte vector.
        [[nodiscard]] static std::uint32_t Calculate(
            const std::vector<std::uint8_t>& data) noexcept;

        // Compares a raw byte buffer against an expected checksum.
        [[nodiscard]] static bool Validate(
            const std::uint8_t* data,
            std::size_t length,
            std::uint32_t expectedChecksum) noexcept;

        // Compares a byte vector against an expected checksum.
        [[nodiscard]] static bool Validate(
            const std::vector<std::uint8_t>& data,
            std::uint32_t expectedChecksum) noexcept;

    private:
        // Starting value used for checksum calculation.
        static inline constexpr std::uint32_t InitialSeed = 2166136261U;

        // Prime used to mix each byte into the checksum.
        static inline constexpr std::uint32_t MixingPrime = 16777619U;
    };
}