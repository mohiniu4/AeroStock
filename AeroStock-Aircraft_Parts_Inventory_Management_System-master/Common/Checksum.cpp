#include "pch.h"
#include "Checksum.h"

namespace AeroStock::Common
{
    std::uint32_t Checksum::Calculate(const std::uint8_t* const data, const std::size_t length) noexcept
    {
        // Return the seed unchanged for an empty buffer.
        if (length == 0U)
        {
            return InitialSeed;
        }

        // Null data is invalid when a non-zero length is provided.
        if (data == nullptr)
        {
            return 0U;
        }

        std::uint32_t checksum = InitialSeed;

        // Mix each byte into the checksum.
        for (std::size_t index = 0U; index < length; ++index)
        {
            const std::uint32_t currentByte = static_cast<std::uint32_t>(data[index]);
            checksum ^= currentByte;
            checksum = static_cast<std::uint32_t>(checksum * MixingPrime);
        }

        return checksum;
    }

    std::uint32_t Checksum::Calculate(const std::vector<std::uint8_t>& data) noexcept
    {
        // Reuse the raw-buffer overload for vector input.
        return Calculate(data.data(), data.size());
    }

    bool Checksum::Validate(
        const std::uint8_t* const data,
        const std::size_t length,
        const std::uint32_t expectedChecksum) noexcept
    {
        // Calculate the actual checksum and compare it to the expected value.
        const std::uint32_t actualChecksum = Calculate(data, length);
        return actualChecksum == expectedChecksum;
    }

    bool Checksum::Validate(
        const std::vector<std::uint8_t>& data,
        const std::uint32_t expectedChecksum) noexcept
    {
        // Reuse the raw-buffer overload for vector input.
        return Validate(data.data(), data.size(), expectedChecksum);
    }
}