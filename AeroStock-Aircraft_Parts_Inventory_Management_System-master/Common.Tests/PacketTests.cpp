#include "pch.h"
#include "CppUnitTest.h"

#include "../Common/Packet.h"
#include "../Common/PayloadUtils.h"
#include "../Common/ProtocolConstants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(PacketTests)
    {
    public:
        // Verifies that packet construction correctly stores the header values
        // and synchronizes the payload length with the provided payload.
        TEST_METHOD(PacketConstructor_SetsHeaderAndPayloadCorrectly)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("HELLO");

            AeroStock::Common::Packet packet(
                1U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::Success,
                payload);

            Assert::AreEqual(1U, packet.GetPacketId());
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Verify),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual(5U, packet.GetPayloadLength());
            Assert::IsTrue(packet.HasPayload());
        }

        // Verifies that assigning a string payload updates the payload size
        // and marks the packet as holding payload data.
        TEST_METHOD(SetPayloadFromString_UpdatesPayloadLength)
        {
            AeroStock::Common::Packet packet(
                1U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::Success);

            packet.SetPayloadFromString("AEROSTOCK_VERIFY");

            Assert::AreEqual(16U, packet.GetPayloadLength());
            Assert::IsTrue(packet.HasPayload());
        }

        // Verifies that clearing the payload resets both the payload buffer
        // and the stored payload length.
        TEST_METHOD(ClearPayload_ResetsPayloadAndLength)
        {
            AeroStock::Common::Packet packet(
                1U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::Success);

            packet.SetPayloadFromString("HELLO");
            packet.ClearPayload();

            Assert::AreEqual(0U, packet.GetPayloadLength());
            Assert::IsFalse(packet.HasPayload());
        }

        // Verifies that appending new payload data preserves existing content
        // and updates the total payload length correctly.
        TEST_METHOD(AppendPayloadData_AppendsCorrectly)
        {
            AeroStock::Common::Packet packet(
                1U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            packet.SetPayloadFromString("ABC");

            const auto extra =
                AeroStock::Common::PayloadUtils::StringToPayload("DEF");

            packet.AppendPayloadData(extra);

            const auto text =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            Assert::AreEqual("ABCDEF", text.c_str());
            Assert::AreEqual(6U, packet.GetPayloadLength());
        }

        // Verifies that a packet using the reserved invalid packet ID fails validation.
        TEST_METHOD(PacketWithInvalidPacketId_IsInvalid)
        {
            AeroStock::Common::Packet packet(
                AeroStock::Common::ProtocolConstants::InvalidPacketId,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::Success);

            Assert::IsFalse(packet.IsValid());
        }

        // Verifies that clearing the checksum resets the stored checksum value to zero.
        TEST_METHOD(ClearChecksum_ResetsChecksumToZero)
        {
            AeroStock::Common::Packet packet(
                20U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            packet.SetChecksum(12345U);
            packet.ClearChecksum();

            Assert::AreEqual(0U, packet.GetChecksum());
        }

        // Verifies that attempting to append data beyond the supported payload size
        // does not increase the packet payload.
        TEST_METHOD(AppendPayloadData_OversizedInput_DoesNotModifyPayload)
        {
            AeroStock::Common::Packet packet(
                21U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            const AeroStock::Common::Packet::PayloadBuffer oversizedPayload(
                static_cast<std::size_t>(AeroStock::Common::ProtocolConstants::MaxPayloadSizeBytes) + 1U,
                static_cast<std::uint8_t>('X'));

            packet.AppendPayloadData(oversizedPayload);

            Assert::AreEqual(0U, packet.GetPayloadLength());
            Assert::IsFalse(packet.HasPayload());
        }
    };
}