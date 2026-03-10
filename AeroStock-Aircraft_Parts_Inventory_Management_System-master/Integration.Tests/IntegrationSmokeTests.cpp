#include "pch.h"
#include "CppUnitTest.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#include <WS2tcpip.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "ClientWorkflow.h"
#include "Packet.h"
#include "PacketHeader.h"
#include "PacketSerializer.h"
#include "PayloadUtils.h"
#include "ProtocolConstants.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace IntegrationTests
{
    namespace
    {
        class WinsockScope final
        {
        public:
            WinsockScope()
            {
                WSADATA winsockData{};
                const int result = WSAStartup(MAKEWORD(2, 2), &winsockData);
                if (result != 0)
                {
                    throw std::runtime_error("WSAStartup failed.");
                }
            }

            ~WinsockScope()
            {
                WSACleanup();
            }

            WinsockScope(const WinsockScope&) = delete;
            WinsockScope& operator=(const WinsockScope&) = delete;
        };

        class TestTcpClient final
        {
        public:
            TestTcpClient() = default;

            ~TestTcpClient()
            {
                Disconnect();
            }

            void Connect(const std::string& ipAddress, const std::uint16_t port)
            {
                if (socket_ != INVALID_SOCKET)
                {
                    throw std::runtime_error("Socket already connected.");
                }

                socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (socket_ == INVALID_SOCKET)
                {
                    throw std::runtime_error("Failed to create socket.");
                }

                sockaddr_in serverAddress{};
                serverAddress.sin_family = AF_INET;
                serverAddress.sin_port = htons(port);

                const int addressResult =
                    inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr);

                if (addressResult != 1)
                {
                    throw std::runtime_error("Invalid server IP address.");
                }

                const int connectResult = connect(
                    socket_,
                    reinterpret_cast<const sockaddr*>(&serverAddress),
                    sizeof(serverAddress));

                if (connectResult == SOCKET_ERROR)
                {
                    throw std::runtime_error("Failed to connect to server.");
                }
            }

            void Disconnect() noexcept
            {
                if (socket_ != INVALID_SOCKET)
                {
                    closesocket(socket_);
                    socket_ = INVALID_SOCKET;
                }
            }

            void SendPacket(const AeroStock::Common::Packet& packet)
            {
                const auto serializedPacket =
                    AeroStock::Common::PacketSerializer::Serialize(packet);

                if (serializedPacket.empty())
                {
                    throw std::runtime_error("Failed to serialize packet.");
                }

                SendAll(
                    serializedPacket.data(),
                    static_cast<int>(serializedPacket.size()));
            }

            [[nodiscard]] AeroStock::Common::Packet ReceivePacket()
            {
                using ByteBuffer = AeroStock::Common::PacketSerializer::ByteBuffer;

                ByteBuffer headerBuffer(
                    AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes,
                    0U);

                ReceiveAll(headerBuffer.data(), static_cast<int>(headerBuffer.size()));

                AeroStock::Common::PacketHeader header{};
                const bool headerOk =
                    AeroStock::Common::PacketSerializer::DeserializeHeader(
                        headerBuffer,
                        header);

                if (!headerOk)
                {
                    throw std::runtime_error("Failed to deserialize packet header.");
                }

                const std::size_t payloadLength =
                    static_cast<std::size_t>(header.payloadLength);

                ByteBuffer fullPacketBuffer{};
                fullPacketBuffer.reserve(
                    AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes +
                    payloadLength);

                fullPacketBuffer.insert(
                    fullPacketBuffer.end(),
                    headerBuffer.begin(),
                    headerBuffer.end());

                if (payloadLength > 0U)
                {
                    ByteBuffer payloadBuffer(payloadLength, 0U);
                    ReceiveAll(payloadBuffer.data(), static_cast<int>(payloadBuffer.size()));

                    fullPacketBuffer.insert(
                        fullPacketBuffer.end(),
                        payloadBuffer.begin(),
                        payloadBuffer.end());
                }

                AeroStock::Common::Packet packet{};
                const bool packetOk =
                    AeroStock::Common::PacketSerializer::Deserialize(
                        fullPacketBuffer,
                        packet);

                if (!packetOk)
                {
                    throw std::runtime_error("Failed to deserialize full packet.");
                }

                return packet;
            }

        private:
            void SendAll(const void* buffer, const int byteCount)
            {
                const auto* current = static_cast<const char*>(buffer);
                int totalSent = 0;

                while (totalSent < byteCount)
                {
                    const int sent = send(
                        socket_,
                        current + totalSent,
                        byteCount - totalSent,
                        0);

                    if (sent == SOCKET_ERROR)
                    {
                        throw std::runtime_error("Socket send failed.");
                    }

                    totalSent += sent;
                }
            }

            void ReceiveAll(void* buffer, const int byteCount)
            {
                auto* current = static_cast<char*>(buffer);
                int totalReceived = 0;

                while (totalReceived < byteCount)
                {
                    const int received = recv(
                        socket_,
                        current + totalReceived,
                        byteCount - totalReceived,
                        0);

                    if (received <= 0)
                    {
                        throw std::runtime_error("Socket receive failed.");
                    }

                    totalReceived += received;
                }
            }

        private:
            SOCKET socket_{ INVALID_SOCKET };
        };

        class ServerProcessHarness final
        {
        public:
            ServerProcessHarness() = default;

            ~ServerProcessHarness()
            {
                Stop();
            }

            void Start()
            {
                const std::filesystem::path serverExePath = FindServerExecutable();

                STARTUPINFOA startupInfo{};
                startupInfo.cb = sizeof(startupInfo);

                PROCESS_INFORMATION processInfo{};
                std::string commandLine = "\"" + serverExePath.string() + "\"";

                const BOOL created = CreateProcessA(
                    nullptr,
                    commandLine.data(),
                    nullptr,
                    nullptr,
                    FALSE,
                    CREATE_NO_WINDOW,
                    nullptr,
                    serverExePath.parent_path().string().c_str(),
                    &startupInfo,
                    &processInfo);

                if (!created)
                {
                    throw std::runtime_error("Failed to launch Server.exe.");
                }

                processHandle_ = processInfo.hProcess;
                threadHandle_ = processInfo.hThread;

                std::this_thread::sleep_for(std::chrono::seconds(2));
            }

            void Stop() noexcept
            {
                if (threadHandle_ != nullptr)
                {
                    CloseHandle(threadHandle_);
                    threadHandle_ = nullptr;
                }

                if (processHandle_ != nullptr)
                {
                    DWORD exitCode = 0;
                    if (GetExitCodeProcess(processHandle_, &exitCode) &&
                        exitCode == STILL_ACTIVE)
                    {
                        TerminateProcess(processHandle_, 0);
                        WaitForSingleObject(processHandle_, 2000);
                    }

                    CloseHandle(processHandle_);
                    processHandle_ = nullptr;
                }
            }

        private:
            [[nodiscard]] static std::filesystem::path FindServerExecutable()
            {
                const std::filesystem::path currentDir = std::filesystem::current_path();

                for (auto dir = currentDir; !dir.empty(); dir = dir.parent_path())
                {
                    const auto candidate = dir / "Server.exe";
                    if (std::filesystem::exists(candidate))
                    {
                        return candidate;
                    }

                    for (const auto& entry :
                        std::filesystem::directory_iterator(dir))
                    {
                        if (!entry.is_directory())
                        {
                            continue;
                        }

                        const auto nestedCandidate = entry.path() / "Server.exe";
                        if (std::filesystem::exists(nestedCandidate))
                        {
                            return nestedCandidate;
                        }
                    }

                    if (dir == dir.root_path())
                    {
                        break;
                    }
                }

                throw std::runtime_error("Could not locate Server.exe.");
            }

        private:
            HANDLE processHandle_{ nullptr };
            HANDLE threadHandle_{ nullptr };
        };

        [[nodiscard]] std::string PayloadToString(
            const AeroStock::Common::Packet& packet)
        {
            return AeroStock::Common::PayloadUtils::PayloadToString(packet.GetPayload());
        }
    }

    TEST_CLASS(IntegrationSmokeTests)
    {
    public:
        TEST_METHOD(ConnectVerifyDisconnect_EndToEnd_Succeeds)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            auto connectPacket =
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(1U);
            client.SendPacket(connectPacket);

            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));
            Assert::AreEqual("CONNECTED", PayloadToString(connectResponse).c_str());

            auto verifyPacket =
                AeroStock::Client::ClientWorkflow::CreateVerifyPacket(2U);
            client.SendPacket(verifyPacket);

            const auto verifyResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(verifyResponse));
            Assert::AreEqual(
                AeroStock::Common::ProtocolConstants::VerificationSuccessText,
                PayloadToString(verifyResponse).c_str());

            auto disconnectPacket =
                AeroStock::Client::ClientWorkflow::CreateDisconnectPacket(3U);
            client.SendPacket(disconnectPacket);

            const auto disconnectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(disconnectResponse));
            Assert::AreEqual("DISCONNECTED", PayloadToString(disconnectResponse).c_str());

            client.Disconnect();
        }

        TEST_METHOD(RequestFile_EndToEnd_ReturnsLargePayload)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(10U));
            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateVerifyPacket(11U));
            const auto verifyResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(verifyResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateRequestFilePacket(12U));
            const auto fileResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(fileResponse));
            Assert::IsTrue(fileResponse.HasPayload());
            Assert::IsTrue(fileResponse.GetPayload().size() >= (1024U * 1024U));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateDisconnectPacket(13U));
            const auto disconnectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(disconnectResponse));

            client.Disconnect();
        }

        TEST_METHOD(SearchByPartNumber_EndToEnd_ReturnsExpectedRecord)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(20U));
            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateVerifyPacket(21U));
            const auto verifyResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(verifyResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateSearchByPartNumberPacket(
                    22U,
                    "PN-1001"));
            const auto searchResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(searchResponse));

            const auto payloadText = PayloadToString(searchResponse);
            Assert::IsTrue(payloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Hydraulic Pump") != std::string::npos);
            Assert::IsTrue(payloadText.find("Stock=25") != std::string::npos);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateDisconnectPacket(23U));
            const auto disconnectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(disconnectResponse));

            client.Disconnect();
        }

        TEST_METHOD(SearchByPartName_EndToEnd_ReturnsMultipleResults)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(30U));
            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateVerifyPacket(31U));
            const auto verifyResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(verifyResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateSearchByPartNamePacket(
                    32U,
                    "Hydraulic"));
            const auto searchResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(searchResponse));

            const auto payloadText = PayloadToString(searchResponse);
            Assert::IsTrue(payloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartNumber=PN-1013") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Hydraulic Pump") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Hydraulic Filter Element") != std::string::npos);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateDisconnectPacket(33U));
            const auto disconnectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(disconnectResponse));

            client.Disconnect();
        }

        TEST_METHOD(GetPartDetails_EndToEnd_ReturnsExpectedRecord)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(40U));
            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateVerifyPacket(41U));
            const auto verifyResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(verifyResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateGetPartDetailsPacket(
                    42U,
                    "PN-1003"));
            const auto detailsResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(detailsResponse));

            const auto payloadText = PayloadToString(detailsResponse);
            Assert::IsTrue(payloadText.find("PartNumber=PN-1003") != std::string::npos);
            Assert::IsTrue(payloadText.find("PartName=Avionics Display Unit") != std::string::npos);
            Assert::IsTrue(payloadText.find("Stock=8") != std::string::npos);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateDisconnectPacket(43U));
            const auto disconnectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(disconnectResponse));

            client.Disconnect();
        }

        TEST_METHOD(InvalidCommandBeforeVerify_ReturnsStructuredError)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(50U));
            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateSearchByPartNumberPacket(
                    51U,
                    "PN-1001"));
            const auto invalidResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsErrorResponse(invalidResponse));

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::InvalidRequest),
                static_cast<int>(invalidResponse.GetStatus()));

            const auto payloadText = PayloadToString(invalidResponse);
            Assert::IsTrue(
                payloadText.find("REQUEST_NOT_ALLOWED_IN_CURRENT_STATE") != std::string::npos);

            client.Disconnect();
        }

        TEST_METHOD(UpdateStock_EndToEnd_UpdatesAndPersistsWithinSession)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(60U));
            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateVerifyPacket(61U));
            const auto verifyResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(verifyResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateUpdateStockPacket(
                    62U,
                    "PN-1001",
                    99U));
            const auto updateResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(updateResponse));

            const auto updatePayloadText = PayloadToString(updateResponse);
            Assert::IsTrue(updatePayloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(updatePayloadText.find("Stock=99") != std::string::npos);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateGetPartDetailsPacket(
                    63U,
                    "PN-1001"));
            const auto detailsResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(detailsResponse));

            const auto detailsPayloadText = PayloadToString(detailsResponse);
            Assert::IsTrue(detailsPayloadText.find("PartNumber=PN-1001") != std::string::npos);
            Assert::IsTrue(detailsPayloadText.find("Stock=99") != std::string::npos);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateDisconnectPacket(64U));
            const auto disconnectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(disconnectResponse));

            client.Disconnect();
        }

        TEST_METHOD(UpdateStock_UnknownPart_ReturnsStructuredError)
        {
            WinsockScope winsock{};
            ServerProcessHarness server{};
            server.Start();

            TestTcpClient client{};
            client.Connect("127.0.0.1", 54000U);

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateConnectPacket(70U));
            const auto connectResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(connectResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateVerifyPacket(71U));
            const auto verifyResponse = client.ReceivePacket();
            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsSuccessfulResponse(verifyResponse));

            client.SendPacket(
                AeroStock::Client::ClientWorkflow::CreateUpdateStockPacket(
                    72U,
                    "PN-9999",
                    50U));
            const auto updateResponse = client.ReceivePacket();

            Assert::IsTrue(
                AeroStock::Client::ClientWorkflow::IsErrorResponse(updateResponse));

            Assert::AreEqual(
                static_cast<int>(AeroStock::Common::StatusCode::NotFound),
                static_cast<int>(updateResponse.GetStatus()));

            const auto payloadText = PayloadToString(updateResponse);
            Assert::IsTrue(payloadText.find("PART_NOT_FOUND") != std::string::npos);

            client.Disconnect();
        }
    };
}