#include "TcpServer.h"
#include "ClientSession.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace AeroStock::Server
{
    TcpServer::TcpServer(const std::uint16_t port)
        : port_{ port }
    {
    }

    // closes the socket and cleans up winsock
    TcpServer::~TcpServer()
    {
        if (listenSocket_ != INVALID_SOCKET)
        {
            closesocket(listenSocket_);
            listenSocket_ = INVALID_SOCKET;
        }

        if (winsockInitialized_)
        {
            WSACleanup();
            winsockInitialized_ = false;
        }
    }

    void TcpServer::Start()
    {
        InitializeWinsock();
        CreateAndBindSocket();
        BeginListening();
    }

    // accepts one client and then closes the connection once the client is done using it
    void TcpServer::AcceptAndProcessSingleClient()
    {
        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);

        const SOCKET clientSocket = accept(
            listenSocket_,
            reinterpret_cast<sockaddr*>(&clientAddr),
            &clientAddrLen);

        if (clientSocket == INVALID_SOCKET)
        {
            throw std::runtime_error(
                "Accept failed: " + std::to_string(WSAGetLastError()));
        }

        char addrStr[INET_ADDRSTRLEN]{};
        inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, sizeof(addrStr));

        std::cout
            << "Client connected from "
            << addrStr << ":" << ntohs(clientAddr.sin_port)
            << std::endl;

        try
        {
            ClientSession session(clientSocket);
            session.ProcessSession();
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Session error: " << ex.what() << std::endl;
        }

        closesocket(clientSocket);
        std::cout << "Client disconnected." << std::endl;
    }

    void TcpServer::InitializeWinsock()
    {
        WSADATA wsaData{};
        const int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

        if (result != 0)
        {
            throw std::runtime_error(
                "WSAStartup failed: " + std::to_string(result));
        }

        winsockInitialized_ = true;
    }

    // binds to all devices on the port 
    void TcpServer::CreateAndBindSocket()
    {
        listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (listenSocket_ == INVALID_SOCKET)
        {
            throw std::runtime_error(
                "Socket creation failed: " + std::to_string(WSAGetLastError()));
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port_);

        const int bindResult = bind(
            listenSocket_,
            reinterpret_cast<sockaddr*>(&serverAddr),
            sizeof(serverAddr));

        if (bindResult == SOCKET_ERROR)
        {
            throw std::runtime_error(
                "Bind failed: " + std::to_string(WSAGetLastError()));
        }
    }

    void TcpServer::BeginListening()
    {
        const int listenResult = listen(listenSocket_, SOMAXCONN);

        if (listenResult == SOCKET_ERROR)
        {
            throw std::runtime_error(
                "Listen failed: " + std::to_string(WSAGetLastError()));
        }
    }
}