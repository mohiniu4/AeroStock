#pragma once

#include "Enums.h"

namespace AeroStock::Server
{
    // tracks and enforces protocol transitions for session
    class ServerStateMachine final
    {
    public:
        ServerStateMachine() = default;

        [[nodiscard]] AeroStock::Common::ServerState GetCurrentState() const noexcept;
        void Reset() noexcept;
        [[nodiscard]] bool CanProcessCommand(AeroStock::Common::CommandType commandType) const noexcept;
        void ApplySuccessfulCommand(AeroStock::Common::CommandType commandType) noexcept;

        // Move into the request-processing phase while a valid command is being handled.
        void MoveToProcessingState() noexcept;

        // Move into the data-sending phase while a response packet is being transmitted.
        void MoveToSendingDataState() noexcept;

        // Move the session into a terminal error state.
        void MoveToErrorState() noexcept;

        // Restore a previously known valid state after a temporary transition.
        void RestoreState(AeroStock::Common::ServerState state) noexcept;

    private:
        AeroStock::Common::ServerState currentState_{ AeroStock::Common::ServerState::Disconnected };
    };
}