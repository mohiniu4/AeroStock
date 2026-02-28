#pragma once

#include <string>

#include "Packet.h"

namespace AeroStock::Client
{
    class ClientFileManager final
    {
    public:
        ClientFileManager() = delete;

        [[nodiscard]] static std::string GetDefaultReceivedFileName();

        static void SavePayloadToFile(
            const AeroStock::Common::Packet::PayloadBuffer& payload,
            const std::string& outputFileName);
    };
}