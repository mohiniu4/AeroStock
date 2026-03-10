#include "pch.h"
#include "CppUnitTest.h"

#include "RequestHandler.h"
#include "ServerStateMachine.h"
#include "PayloadUtils.h"
#include "ProtocolConstants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ServerTests
{
    TEST_CLASS(RequestHandlerTests)
    {
    public:
        TEST_METHOD(HandleRequest_ConnectInDisconnectedState_Succeeds)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet requestPacket(
                1U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(requestPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual("CONNECTED",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Connected),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_ValidVerifyRequest_Succeeds)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                2U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);

            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                3U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);

            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));

            const auto responsePacket = handler.HandleRequest(verifyPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                AeroStock::Common::ProtocolConstants::VerificationSuccessText,
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_InvalidVerifyPayload_ReturnsVerificationFailed)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                4U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);

            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                5U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);

            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("WRONG_VERIFY_TEXT"));

            const auto responsePacket = handler.HandleRequest(verifyPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::VerificationFailed),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                AeroStock::Common::ProtocolConstants::VerificationFailureText,
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Error),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_CommandNotAllowedInCurrentState_ReturnsInvalidRequest)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet verifyPacket(
                6U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);

            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));

            const auto responsePacket = handler.HandleRequest(verifyPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::InvalidRequest),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "REQUEST_NOT_ALLOWED_IN_CURRENT_STATE",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Error),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_SearchByPartNumber_EmptyPayload_ReturnsInvalidRequest)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                7U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                8U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet searchPacket(
                9U,
                AeroStock::Common::CommandType::SearchByPartNumber,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(searchPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::InvalidRequest),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "PART_NUMBER_REQUIRED",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_SearchByPartNumber_UnknownPart_ReturnsNotFound)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                10U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                11U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet searchPacket(
                12U,
                AeroStock::Common::CommandType::SearchByPartNumber,
                AeroStock::Common::StatusCode::None);
            searchPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("PN-9999"));

            const auto responsePacket = handler.HandleRequest(searchPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::NotFound),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "PART_NOT_FOUND",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_SearchByPartNumber_KnownPart_ReturnsSerializedRecord)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                13U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                14U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet searchPacket(
                15U,
                AeroStock::Common::CommandType::SearchByPartNumber,
                AeroStock::Common::StatusCode::None);
            searchPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("PN-1001"));

            const auto responsePacket = handler.HandleRequest(searchPacket, stateMachine);
            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));

            Assert::IsTrue(payloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Hydraulic Pump") != std::string::npos);
            Assert::IsTrue(payloadText.find("Stock=25") != std::string::npos);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_SearchByPartName_EmptyPayload_ReturnsInvalidRequest)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                18U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                19U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet searchPacket(
                20U,
                AeroStock::Common::CommandType::SearchByPartName,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(searchPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::InvalidRequest),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "PART_NAME_REQUIRED",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_SearchByPartName_UnknownName_ReturnsNotFound)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                21U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                22U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet searchPacket(
                23U,
                AeroStock::Common::CommandType::SearchByPartName,
                AeroStock::Common::StatusCode::None);
            searchPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("Quantum Reactor"));

            const auto responsePacket = handler.HandleRequest(searchPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::NotFound),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "PART_NAME_NOT_FOUND",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_SearchByPartName_KnownName_ReturnsSerializedResults)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                24U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                25U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet searchPacket(
                26U,
                AeroStock::Common::CommandType::SearchByPartName,
                AeroStock::Common::StatusCode::None);
            searchPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("Hydraulic"));

            const auto responsePacket = handler.HandleRequest(searchPacket, stateMachine);
            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));

            Assert::IsTrue(payloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartNumber=PN-1013") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Hydraulic Pump") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Hydraulic Filter Element") != std::string::npos);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_Disconnect_SucceedsAndResetsState)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                27U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet disconnectPacket(
                28U,
                AeroStock::Common::CommandType::Disconnect,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(disconnectPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "DISCONNECTED",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Disconnected),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_GetPartDetails_EmptyPayload_ReturnsInvalidRequest)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                29U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                30U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet detailsPacket(
                31U,
                AeroStock::Common::CommandType::GetPartDetails,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(detailsPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::InvalidRequest),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "PART_NUMBER_REQUIRED",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_GetPartDetails_UnknownPart_ReturnsNotFound)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                32U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                33U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet detailsPacket(
                34U,
                AeroStock::Common::CommandType::GetPartDetails,
                AeroStock::Common::StatusCode::None);
            detailsPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("PN-9999"));

            const auto responsePacket = handler.HandleRequest(detailsPacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::NotFound),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "PART_DETAILS_NOT_FOUND",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_GetPartDetails_KnownPart_ReturnsSerializedRecord)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                35U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                36U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet detailsPacket(
                37U,
                AeroStock::Common::CommandType::GetPartDetails,
                AeroStock::Common::StatusCode::None);
            detailsPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("PN-1003"));

            const auto responsePacket = handler.HandleRequest(detailsPacket, stateMachine);
            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));

            Assert::IsTrue(payloadText.find("PartNumber=PN-1003") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Avionics Display Unit") != std::string::npos);
            Assert::IsTrue(payloadText.find("Stock=8") != std::string::npos);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_UpdateStock_EmptyPayload_ReturnsInvalidRequest)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                38U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                39U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet updatePacket(
                40U,
                AeroStock::Common::CommandType::UpdateStock,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(updatePacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::InvalidRequest),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "UPDATE_STOCK_PAYLOAD_REQUIRED",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_UpdateStock_InvalidFormat_ReturnsInvalidRequest)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                41U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                42U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet updatePacket(
                43U,
                AeroStock::Common::CommandType::UpdateStock,
                AeroStock::Common::StatusCode::None);
            updatePacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("PN-1001"));

            const auto responsePacket = handler.HandleRequest(updatePacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::InvalidRequest),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "INVALID_UPDATE_STOCK_FORMAT",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_UpdateStock_UnknownPart_ReturnsNotFound)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                44U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                45U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet updatePacket(
                46U,
                AeroStock::Common::CommandType::UpdateStock,
                AeroStock::Common::StatusCode::None);
            updatePacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("PN-9999,50"));

            const auto responsePacket = handler.HandleRequest(updatePacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Error),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::NotFound),
                static_cast<int>(responsePacket.GetStatus()));
            Assert::AreEqual(
                "PART_NOT_FOUND",
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload()).c_str());
        }

        TEST_METHOD(HandleRequest_UpdateStock_ValidPayload_UpdatesRecordAndReturnsSuccess)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                47U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                48U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet updatePacket(
                49U,
                AeroStock::Common::CommandType::UpdateStock,
                AeroStock::Common::StatusCode::None);
            updatePacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload("PN-1001,99"));

            const auto responsePacket = handler.HandleRequest(updatePacket, stateMachine);
            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload());

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));

            Assert::IsTrue(payloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(payloadText.find("Stock=99") != std::string::npos);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_RequestFile_EmptyPayload_StillReturnsFileSuccessfully)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                50U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                51U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet requestFilePacket(
                52U,
                AeroStock::Common::CommandType::RequestFile,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(requestFilePacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));

            Assert::IsTrue(responsePacket.HasPayload());
            Assert::IsTrue(responsePacket.GetPayload().size() >= (1024U * 1024U));

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(HandleRequest_RequestFile_ReturnsLargeInventoryCatalogPayload)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                53U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                54U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet requestFilePacket(
                55U,
                AeroStock::Common::CommandType::RequestFile,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(requestFilePacket, stateMachine);
            const auto& payload = responsePacket.GetPayload();

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));

            Assert::IsTrue(responsePacket.HasPayload());
            Assert::IsTrue(payload.size() >= (1024U * 1024U));
            Assert::IsTrue(
                payload.size() <= AeroStock::Common::ProtocolConstants::MaxPayloadSizeBytes);
        }

        TEST_METHOD(HandleRequest_RequestFile_PayloadContainsInventoryExportContent)
        {
            AeroStock::Server::RequestHandler handler{};
            AeroStock::Server::ServerStateMachine stateMachine{};

            AeroStock::Common::Packet connectPacket(
                56U,
                AeroStock::Common::CommandType::Connect,
                AeroStock::Common::StatusCode::None);
            static_cast<void>(handler.HandleRequest(connectPacket, stateMachine));

            AeroStock::Common::Packet verifyPacket(
                57U,
                AeroStock::Common::CommandType::Verify,
                AeroStock::Common::StatusCode::None);
            verifyPacket.SetPayload(
                AeroStock::Common::PayloadUtils::StringToPayload(
                    AeroStock::Common::ProtocolConstants::VerificationRequestText));
            static_cast<void>(handler.HandleRequest(verifyPacket, stateMachine));

            AeroStock::Common::Packet requestFilePacket(
                58U,
                AeroStock::Common::CommandType::RequestFile,
                AeroStock::Common::StatusCode::None);

            const auto responsePacket = handler.HandleRequest(requestFilePacket, stateMachine);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::CommandType::Response),
                static_cast<int>(responsePacket.GetCommandType()));
            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::Success),
                static_cast<int>(responsePacket.GetStatus()));

            const auto payloadText =
                AeroStock::Common::PayloadUtils::PayloadToString(responsePacket.GetPayload());

            Assert::IsTrue(payloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Hydraulic Pump") != std::string::npos);
            Assert::IsTrue(payloadText.find("ExportBatch=") != std::string::npos);
        }
    };
}