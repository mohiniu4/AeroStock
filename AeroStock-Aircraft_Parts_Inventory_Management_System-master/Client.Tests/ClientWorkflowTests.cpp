#include "pch.h"
#include "CppUnitTest.h"

#include "ClientWorkflow.h"
#include "PayloadUtils.h"
#include "ProtocolConstants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ClientTests
{
    TEST_CLASS(ClientWorkflowTests)
    {
    public:
        TEST_METHOD(CreateConnectPacket_CreatesExpectedPacket)
        {
            const auto packet = AeroStock::Client::ClientWorkflow::CreateConnectPacket(1U);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Connect),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::None),
                static_cast<int>(packet.GetStatus()));
            Assert::AreEqual(static_cast<std::size_t>(0), packet.GetPayload().size());
        }

        TEST_METHOD(CreateVerifyPacket_CreatesExpectedPacket)
        {
            const auto packet = AeroStock::Client::ClientWorkflow::CreateVerifyPacket(2U);
            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Verify),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual(
                AeroStock::Common::ProtocolConstants::VerificationRequestText,
                payloadText.c_str());
        }

        TEST_METHOD(CreateSearchByPartNumberPacket_CreatesExpectedPacket)
        {
            const auto packet =
                AeroStock::Client::ClientWorkflow::CreateSearchByPartNumberPacket(3U, "PN-1001");

            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::SearchByPartNumber),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual("PN-1001", payloadText.c_str());
        }

        TEST_METHOD(CreateSearchByPartNamePacket_CreatesExpectedPacket)
        {
            const auto packet =
                AeroStock::Client::ClientWorkflow::CreateSearchByPartNamePacket(4U, "Hydraulic");

            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::SearchByPartName),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual("Hydraulic", payloadText.c_str());
        }

        TEST_METHOD(CreateGetPartDetailsPacket_CreatesExpectedPacket)
        {
            const auto packet =
                AeroStock::Client::ClientWorkflow::CreateGetPartDetailsPacket(5U, "PN-1003");

            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::GetPartDetails),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual("PN-1003", payloadText.c_str());
        }

        TEST_METHOD(CreateUpdateStockPacket_CreatesExpectedPacket)
        {
            const auto packet =
                AeroStock::Client::ClientWorkflow::CreateUpdateStockPacket(6U, "PN-1001", 99U);

            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::UpdateStock),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual("PN-1001,99", payloadText.c_str());
        }

        TEST_METHOD(CreateRequestFilePacket_CreatesExpectedPacket)
        {
            const auto packet =
                AeroStock::Client::ClientWorkflow::CreateRequestFilePacket(7U);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::RequestFile),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual(static_cast<std::size_t>(0), packet.GetPayload().size());
        }

        TEST_METHOD(CreateDisconnectPacket_CreatesExpectedPacket)
        {
            const auto packet =
                AeroStock::Client::ClientWorkflow::CreateDisconnectPacket(8U);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Disconnect),
                static_cast<int>(packet.GetCommandType()));
            Assert::AreEqual(static_cast<std::size_t>(0), packet.GetPayload().size());
        }

        TEST_METHOD(IsSuccessfulResponse_SuccessResponse_ReturnsTrue)
        {
            AeroStock::Common::Packet packet(
                9U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            Assert::IsTrue(AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(packet));
        }

        TEST_METHOD(IsSuccessfulResponse_ErrorResponse_ReturnsFalse)
        {
            AeroStock::Common::Packet packet(
                10U,
                AeroStock::Common::CommandType::Error,
                AeroStock::Common::StatusCode::InvalidRequest);

            Assert::IsFalse(AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(packet));
        }

        TEST_METHOD(IsErrorResponse_ErrorPacket_ReturnsTrue)
        {
            AeroStock::Common::Packet packet(
                11U,
                AeroStock::Common::CommandType::Error,
                AeroStock::Common::StatusCode::InvalidRequest);

            Assert::IsTrue(AeroStock::Client::ClientWorkflow::IsErrorResponse(packet));
        }

        TEST_METHOD(IsErrorResponse_SuccessPacket_ReturnsFalse)
        {
            AeroStock::Common::Packet packet(
                12U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            Assert::IsFalse(AeroStock::Client::ClientWorkflow::IsErrorResponse(packet));
        }

        TEST_METHOD(GetPayloadText_WithPayload_ReturnsText)
        {
            AeroStock::Common::Packet packet(
                13U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            packet.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("CONNECTED"));

            const auto payloadText =
                AeroStock::Client::ClientWorkflow::GetPayloadText(packet);

            Assert::AreEqual("CONNECTED", payloadText.c_str());
        }

        TEST_METHOD(GetPayloadText_WithoutPayload_ReturnsEmptyString)
        {
            AeroStock::Common::Packet packet(
                14U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            const auto payloadText =
                AeroStock::Client::ClientWorkflow::GetPayloadText(packet);

            Assert::AreEqual("", payloadText.c_str());
        }

        TEST_METHOD(FormatResponseForDisplay_NonRecordPayload_ReturnsOriginalText)
        {
            AeroStock::Common::Packet packet(
                15U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            packet.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("VERIFIED"));

            const auto formatted =
                AeroStock::Client::ClientWorkflow::FormatResponseForDisplay(packet);

            Assert::AreEqual("VERIFIED", formatted.c_str());
        }

        TEST_METHOD(FormatResponseForDisplay_SingleRecord_FormatsFields)
        {
            AeroStock::Common::Packet packet(
                16U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            packet.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    "PartNumber=PN-1001;PartName=Hydraulic Pump;Category=Hydraulics;Location=A1-SHELF-3;Supplier=Boeing Supply;Stock=25"));

            const auto formatted =
                AeroStock::Client::ClientWorkflow::FormatResponseForDisplay(packet);

            Assert::IsTrue(formatted.find("Part Number : PN-1001") != std::string::npos);
            Assert::IsTrue(formatted.find("Part Name   : Hydraulic Pump") != std::string::npos);
            Assert::IsTrue(formatted.find("Category    : Hydraulics") != std::string::npos);
            Assert::IsTrue(formatted.find("Location    : A1-SHELF-3") != std::string::npos);
            Assert::IsTrue(formatted.find("Supplier    : Boeing Supply") != std::string::npos);
            Assert::IsTrue(formatted.find("Stock       : 25") != std::string::npos);
        }

        TEST_METHOD(FormatResponseForDisplay_MultiRecord_FormatsMultipleResults)
        {
            AeroStock::Common::Packet packet(
                17U,
                AeroStock::Common::CommandType::Response,
                AeroStock::Common::StatusCode::Success);

            packet.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    "PartNumber=PN-1001;PartName=Hydraulic Pump;Category=Hydraulics;Location=A1-SHELF-3;Supplier=Boeing Supply;Stock=25\n"
                    "PartNumber=PN-1013;PartName=Hydraulic Filter Element;Category=Hydraulics;Location=A1-SHELF-7;Supplier=Pall Corporation;Stock=35"));

            const auto formatted =
                AeroStock::Client::ClientWorkflow::FormatResponseForDisplay(packet);

            Assert::IsTrue(formatted.find("Result 1") != std::string::npos);
            Assert::IsTrue(formatted.find("Result 2") != std::string::npos);
            Assert::IsTrue(formatted.find("Part Number : PN-1001") != std::string::npos);
            Assert::IsTrue(formatted.find("Part Number : PN-1013") != std::string::npos);
        }
    };
}