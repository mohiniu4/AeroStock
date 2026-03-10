#include "pch.h"
#include "CppUnitTest.h"

#include "../Common/PacketHeader.h"
#include "../Common/ProtocolConstants.h"
#include "../Common/Enums.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(PacketHeaderTests)
    {
    public:
        // Verifies that a default-constructed header is not considered valid.
        TEST_METHOD(DefaultConstructedHeader_IsInvalid)
        {
            AeroStock::Common::PacketHeader header{};

            Assert::IsFalse(header.IsValid());
        }

        // Verifies that a fully populated header with valid values passes validation.
        TEST_METHOD(ValidHeader_IsValid)
        {
            AeroStock::Common::PacketHeader header{};
            header.version = AeroStock::Common::ProtocolConstants::ProtocolVersion;
            header.packetId = 1U;
            header.commandType = AeroStock::Common::CommandType::Verify;
            header.status = AeroStock::Common::StatusCode::Success;
            header.payloadLength = 16U;
            header.checksum = 12345U;

            Assert::IsTrue(header.IsValid());
        }

        // Verifies that an invalid protocol version is rejected.
        TEST_METHOD(HasValidVersion_InvalidVersion_ReturnsFalse)
        {
            AeroStock::Common::PacketHeader header{};
            header.version =
                static_cast<std::uint8_t>(
                    AeroStock::Common::ProtocolConstants::ProtocolVersion + 1U);

            Assert::IsFalse(header.HasValidVersion());
        }

        // Verifies that the reserved invalid packet ID is rejected.
        TEST_METHOD(HasValidPacketId_InvalidPacketId_ReturnsFalse)
        {
            AeroStock::Common::PacketHeader header{};
            header.packetId = AeroStock::Common::ProtocolConstants::InvalidPacketId;

            Assert::IsFalse(header.HasValidPacketId());
        }

        // Verifies that a normal non-reserved packet ID is accepted.
        TEST_METHOD(HasValidPacketId_ValidPacketId_ReturnsTrue)
        {
            AeroStock::Common::PacketHeader header{};
            header.packetId = 100U;

            Assert::IsTrue(header.HasValidPacketId());
        }

        // Verifies that an out-of-range command type is rejected.
        TEST_METHOD(HasValidCommandType_InvalidCommand_ReturnsFalse)
        {
            AeroStock::Common::PacketHeader header{};
            header.commandType =
                static_cast<AeroStock::Common::CommandType>(255);

            Assert::IsFalse(header.HasValidCommandType());
        }

        // Verifies that an out-of-range status code is rejected.
        TEST_METHOD(HasValidStatusCode_InvalidStatus_ReturnsFalse)
        {
            AeroStock::Common::PacketHeader header{};
            header.status =
                static_cast<AeroStock::Common::StatusCode>(255);

            Assert::IsFalse(header.HasValidStatusCode());
        }

        // Verifies that a payload length within the supported maximum is accepted.
        TEST_METHOD(HasValidPayloadLength_WithinMaximum_ReturnsTrue)
        {
            AeroStock::Common::PacketHeader header{};
            header.payloadLength =
                AeroStock::Common::ProtocolConstants::MaxPayloadSizeBytes;

            Assert::IsTrue(header.HasValidPayloadLength());
        }

        // Verifies that a payload length exceeding the supported maximum is rejected.
        TEST_METHOD(HasValidPayloadLength_ExceedsMaximum_ReturnsFalse)
        {
            AeroStock::Common::PacketHeader header{};
            header.payloadLength =
                AeroStock::Common::ProtocolConstants::MaxPayloadSizeBytes + 1U;

            Assert::IsFalse(header.HasValidPayloadLength());
        }

        // Verifies that a header with excessive payload length fails overall validation.
        TEST_METHOD(IsValid_ExcessivePayloadLength_ReturnsFalse)
        {
            AeroStock::Common::PacketHeader header{};
            header.version = AeroStock::Common::ProtocolConstants::ProtocolVersion;
            header.packetId = 1U;
            header.commandType = AeroStock::Common::CommandType::Response;
            header.status = AeroStock::Common::StatusCode::Success;
            header.payloadLength =
                AeroStock::Common::ProtocolConstants::MaxPayloadSizeBytes + 1U;
            header.checksum = 12345U;

            Assert::IsFalse(header.IsValid());
        }

        // Verifies that a checksum value of zero does not by itself invalidate
        // an otherwise valid header.
        TEST_METHOD(IsValid_ZeroChecksum_DoesNotInvalidateHeader)
        {
            AeroStock::Common::PacketHeader header{};
            header.version = AeroStock::Common::ProtocolConstants::ProtocolVersion;
            header.packetId = 1U;
            header.commandType = AeroStock::Common::CommandType::Response;
            header.status = AeroStock::Common::StatusCode::Success;
            header.payloadLength = 0U;
            header.checksum = 0U;

            Assert::IsTrue(header.IsValid());
        }
    };
}