#include "pch.h"
#include "CppUnitTest.h"

#include "../Common/PacketSerializer.h"
#include "../Common/PayloadUtils.h"
#include "../Common/ProtocolConstants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(PacketSerializerTests)
    {
    public:
        TEST_METHOD(SerializeDeserialize_ValidPacket_RoundTripsSuccessfully)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("AEROSTOCK_VERIFY");

            AeroStock::Common::Packet originalPacket(
                1U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::Success,
                payload);

            const auto buffer =
                AeroStock::Common::PacketSerializer::Serialize(originalPacket);

            AeroStock::Common::Packet deserializedPacket{};

            const bool result =
                AeroStock::Common::PacketSerializer::Deserialize(buffer, deserializedPacket);

            Assert::IsTrue(result);
            Assert::AreEqual(originalPacket.GetPacketId(), deserializedPacket.GetPacketId());
            Assert::AreEqual(
                static_cast<int>(originalPacket.GetCommandType()),
                static_cast<int>(deserializedPacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(originalPacket.GetStatus()),
                static_cast<int>(deserializedPacket.GetStatus()));
            Assert::AreEqual(
                AeroStock::Common::PayloadUtils::PayloadToString(originalPacket.GetPayload()).c_str(),
                AeroStock::Common::PayloadUtils::PayloadToString(deserializedPacket.GetPayload()).c_str());
        }

        TEST_METHOD(Deserialize_ModifiedPayload_FailsChecksumValidation)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("AEROSTOCK_VERIFY");

            AeroStock::Common::Packet packet(
                1U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::Success,
                payload);

            auto buffer = AeroStock::Common::PacketSerializer::Serialize(packet);

            Assert::IsTrue(buffer.size() > AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes);

            buffer[AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes] ^= 0x01U;

            AeroStock::Common::Packet outputPacket{};
            const bool result =
                AeroStock::Common::PacketSerializer::Deserialize(buffer, outputPacket);

            Assert::IsFalse(result);
        }

        TEST_METHOD(Deserialize_InvalidPayloadLength_Fails)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("TEST");

            AeroStock::Common::Packet packet(
                2U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success,
                payload);

            auto buffer = AeroStock::Common::PacketSerializer::Serialize(packet);

            // Corrupt the payload length field in the serialized header.
            buffer[AeroStock::Common::ProtocolConstants::PayloadLengthOffsetBytes + 3U] = 10U;

            AeroStock::Common::Packet outputPacket{};
            const bool result =
                AeroStock::Common::PacketSerializer::Deserialize(buffer, outputPacket);

            Assert::IsFalse(result);
        }

        TEST_METHOD(CanDeserializeHeader_BufferTooSmall_ReturnsFalse)
        {
            AeroStock::Common::PacketSerializer::ByteBuffer smallBuffer(5U, 0U);

            Assert::IsFalse(
                AeroStock::Common::PacketSerializer::CanDeserializeHeader(smallBuffer));
        }

        TEST_METHOD(DeserializeHeader_ValidSerializedPacketHeader_ReturnsTrue)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("OK");

            AeroStock::Common::Packet packet(
                3U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success,
                payload);

            const auto buffer = AeroStock::Common::PacketSerializer::Serialize(packet);

            AeroStock::Common::PacketHeader header{};
            const bool result =
                AeroStock::Common::PacketSerializer::DeserializeHeader(buffer, header);

            Assert::IsTrue(result);
            Assert::AreEqual(3U, header.packetId);
            Assert::AreEqual(2U, header.payloadLength);
        }

        // Verifies that a packet with no payload can still round-trip successfully.
        TEST_METHOD(SerializeDeserialize_EmptyPayload_RoundTripsSuccessfully)
        {
            AeroStock::Common::Packet originalPacket(
                10U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            const auto buffer =
                AeroStock::Common::PacketSerializer::Serialize(originalPacket);

            AeroStock::Common::Packet deserializedPacket{};
            const bool result =
                AeroStock::Common::PacketSerializer::Deserialize(buffer, deserializedPacket);

            Assert::IsTrue(result);
            Assert::AreEqual(0U, deserializedPacket.GetPayloadLength());
            Assert::IsFalse(deserializedPacket.HasPayload());
        }

        // Verifies that deserialization fails if the command byte is corrupted to an invalid value.
        TEST_METHOD(Deserialize_InvalidCommandType_Fails)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("TEST");

            AeroStock::Common::Packet packet(
                11U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success,
                payload);

            auto buffer = AeroStock::Common::PacketSerializer::Serialize(packet);

            buffer[AeroStock::Common::ProtocolConstants::CommandOffsetBytes] = 255U;

            AeroStock::Common::Packet outputPacket{};
            const bool result =
                AeroStock::Common::PacketSerializer::Deserialize(buffer, outputPacket);

            Assert::IsFalse(result);
        }

        // Verifies that deserialization fails when the serialized buffer is too short
        // to contain the full payload declared in the header.
        TEST_METHOD(Deserialize_TruncatedPayload_Fails)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("HELLO");

            AeroStock::Common::Packet packet(
                12U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success,
                payload);

            auto buffer = AeroStock::Common::PacketSerializer::Serialize(packet);

            buffer.pop_back();

            AeroStock::Common::Packet outputPacket{};
            const bool result =
                AeroStock::Common::PacketSerializer::Deserialize(buffer, outputPacket);

            Assert::IsFalse(result);
        }
    };
}