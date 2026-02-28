#include "pch.h"
#include "ClientFileManager.h"

#include <fstream>
#include <stdexcept>

namespace AeroStock::Client
{
	// Returns the default file name to use when saving received inventory catalog files.
    std::string ClientFileManager::GetDefaultReceivedFileName()
    {
        return "received_inventory_catalog.dat";
    }

	// Saves the given payload buffer to a file with the specified name. Throws if the payload is empty or if file writing fails.
    void ClientFileManager::SavePayloadToFile(
        const AeroStock::Common::Packet::PayloadBuffer& payload,
        const std::string& outputFileName)
    {
        if (payload.empty())
        {
            throw std::runtime_error("Cannot save file: payload is empty.");
        }

        std::ofstream outputFile(outputFileName, std::ios::binary | std::ios::trunc);
        if (!outputFile.is_open())
        {
            throw std::runtime_error("Failed to open output file for writing.");
        }

        outputFile.write(
            reinterpret_cast<const char*>(payload.data()),
            static_cast<std::streamsize>(payload.size()));

        if (!outputFile.good())
        {
            throw std::runtime_error("Failed while writing received file payload.");
        }
    }
}