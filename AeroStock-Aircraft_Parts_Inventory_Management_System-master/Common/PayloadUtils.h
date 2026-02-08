#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Packet.h"

namespace AeroStock::Common
{
    // Provides simple helpers for converting between text and payload bytes.
    class PayloadUtils final
    {
    public:
        PayloadUtils() = delete;

        // Converts text into a payload buffer.
        [[nodiscard]] static Packet::PayloadBuffer StringToPayload(std::string_view text);

        // Converts a payload buffer into a string.
        [[nodiscard]] static std::string PayloadToString(const Packet::PayloadBuffer& payload);

        // Returns true if the payload bytes exactly match the expected text.
        [[nodiscard]] static bool PayloadEqualsText(
            const Packet::PayloadBuffer& payload,
            std::string_view expectedText);
    };
}