#pragma once

#include <cstdint>
#include <string>

namespace AeroStock::Client
{
    // Coordinates the interactive CLI session and delegates packet creation/parsing.
    class ClientApp final
    {
    public:
        [[nodiscard]] int Run();

    private:
        [[nodiscard]] std::string PromptForServerIp() const;
        [[nodiscard]] std::uint16_t PromptForPort() const;
        [[nodiscard]] int PromptForMenuSelection() const;
        [[nodiscard]] std::string PromptForText(const std::string& prompt) const;
        [[nodiscard]] std::uint32_t PromptForQuantity() const;

        void PrintMenu() const;
        void ClearScreen() const;
        void PauseForReturnToMenu() const;
        void PrintOperationHeader(const std::string& title) const;
    };
}