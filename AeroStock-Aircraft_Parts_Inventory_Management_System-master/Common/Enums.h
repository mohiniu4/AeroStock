#pragma once

#include <cstdint>
#include <string_view>

namespace AeroStock::Common
{
    // Identifies the type of operation carried by a packet.
    enum class CommandType : std::uint8_t
    {
        None = 0,
        Connect = 1,
        Verify = 2,
        SearchByPartNumber = 3,
        SearchByPartName = 4,
        GetPartDetails = 5,
        UpdateStock = 6,
        RequestFile = 7,
        Disconnect = 8,
        Response = 9,
        Error = 10
    };

    // Identifies the overall result of a request or response.
    enum class StatusCode : std::uint8_t
    {
        None = 0,
        Success = 1,
        Failure = 2,
        InvalidRequest = 3,
        VerificationFailed = 4,
        NotFound = 5,
        TransmissionError = 6
    };

    // Represents the server communication state.
    enum class ServerState : std::uint8_t
    {
        Disconnected = 0,
        Connected = 1,
        Verified = 2,
        Processing = 3,
        SendingData = 4,
        Error = 5
    };

    // Provides more specific error detail when needed.
    enum class ErrorCode : std::uint8_t
    {
        None = 0,
        InvalidPacket = 1,
        InvalidCommand = 2,
        InvalidPayload = 3,
        VerificationRejected = 4,
        PartNotFound = 5,
        ChecksumMismatch = 6,
        FileTransferFailed = 7,
        InternalError = 8
    };

    // Returns true if the command value is one of the supported protocol commands.
    [[nodiscard]] constexpr bool IsValidCommandType(const CommandType command) noexcept
    {
        switch (command)
        {
        case CommandType::None:
        case CommandType::Connect:
        case CommandType::Verify:
        case CommandType::SearchByPartNumber:
        case CommandType::SearchByPartName:
        case CommandType::GetPartDetails:
        case CommandType::UpdateStock:
        case CommandType::RequestFile:
        case CommandType::Disconnect:
        case CommandType::Response:
        case CommandType::Error:
            return true;
        default:
            return false;
        }
    }

    // Returns true if the status value is supported by the protocol.
    [[nodiscard]] constexpr bool IsValidStatusCode(const StatusCode status) noexcept
    {
        switch (status)
        {
        case StatusCode::None:
        case StatusCode::Success:
        case StatusCode::Failure:
        case StatusCode::InvalidRequest:
        case StatusCode::VerificationFailed:
        case StatusCode::NotFound:
        case StatusCode::TransmissionError:
            return true;
        default:
            return false;
        }
    }

    // Returns true if the server state value is valid.
    [[nodiscard]] constexpr bool IsValidServerState(const ServerState state) noexcept
    {
        switch (state)
        {
        case ServerState::Disconnected:
        case ServerState::Connected:
        case ServerState::Verified:
        case ServerState::Processing:
        case ServerState::SendingData:
        case ServerState::Error:
            return true;
        default:
            return false;
        }
    }

    // Returns true if the error code value is valid.
    [[nodiscard]] constexpr bool IsValidErrorCode(const ErrorCode errorCode) noexcept
    {
        switch (errorCode)
        {
        case ErrorCode::None:
        case ErrorCode::InvalidPacket:
        case ErrorCode::InvalidCommand:
        case ErrorCode::InvalidPayload:
        case ErrorCode::VerificationRejected:
        case ErrorCode::PartNotFound:
        case ErrorCode::ChecksumMismatch:
        case ErrorCode::FileTransferFailed:
        case ErrorCode::InternalError:
            return true;
        default:
            return false;
        }
    }

    // Converts a command value to readable text.
    [[nodiscard]] constexpr std::string_view ToString(const CommandType command) noexcept
    {
        switch (command)
        {
        case CommandType::None: return "None";
        case CommandType::Connect: return "Connect";
        case CommandType::Verify: return "Verify";
        case CommandType::SearchByPartNumber: return "SearchByPartNumber";
        case CommandType::SearchByPartName: return "SearchByPartName";
        case CommandType::GetPartDetails: return "GetPartDetails";
        case CommandType::UpdateStock: return "UpdateStock";
        case CommandType::RequestFile: return "RequestFile";
        case CommandType::Disconnect: return "Disconnect";
        case CommandType::Response: return "Response";
        case CommandType::Error: return "Error";
        default: return "UnknownCommandType";
        }
    }

    // Converts a status value to readable text.
    [[nodiscard]] constexpr std::string_view ToString(const StatusCode status) noexcept
    {
        switch (status)
        {
        case StatusCode::None: return "None";
        case StatusCode::Success: return "Success";
        case StatusCode::Failure: return "Failure";
        case StatusCode::InvalidRequest: return "InvalidRequest";
        case StatusCode::VerificationFailed: return "VerificationFailed";
        case StatusCode::NotFound: return "NotFound";
        case StatusCode::TransmissionError: return "TransmissionError";
        default: return "UnknownStatusCode";
        }
    }

    // Converts a server state value to readable text.
    [[nodiscard]] constexpr std::string_view ToString(const ServerState state) noexcept
    {
        switch (state)
        {
        case ServerState::Disconnected: return "Disconnected";
        case ServerState::Connected: return "Connected";
        case ServerState::Verified: return "Verified";
        case ServerState::Processing: return "Processing";
        case ServerState::SendingData: return "SendingData";
        case ServerState::Error: return "Error";
        default: return "UnknownServerState";
        }
    }

    // Converts an error code to readable text.
    [[nodiscard]] constexpr std::string_view ToString(const ErrorCode errorCode) noexcept
    {
        switch (errorCode)
        {
        case ErrorCode::None: return "None";
        case ErrorCode::InvalidPacket: return "InvalidPacket";
        case ErrorCode::InvalidCommand: return "InvalidCommand";
        case ErrorCode::InvalidPayload: return "InvalidPayload";
        case ErrorCode::VerificationRejected: return "VerificationRejected";
        case ErrorCode::PartNotFound: return "PartNotFound";
        case ErrorCode::ChecksumMismatch: return "ChecksumMismatch";
        case ErrorCode::FileTransferFailed: return "FileTransferFailed";
        case ErrorCode::InternalError: return "InternalError";
        default: return "UnknownErrorCode";
        }
    }
}