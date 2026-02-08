#include "pch.h"
#include "PayloadUtils.h"
#include <algorithm>

namespace AeroStock::Common
{
    Packet::PayloadBuffer PayloadUtils::StringToPayload(const std::string_view text)
    {
        // Copy the input text directly into a payload buffer.
        return Packet::PayloadBuffer(text.begin(), text.end());
    }

    std::string PayloadUtils::PayloadToString(const Packet::PayloadBuffer& payload)
    {
        // Rebuild a string from the payload bytes.
        return std::string(payload.begin(), payload.end());
    }

    bool PayloadUtils::PayloadEqualsText(
        const Packet::PayloadBuffer& payload,
        const std::string_view expectedText)
    {
        // Mismatched sizes cannot be equal.
        if (payload.size() != expectedText.size())
        {
            return false;
        }

        // Compare each byte against the expected text.
        return std::equal(payload.begin(), payload.end(), expectedText.begin());
    }
}