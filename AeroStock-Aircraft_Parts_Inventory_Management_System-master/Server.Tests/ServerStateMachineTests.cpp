#include "pch.h"
#include "CppUnitTest.h"

#include "ServerStateMachine.h"
#include "Enums.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ServerTests
{
    TEST_CLASS(ServerStateMachineTests)
    {
    public:
        TEST_METHOD(DefaultState_IsDisconnected)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Disconnected),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(CanProcessCommand_Disconnected_AllowsOnlyConnect)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};

            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Connect));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Verify));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Disconnect));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::SearchByPartNumber));
        }

        TEST_METHOD(ApplySuccessfulCommand_Connect_MovesToConnected)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};

            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Connected),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(CanProcessCommand_Connected_AllowsVerifyAndDisconnect)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);

            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Verify));
            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Disconnect));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Connect));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::SearchByPartNumber));
        }

        TEST_METHOD(ApplySuccessfulCommand_Verify_MovesToVerified)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);

            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Verify);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(CanProcessCommand_Verified_AllowsOperationalCommands)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Verify);

            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::SearchByPartNumber));
            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::SearchByPartName));
            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::GetPartDetails));
            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::UpdateStock));
            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::RequestFile));
            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Disconnect));

            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Connect));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Verify));
        }

        TEST_METHOD(ApplySuccessfulCommand_Disconnect_MovesToDisconnected)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Verify);

            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Disconnect);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Disconnected),
                static_cast<int>(stateMachine.GetCurrentState()));
        }

        TEST_METHOD(MoveToSendingDataState_MovesToSendingData)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Verify);

            stateMachine.MoveToSendingDataState();

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::SendingData),
                static_cast<int>(stateMachine.GetCurrentState()));

            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::SearchByPartNumber));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Disconnect));
        }

        TEST_METHOD(RestoreState_RestoresPreviousState)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Verify);

            stateMachine.MoveToSendingDataState();
            stateMachine.RestoreState(AeroStock::Common::ServerState::Verified);

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Verified),
                static_cast<int>(stateMachine.GetCurrentState()));

            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::SearchByPartNumber));
            Assert::IsTrue(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Disconnect));
        }

        TEST_METHOD(MoveToErrorState_BlocksCommands)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.MoveToErrorState();

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Error),
                static_cast<int>(stateMachine.GetCurrentState()));

            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Connect));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Verify));
            Assert::IsFalse(stateMachine.CanProcessCommand(AeroStock::Common::CommandType::Disconnect));
        }

        TEST_METHOD(Reset_ReturnsToDisconnected)
        {
            AeroStock::Server::ServerStateMachine stateMachine{};
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Connect);
            stateMachine.ApplySuccessfulCommand(AeroStock::Common::CommandType::Verify);

            stateMachine.Reset();

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::ServerState::Disconnected),
                static_cast<int>(stateMachine.GetCurrentState()));
        }
    };
}