#include "pch.h"
#include "ServerStateMachine.h"

namespace AeroStock::Server
{
    AeroStock::Common::ServerState ServerStateMachine::GetCurrentState() const noexcept
    {
        return currentState_;
    }

    void ServerStateMachine::Reset() noexcept
    {
        currentState_ = AeroStock::Common::ServerState::Disconnected;
    }

    // defines which commands are valid in each state
    bool ServerStateMachine::CanProcessCommand(const AeroStock::Common::CommandType commandType) const noexcept
    {
        using AeroStock::Common::CommandType;
        using AeroStock::Common::ServerState;

        switch (currentState_)
        {
        case ServerState::Disconnected:
            return commandType == CommandType::Connect;

        case ServerState::Connected:
            return (commandType == CommandType::Verify) ||
                (commandType == CommandType::Disconnect);

        case ServerState::Verified:
            switch (commandType)
            {
            case CommandType::Disconnect:
            case CommandType::SearchByPartNumber:
            case CommandType::SearchByPartName:
            case CommandType::GetPartDetails:
            case CommandType::UpdateStock:
            case CommandType::RequestFile:
                return true;

            default:
                return false;
            }

            // While the server is actively processing or sending data, it should not
            // accept any new command for the current client session.
        case ServerState::Processing:
        case ServerState::SendingData:
        case ServerState::Error:
        default:
            return false;
        }
    }

    // maps each command to the state it should transition to on success
    void ServerStateMachine::ApplySuccessfulCommand(const AeroStock::Common::CommandType commandType) noexcept
    {
        using AeroStock::Common::CommandType;
        using AeroStock::Common::ServerState;

        switch (commandType)
        {
        case CommandType::Connect:
            currentState_ = ServerState::Connected;
            break;

        case CommandType::Verify:
        case CommandType::SearchByPartNumber:
        case CommandType::SearchByPartName:
        case CommandType::GetPartDetails:
        case CommandType::UpdateStock:
        case CommandType::RequestFile:
            currentState_ = ServerState::Verified;
            break;

        case CommandType::Disconnect:
            currentState_ = ServerState::Disconnected;
            break;

        default:
            break;
        }
    }

    void ServerStateMachine::MoveToProcessingState() noexcept
    {
        currentState_ = AeroStock::Common::ServerState::Processing;
    }

    void ServerStateMachine::MoveToSendingDataState() noexcept
    {
        currentState_ = AeroStock::Common::ServerState::SendingData;
    }

    void ServerStateMachine::MoveToErrorState() noexcept
    {
        currentState_ = AeroStock::Common::ServerState::Error;
    }

    void ServerStateMachine::RestoreState(const AeroStock::Common::ServerState state) noexcept
    {
        currentState_ = state;
    }
}