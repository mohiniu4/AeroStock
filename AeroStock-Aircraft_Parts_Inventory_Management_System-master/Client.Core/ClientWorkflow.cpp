#include "pch.h"
#include "ClientWorkflow.h"

#include <sstream>
#include <vector>

#include "PayloadUtils.h"
#include "ProtocolConstants.h"

namespace AeroStock::Client
{
    namespace
    {
		// Utility function to split a string by a specified delimiter and return the parts as a vector.
        [[nodiscard]] std::vector<std::string> SplitString(
            const std::string& text,
            const char delimiter)
        {
            std::vector<std::string> parts;
            std::stringstream stream(text);
            std::string item;

            while (std::getline(stream, item, delimiter))
            {
                parts.push_back(item);
            }

            return parts;
        }
    }

	// The ClientWorkflow class provides static methods to create various types of packets for client-server communication,
    AeroStock::Common::Packet ClientWorkflow::CreateConnectPacket(const std::uint32_t packetId)
    {
        return AeroStock::Common::Packet(
            packetId,
            AeroStock::Common::CommandType::Connect,
            AeroStock::Common::StatusCode::None);
    }

	// Each method constructs a packet with the appropriate command type and payload based on the intended client action,
    // such as verifying the connection,
    AeroStock::Common::Packet ClientWorkflow::CreateVerifyPacket(const std::uint32_t packetId)
    {
        AeroStock::Common::Packet packet(
            packetId,
            AeroStock::Common::CommandType::Verify,
            AeroStock::Common::StatusCode::None);

        packet.SetPayload(
            AeroStock::Common::PayloadUtils::StringToPayload(
                AeroStock::Common::ProtocolConstants::VerificationRequestText));

        return packet;
    }

	// searching for parts by number or name, retrieving part details,
    // updating stock quantities, requesting files, and disconnecting.
    AeroStock::Common::Packet ClientWorkflow::CreateSearchByPartNumberPacket(
        const std::uint32_t packetId,
        const std::string& partNumber)
    {
        AeroStock::Common::Packet packet(
            packetId,
            AeroStock::Common::CommandType::SearchByPartNumber,
            AeroStock::Common::StatusCode::None);

        packet.SetPayload(
            AeroStock::Common::PayloadUtils::StringToPayload(partNumber));

        return packet;
    }

	// The CreateSearchByPartNamePacket method constructs a packet for searching parts by name,
    // with the part name included in the payload.
    AeroStock::Common::Packet ClientWorkflow::CreateSearchByPartNamePacket(
        const std::uint32_t packetId,
        const std::string& partName)
    {
        AeroStock::Common::Packet packet(
            packetId,
            AeroStock::Common::CommandType::SearchByPartName,
            AeroStock::Common::StatusCode::None);

        packet.SetPayload(
            AeroStock::Common::PayloadUtils::StringToPayload(partName));

        return packet;
    }

	// The CreateGetPartDetailsPacket method constructs a packet for retrieving detailed information about a specific part,
    AeroStock::Common::Packet ClientWorkflow::CreateGetPartDetailsPacket(
        const std::uint32_t packetId,
        const std::string& partNumber)
    {
        AeroStock::Common::Packet packet(
            packetId,
            AeroStock::Common::CommandType::GetPartDetails,
            AeroStock::Common::StatusCode::None);

        packet.SetPayload(
            AeroStock::Common::PayloadUtils::StringToPayload(partNumber));

        return packet;
    }

	// The CreateUpdateStockPacket method constructs a packet for updating the stock quantity of a specific part
    AeroStock::Common::Packet ClientWorkflow::CreateUpdateStockPacket(
        const std::uint32_t packetId,
        const std::string& partNumber,
        const std::uint32_t newQuantity)
    {
        AeroStock::Common::Packet packet(
            packetId,
            AeroStock::Common::CommandType::UpdateStock,
            AeroStock::Common::StatusCode::None);

        const std::string payloadText =
            partNumber + "," + std::to_string(newQuantity);

        packet.SetPayload(
            AeroStock::Common::PayloadUtils::StringToPayload(payloadText));

        return packet;
    }

	// The CreateRequestFilePacket method constructs a packet for requesting a file from the server, with no payload needed.
    AeroStock::Common::Packet ClientWorkflow::CreateRequestFilePacket(const std::uint32_t packetId)
    {
        return AeroStock::Common::Packet(
            packetId,
            AeroStock::Common::CommandType::RequestFile,
            AeroStock::Common::StatusCode::None);
    }

	// The CreateDisconnectPacket method constructs a packet for disconnecting from the server, with no payload needed.
    AeroStock::Common::Packet ClientWorkflow::CreateDisconnectPacket(const std::uint32_t packetId)
    {
        return AeroStock::Common::Packet(
            packetId,
            AeroStock::Common::CommandType::Disconnect,
            AeroStock::Common::StatusCode::None);
    }

	// The IsSuccessfulResponse method checks if a given response packet indicates a successful operation
    // by verifying that the command type is Response and the status code is Success.
    bool ClientWorkflow::IsSuccessfulResponse(const AeroStock::Common::Packet& responsePacket)
    {
        return
            responsePacket.GetCommandType() == AeroStock::Common::CommandType::Response &&
            responsePacket.GetStatus() == AeroStock::Common::StatusCode::Success;
    }

	// The IsErrorResponse method checks if a given response packet indicates an error by
    // verifying that the command type is Error.
    bool ClientWorkflow::IsErrorResponse(const AeroStock::Common::Packet& responsePacket)
    {
        return responsePacket.GetCommandType() == AeroStock::Common::CommandType::Error;
    }

	// The GetPayloadText method extracts the payload from a packet and converts it to a string for easier processing and display.
    std::string ClientWorkflow::GetPayloadText(const AeroStock::Common::Packet& packet)
    {
        if (!packet.HasPayload())
        {
            return {};
        }

        return AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());
    }

	// The FormatResponseForDisplay method takes a response packet and formats its payload text for user-friendly display.
    std::string ClientWorkflow::FormatResponseForDisplay(const AeroStock::Common::Packet& packet)
    {
        const std::string payloadText = GetPayloadText(packet);

        if (payloadText.empty())
        {
            return "No payload returned.";
        }

        if (payloadText.find("PartNumber=") == std::string::npos)
        {
            return payloadText;
        }

        if (payloadText.find('\n') != std::string::npos)
        {
            return FormatMultiRecordResponseForDisplay(payloadText);
        }

        return FormatSingleRecordForDisplay(payloadText);
    }

	// The FormatSingleRecordForDisplay method takes a single record's text (with fields separated by semicolons)
    std::string ClientWorkflow::FormatSingleRecordForDisplay(const std::string& recordText)
    {
        const auto fields = SplitString(recordText, ';');

        std::ostringstream stream;
        for (const auto& field : fields)
        {
            const auto equalsPos = field.find('=');
            if (equalsPos == std::string::npos)
            {
                continue;
            }

            const std::string key = field.substr(0, equalsPos);
            const std::string value = field.substr(equalsPos + 1);

            if (key == "PartNumber")
            {
                stream << "Part Number : " << value << '\n';
            }
            else if (key == "PartName")
            {
                stream << "Part Name   : " << value << '\n';
            }
            else if (key == "Category")
            {
                stream << "Category    : " << value << '\n';
            }
            else if (key == "Location")
            {
                stream << "Location    : " << value << '\n';
            }
            else if (key == "Supplier")
            {
                stream << "Supplier    : " << value << '\n';
            }
            else if (key == "Stock")
            {
                stream << "Stock       : " << value << '\n';
            }
        }

        return stream.str();
    }

	// The FormatMultiRecordResponseForDisplay method takes a payload text containing multiple records
    // (separated by newlines) and formats each record for display, separating them with lines and numbering the results.
    std::string ClientWorkflow::FormatMultiRecordResponseForDisplay(const std::string& payloadText)
    {
        const auto records = SplitString(payloadText, '\n');

        std::ostringstream stream;
        for (std::size_t index = 0; index < records.size(); ++index)
        {
            stream << "----------------------------------------\n";
            stream << "Result " << (index + 1U) << '\n';
            stream << "----------------------------------------\n";
            stream << FormatSingleRecordForDisplay(records[index]);

            if (index + 1U < records.size())
            {
                stream << '\n';
            }
        }

        return stream.str();
    }
}