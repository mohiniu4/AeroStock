#include "pch.h"
#include "RequestHandler.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "PayloadUtils.h"
#include "ProtocolConstants.h"

namespace AeroStock::Server
{
    namespace
    {
        [[nodiscard]] std::string SerializePartRecord(
            const AeroStock::Common::PartRecord& record)
        {
            return
                "PartNumber=" + record.partNumber +
                ";PartName=" + record.partName +
                ";Category=" + record.category +
                ";Location=" + record.storageLocation +
                ";Supplier=" + record.supplier +
                ";Stock=" + std::to_string(record.stockQuantity);
        }

        // joins multiple records with newline separators
        [[nodiscard]] std::string SerializePartRecordList(
            const std::vector<AeroStock::Common::PartRecord>& records)
        {
            std::string output;

            for (std::size_t index = 0; index < records.size(); ++index)
            {
                if (index > 0U)
                {
                    output += '\n';
                }

                output += SerializePartRecord(records[index]);
            }

            return output;
        }

        // parses "partNumber,quantity" format returns false on bad input
        [[nodiscard]] bool TryParseUpdateStockPayload(
            const std::string_view payloadText,
            std::string& outPartNumber,
            std::uint32_t& outQuantity)
        {
            const std::size_t delimiterPos = payloadText.find(',');
            if (delimiterPos == std::string_view::npos)
            {
                return false;
            }

            const std::string_view partNumberView = payloadText.substr(0U, delimiterPos);
            const std::string_view quantityView = payloadText.substr(delimiterPos + 1U);

            if (partNumberView.empty() || quantityView.empty())
            {
                return false;
            }

            try
            {
                outPartNumber = std::string(partNumberView);
                outQuantity = static_cast<std::uint32_t>(std::stoul(std::string(quantityView)));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        void EnsureLargeInventoryCatalogFileExists(const Inventory& inventory)
        {
            namespace fs = std::filesystem;

            const fs::path filePath{ AeroStock::Common::ProtocolConstants::DefaultInventoryFileName };
            constexpr std::uintmax_t MinimumRequiredBytes = 1024U * 1024U;
            constexpr std::uintmax_t TargetBytes = 1200U * 1024U;

            if (fs::exists(filePath))
            {
                std::error_code sizeError;
                const auto existingSize = fs::file_size(filePath, sizeError);

                if (!sizeError && existingSize >= MinimumRequiredBytes)
                {
                    return;
                }
            }

            std::ofstream outputFile(filePath, std::ios::binary | std::ios::trunc);
            if (!outputFile.is_open())
            {
                throw std::runtime_error("Failed to create inventory catalog file.");
            }

            const auto& records = inventory.GetAllRecords();
            if (records.empty())
            {
                throw std::runtime_error("Inventory is empty; cannot generate catalog file.");
            }

            std::size_t index = 0U;
            std::uintmax_t bytesWritten = 0U;

            while (bytesWritten < TargetBytes)
            {
                const auto& record = records[index % records.size()];

                const std::string line =
                    SerializePartRecord(record) +
                    ";ExportBatch=" + std::to_string(index) +
                    ";Notes=Aircraft inventory export record for synchronization and archival.\n";

                outputFile.write(line.data(), static_cast<std::streamsize>(line.size()));

                if (!outputFile.good())
                {
                    throw std::runtime_error("Failed while writing inventory catalog file.");
                }

                bytesWritten += static_cast<std::uintmax_t>(line.size());
                ++index;
            }
        }

        // reads file into payload buffer
        [[nodiscard]] AeroStock::Common::Packet::PayloadBuffer ReadFileAsPayload(
            const std::filesystem::path& filePath)
        {
            std::ifstream inputFile(filePath, std::ios::binary);
            if (!inputFile.is_open())
            {
                throw std::runtime_error("Failed to open requested file for transfer.");
            }

            inputFile.seekg(0, std::ios::end);
            const std::streamoff fileSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);

            if (fileSize <= 0)
            {
                throw std::runtime_error("Requested transfer file is empty.");
            }

            if (static_cast<std::uint64_t>(fileSize) >
                static_cast<std::uint64_t>(AeroStock::Common::ProtocolConstants::MaxPayloadSizeBytes))
            {
                throw std::runtime_error("Requested transfer file exceeds maximum packet payload size.");
            }

            AeroStock::Common::Packet::PayloadBuffer payload(
                static_cast<std::size_t>(fileSize));

            inputFile.read(
                reinterpret_cast<char*>(payload.data()),
                static_cast<std::streamsize>(payload.size()));

            if (!inputFile.good() && !inputFile.eof())
            {
                throw std::runtime_error("Failed while reading requested transfer file.");
            }

            return payload;
        }
    }

    // routes each command to its respective handler
    AeroStock::Common::Packet RequestHandler::HandleRequest(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine)
    {
        using AeroStock::Common::CommandType;
        using AeroStock::Common::ServerState;
        using AeroStock::Common::StatusCode;

        const CommandType commandType = requestPacket.GetCommandType();

        // Normal direct calls into RequestHandler still validate the command against
        // the current state machine state. When the session layer has already validated
        // the command and moved into Processing, skip this second permission check.
        if (stateMachine.GetCurrentState() != ServerState::Processing)
        {
            if (!stateMachine.CanProcessCommand(commandType))
            {
                stateMachine.MoveToErrorState();

                return CreateErrorResponse(
                    requestPacket.GetPacketId(),
                    StatusCode::InvalidRequest,
                    "REQUEST_NOT_ALLOWED_IN_CURRENT_STATE");
            }
        }

        switch (commandType)
        {
        case CommandType::Connect:
            return HandleConnectRequest(requestPacket, stateMachine);

        case CommandType::Verify:
            return HandleVerifyRequest(requestPacket, stateMachine);

        case CommandType::Disconnect:
            stateMachine.ApplySuccessfulCommand(CommandType::Disconnect);
            return CreateSuccessResponse(requestPacket.GetPacketId(), "DISCONNECTED");

        case CommandType::SearchByPartNumber:
            return HandleSearchByPartNumber(requestPacket, stateMachine);

        case CommandType::SearchByPartName:
            return HandleSearchByPartName(requestPacket, stateMachine);

        case CommandType::GetPartDetails:
            return HandleGetPartDetails(requestPacket, stateMachine);

        case CommandType::UpdateStock:
            return HandleUpdateStock(requestPacket, stateMachine);

        case CommandType::RequestFile:
            return HandleRequestFile(requestPacket, stateMachine);

        case CommandType::Response:
        case CommandType::Error:
        case CommandType::None:
        default:
            stateMachine.MoveToErrorState();

            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                StatusCode::InvalidRequest,
                AeroStock::Common::ProtocolConstants::InvalidRequestText);
        }
    }

    AeroStock::Common::Packet RequestHandler::HandleConnectRequest(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine) const
    {
        stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);
        return CreateSuccessResponse(requestPacket.GetPacketId(), "CONNECTED");
    }

    AeroStock::Common::Packet RequestHandler::HandleVerifyRequest(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine) const
    {
        const bool isExpectedVerificationPayload =
            AeroStock::Common::PayloadUtils::PayloadEqualsText(
                requestPacket.GetPayload(),
                AeroStock::Common::ProtocolConstants::VerificationRequestText);

        if (!isExpectedVerificationPayload)
        {
            stateMachine.MoveToErrorState();

            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::VerificationFailed,
                AeroStock::Common::ProtocolConstants::VerificationFailureText);
        }

        stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Verify);

        return CreateSuccessResponse(
            requestPacket.GetPacketId(),
            AeroStock::Common::ProtocolConstants::VerificationSuccessText);
    }

    // searches part number in inventory
    AeroStock::Common::Packet RequestHandler::HandleSearchByPartNumber(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine)
    {
        const std::string partNumber =
            AeroStock::Common::PayloadUtils::PayloadToString(requestPacket.GetPayload());

        if (partNumber.empty())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::InvalidRequest,
                "PART_NUMBER_REQUIRED");
        }

        const auto record = inventory_.FindByPartNumber(partNumber);
        if (!record.has_value())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::NotFound,
                "PART_NOT_FOUND");
        }

        stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::SearchByPartNumber);

        const std::string serializedRecord = SerializePartRecord(*record);
        return CreateSuccessResponse(requestPacket.GetPacketId(), serializedRecord);
    }

    // searched inventory and returns all matches
    AeroStock::Common::Packet RequestHandler::HandleSearchByPartName(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine)
    {
        const std::string partName =
            AeroStock::Common::PayloadUtils::PayloadToString(requestPacket.GetPayload());

        if (partName.empty())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::InvalidRequest,
                "PART_NAME_REQUIRED");
        }

        const auto results = inventory_.FindByPartName(partName);
        if (results.empty())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::NotFound,
                "PART_NAME_NOT_FOUND");
        }

        stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::SearchByPartName);

        const std::string serializedResults = SerializePartRecordList(results);
        return CreateSuccessResponse(requestPacket.GetPacketId(), serializedResults);
    }

    // returns requested part details 
    AeroStock::Common::Packet RequestHandler::HandleGetPartDetails(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine)
    {
        const std::string partNumber =
            AeroStock::Common::PayloadUtils::PayloadToString(requestPacket.GetPayload());

        if (partNumber.empty())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::InvalidRequest,
                "PART_NUMBER_REQUIRED");
        }

        const auto record = inventory_.GetPartDetails(partNumber);
        if (!record.has_value())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::NotFound,
                "PART_DETAILS_NOT_FOUND");
        }

        stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::GetPartDetails);

        const std::string serializedRecord = SerializePartRecord(*record);
        return CreateSuccessResponse(requestPacket.GetPacketId(), serializedRecord);
    }

    // gets partNumber & quantity from payload and updates inventory
    AeroStock::Common::Packet RequestHandler::HandleUpdateStock(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine)
    {
        const std::string payloadText =
            AeroStock::Common::PayloadUtils::PayloadToString(requestPacket.GetPayload());

        if (payloadText.empty())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::InvalidRequest,
                "UPDATE_STOCK_PAYLOAD_REQUIRED");
        }

        std::string partNumber;
        std::uint32_t newQuantity = 0U;

        if (!TryParseUpdateStockPayload(payloadText, partNumber, newQuantity))
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::InvalidRequest,
                "INVALID_UPDATE_STOCK_FORMAT");
        }

        // Validate the quantity explicitly so the response can distinguish between
        // bad input and a genuinely missing part.
        if (newQuantity > AeroStock::Common::PartRecord::MaxSupportedStockQuantity)
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::InvalidRequest,
                "INVALID_STOCK_QUANTITY");
        }

        // Check for existence before update so the failure reason is accurate.
        const auto existingRecord = inventory_.GetPartDetails(partNumber);
        if (!existingRecord.has_value())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::NotFound,
                "PART_NOT_FOUND");
        }

        const bool updateSucceeded = inventory_.UpdateStock(partNumber, newQuantity);
        if (!updateSucceeded)
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::Failure,
                "UPDATE_STOCK_FAILED");
        }

        const auto updatedRecord = inventory_.GetPartDetails(partNumber);
        if (!updatedRecord.has_value())
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::Failure,
                "UPDATED_RECORD_NOT_FOUND");
        }

        stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::UpdateStock);

        const std::string serializedRecord = SerializePartRecord(*updatedRecord);
        return CreateSuccessResponse(requestPacket.GetPacketId(), serializedRecord);
    }

    AeroStock::Common::Packet RequestHandler::HandleRequestFile(
        const AeroStock::Common::Packet& requestPacket,
        ServerStateMachine& stateMachine)
    {
        try
        {
            EnsureLargeInventoryCatalogFileExists(inventory_);

            const std::filesystem::path filePath{
                AeroStock::Common::ProtocolConstants::DefaultInventoryFileName
            };

            const auto filePayload = ReadFileAsPayload(filePath);

            AeroStock::Common::Packet responsePacket(
                requestPacket.GetPacketId(),
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            responsePacket.SetPayload(filePayload);

            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::RequestFile);
            return responsePacket;
        }
        catch (const std::exception&)
        {
            return CreateErrorResponse(
                requestPacket.GetPacketId(),
                AeroStock::Common::StatusCode::Failure,
                "FILE_TRANSFER_FAILED");
        }
    }

    AeroStock::Common::Packet RequestHandler::CreateSuccessResponse(
        const std::uint32_t packetId,
        const std::string_view responseText) const
    {
        AeroStock::Common::Packet responsePacket(
            packetId,
            AeroStock::Common::CommandType::Response,
            AeroStock::Common::StatusCode::Success);

        responsePacket.SetPayload(
            AeroStock::Common::PayloadUtils::StringToPayload(responseText));

        return responsePacket;
    }

    AeroStock::Common::Packet RequestHandler::CreateErrorResponse(
        const std::uint32_t packetId,
        const AeroStock::Common::StatusCode status,
        const std::string_view errorText) const
    {
        AeroStock::Common::Packet responsePacket(
            packetId,
            AeroStock::Common::CommandType::Error,
            status);

        responsePacket.SetPayload(
            AeroStock::Common::PayloadUtils::StringToPayload(errorText));

        return responsePacket;
    }
}