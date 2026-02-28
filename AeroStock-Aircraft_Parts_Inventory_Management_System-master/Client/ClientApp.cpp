#include "ClientApp.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "TcpClient.h"
#include "../Client.Core/ClientFileManager.h"
#include "../Client.Core/ClientWorkflow.h"
#include "../Common/ProtocolConstants.h"

// Implementation of the ClientApp class, which manages user interaction and communication with the server
namespace AeroStock::Client
{
    namespace
    {
        [[nodiscard]] bool TryParseUnsignedInt(
            const std::string& text,
            unsigned int& outValue)
        {
            if (text.empty())
            {
                return false;
            }

            try
            {
                std::size_t processedCharacters = 0U;
                const unsigned long parsedValue =
                    std::stoul(text, &processedCharacters);

                if (processedCharacters != text.size())
                {
                    return false;
                }

                if (parsedValue > static_cast<unsigned long>(
                    (std::numeric_limits<unsigned int>::max)()))
                {
                    return false;
                }

                outValue = static_cast<unsigned int>(parsedValue);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
    }

    // Prompt for server IP address and validate it
    std::string ClientApp::PromptForServerIp() const
    {
        std::string serverIpAddress;

        std::cout << "Enter server IP address: ";
        std::getline(std::cin, serverIpAddress);

        if (serverIpAddress.empty())
        {
            throw std::runtime_error("Server IP address cannot be empty.");
        }

        return serverIpAddress;
    }

    // Prompt for server port and validate it is a number between 1 and 65535.
    // If the user presses Enter without typing a value, use the shared default port.
    std::uint16_t ClientApp::PromptForPort() const
    {
        std::string input;
        unsigned int port = 0U;

        std::cout
            << "Enter server port (press Enter for default "
            << AeroStock::Common::ProtocolConstants::DefaultPort
            << "): ";
        std::getline(std::cin, input);

        if (input.empty())
        {
            return AeroStock::Common::ProtocolConstants::DefaultPort;
        }

        if (!TryParseUnsignedInt(input, port))
        {
            throw std::runtime_error("Invalid port input.");
        }

        if (port == 0U || port > 65535U)
        {
            throw std::runtime_error("Port must be between 1 and 65535.");
        }

        return static_cast<std::uint16_t>(port);
    }

    // Prompt for menu selection and validate it is a number corresponding to a menu option
    int ClientApp::PromptForMenuSelection() const
    {
        std::string input;
        unsigned int selection = 0U;

        std::cout << "\nSelect an option: ";
        std::getline(std::cin, input);

        if (!TryParseUnsignedInt(input, selection))
        {
            throw std::runtime_error("Invalid menu selection.");
        }

        return static_cast<int>(selection);
    }

    // Prompt for generic text input with a custom prompt message
    std::string ClientApp::PromptForText(const std::string& prompt) const
    {
        std::string value;

        std::cout << prompt;
        std::getline(std::cin, value);

        if (value.empty())
        {
            throw std::runtime_error("Input cannot be empty.");
        }

        return value;
    }

    // Prompt for stock quantity and validate it is a non-negative integer
    std::uint32_t ClientApp::PromptForQuantity() const
    {
        std::string input;
        unsigned int quantity = 0U;

        std::cout << "Enter new stock quantity: ";
        std::getline(std::cin, input);

        if (!TryParseUnsignedInt(input, quantity))
        {
            throw std::runtime_error("Invalid quantity input.");
        }

        return static_cast<std::uint32_t>(quantity);
    }

    // Display the main menu options to the user
    void ClientApp::PrintMenu() const
    {
        std::cout << "========================================" << std::endl;
        std::cout << " AeroStock Client Menu" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "1. Search by Part Number" << std::endl;
        std::cout << "2. Search by Part Name" << std::endl;
        std::cout << "3. Update Stock" << std::endl;
        std::cout << "4. Request Inventory Catalog File" << std::endl;
        std::cout << "5. Disconnect and Exit" << std::endl;
    }

    // Clear the console screen using Windows API calls
    void ClientApp::ClearScreen() const
    {
        const HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        if (consoleHandle == INVALID_HANDLE_VALUE)
        {
            return;
        }

        CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo{};
        if (!GetConsoleScreenBufferInfo(consoleHandle, &screenBufferInfo))
        {
            return;
        }

        const DWORD consoleCellCount =
            static_cast<DWORD>(screenBufferInfo.dwSize.X) *
            static_cast<DWORD>(screenBufferInfo.dwSize.Y);

        DWORD cellsWritten = 0U;

        static_cast<void>(FillConsoleOutputCharacter(
            consoleHandle,
            TEXT(' '),
            consoleCellCount,
            { 0, 0 },
            &cellsWritten));

        static_cast<void>(FillConsoleOutputAttribute(
            consoleHandle,
            screenBufferInfo.wAttributes,
            consoleCellCount,
            { 0, 0 },
            &cellsWritten));

        static_cast<void>(SetConsoleCursorPosition(consoleHandle, { 0, 0 }));
    }

    // Pause the console and wait for the user to press Enter before returning to the main menu
    void ClientApp::PauseForReturnToMenu() const
    {
        std::cout << "\nPress Enter to return to the main menu...";
        std::string ignoredInput;
        std::getline(std::cin, ignoredInput);
    }

    // Print a header for the current operation being performed, clearing the screen first
    void ClientApp::PrintOperationHeader(const std::string& title) const
    {
        ClearScreen();
        std::cout << "========================================" << std::endl;
        std::cout << ' ' << title << std::endl;
        std::cout << "========================================" << std::endl;
    }

    // Run the full interactive client session from connect to disconnect.
    int ClientApp::Run()
    {
        ClearScreen();

        std::cout << "========================================" << std::endl;
        std::cout << " AeroStock Client" << std::endl;
        std::cout << " Inventory Management CLI" << std::endl;
        std::cout << "========================================" << std::endl;

        const std::string serverIpAddress = PromptForServerIp();
        const std::uint16_t port = PromptForPort();

        TcpClient tcpClient{};
        std::uint32_t nextPacketId = 1U;

        std::cout << "\nConnecting to server..." << std::endl;
        tcpClient.Connect(serverIpAddress, port);
        std::cout << "Connected successfully." << std::endl;

        const auto connectPacket = ClientWorkflow::CreateConnectPacket(nextPacketId++);
        tcpClient.SendPacket(connectPacket);
        const auto connectResponse = tcpClient.ReceivePacket();

        if (!ClientWorkflow::IsSuccessfulResponse(connectResponse))
        {
            throw std::runtime_error(
                "Server rejected Connect request: " +
                ClientWorkflow::GetPayloadText(connectResponse));
        }

        std::cout
            << "Connect response: "
            << ClientWorkflow::GetPayloadText(connectResponse)
            << std::endl;

        const auto verifyPacket = ClientWorkflow::CreateVerifyPacket(nextPacketId++);
        tcpClient.SendPacket(verifyPacket);
        const auto verifyResponse = tcpClient.ReceivePacket();

        // If the verify response is not successful, throw an error with the server's response text
        if (!ClientWorkflow::IsSuccessfulResponse(verifyResponse))
        {
            throw std::runtime_error(
                "Server verification failed: " +
                ClientWorkflow::GetPayloadText(verifyResponse));
        }

        std::cout
            << "Verify response: "
            << ClientWorkflow::GetPayloadText(verifyResponse)
            << std::endl;

        bool sessionActive = true;

        // Main menu loop - continues until the user chooses to disconnect or an error occurs
        while (sessionActive)
        {
            try
            {
                ClearScreen();
                PrintMenu();
                const int selection = PromptForMenuSelection();

                AeroStock::Common::Packet requestPacket{};

                // Handle the user's menu selection and perform the corresponding client-server interaction
                switch (selection)
                {
                case 1:
                {
                    // For the "Search by Part Number" option, prompt the user for a part number,
                    // create a search packet, send it to the server,
                    PrintOperationHeader("Search by Part Number");

                    const std::string partNumber =
                        PromptForText("Enter part number (PN-####): ");

                    requestPacket = ClientWorkflow::CreateSearchByPartNumberPacket(
                        nextPacketId++,
                        partNumber);

                    tcpClient.SendPacket(requestPacket);
                    const auto responsePacket = tcpClient.ReceivePacket();

                    if (ClientWorkflow::IsSuccessfulResponse(responsePacket))
                    {
                        std::cout << "\nResponse:\n"
                            << ClientWorkflow::FormatResponseForDisplay(responsePacket)
                            << std::endl;
                    }
                    else
                    {
                        std::cout << "\nError:\n"
                            << ClientWorkflow::GetPayloadText(responsePacket)
                            << std::endl;
                    }

                    PauseForReturnToMenu();
                    break;
                }

                case 2:
                {
                    // For the "Search by Part Name" option, prompt the user for part name text
                    PrintOperationHeader("Search by Part Name");

                    const std::string partName =
                        PromptForText("Enter part name text to search: ");

                    requestPacket = ClientWorkflow::CreateSearchByPartNamePacket(
                        nextPacketId++,
                        partName);

                    tcpClient.SendPacket(requestPacket);
                    const auto responsePacket = tcpClient.ReceivePacket();

                    if (ClientWorkflow::IsSuccessfulResponse(responsePacket))
                    {
                        std::cout << "\nResponse:\n"
                            << ClientWorkflow::FormatResponseForDisplay(responsePacket)
                            << std::endl;
                    }
                    else
                    {
                        std::cout << "\nError:\n"
                            << ClientWorkflow::GetPayloadText(responsePacket)
                            << std::endl;
                    }

                    PauseForReturnToMenu();
                    break;
                }

                case 3:
                {
                    // For the "Update Stock" option, prompt the user for a part number and new stock quantity
                    PrintOperationHeader("Update Stock");

                    const std::string partNumber =
                        PromptForText("Enter part number to update: ");
                    const std::uint32_t quantity = PromptForQuantity();

                    requestPacket = ClientWorkflow::CreateUpdateStockPacket(
                        nextPacketId++,
                        partNumber,
                        quantity);

                    tcpClient.SendPacket(requestPacket);
                    const auto responsePacket = tcpClient.ReceivePacket();

                    if (ClientWorkflow::IsSuccessfulResponse(responsePacket))
                    {
                        std::cout << "\nResponse:\n"
                            << ClientWorkflow::FormatResponseForDisplay(responsePacket)
                            << std::endl;
                    }
                    else
                    {
                        std::cout << "\nError:\n"
                            << ClientWorkflow::GetPayloadText(responsePacket)
                            << std::endl;
                    }

                    PauseForReturnToMenu();
                    break;
                }

                case 4:
                {
                    // For the "Request Inventory Catalog File" option, send a file request packet
                    // and save the received file to disk
                    PrintOperationHeader("Request Inventory Catalog File");

                    requestPacket = ClientWorkflow::CreateRequestFilePacket(nextPacketId++);

                    tcpClient.SendPacket(requestPacket);
                    const auto responsePacket = tcpClient.ReceivePacket();

                    if (!ClientWorkflow::IsSuccessfulResponse(responsePacket))
                    {
                        std::cout
                            << "\nFile request failed:\n"
                            << ClientWorkflow::GetPayloadText(responsePacket)
                            << std::endl;

                        PauseForReturnToMenu();
                        break;
                    }

                    const std::string outputFileName =
                        ClientFileManager::GetDefaultReceivedFileName();

                    ClientFileManager::SavePayloadToFile(
                        responsePacket.GetPayload(),
                        outputFileName);

                    std::cout
                        << "\nReceived file saved to: "
                        << outputFileName
                        << std::endl;

                    std::cout
                        << "Received file size: "
                        << responsePacket.GetPayload().size()
                        << " bytes"
                        << std::endl;

                    PauseForReturnToMenu();
                    break;
                }

                case 5:
                {
                    // For the "Disconnect and Exit" option, send a disconnect packet and end the session
                    PrintOperationHeader("Disconnect");

                    const auto disconnectPacket =
                        ClientWorkflow::CreateDisconnectPacket(nextPacketId++);
                    tcpClient.SendPacket(disconnectPacket);

                    const auto disconnectResponse = tcpClient.ReceivePacket();
                    if (ClientWorkflow::IsSuccessfulResponse(disconnectResponse))
                    {
                        std::cout
                            << "Disconnect response: "
                            << ClientWorkflow::GetPayloadText(disconnectResponse)
                            << std::endl;
                    }
                    else
                    {
                        std::cout << "Disconnect completed with a non-success response." << std::endl;
                    }

                    sessionActive = false;
                    continue;
                }

                default:
                    std::cout << "Invalid menu selection." << std::endl;
                    PauseForReturnToMenu();
                    continue;
                }
            }
            catch (const std::exception& exception)
            {
                std::cout << "\nOperation failed: " << exception.what() << std::endl;
                PauseForReturnToMenu();
            }
        }

        // After exiting the menu loop, ensure the client is properly disconnected
        // and clean up resources before ending the program
        tcpClient.Disconnect();
        ClearScreen();
        std::cout << "Client session completed." << std::endl;

        return 0;
    }
}