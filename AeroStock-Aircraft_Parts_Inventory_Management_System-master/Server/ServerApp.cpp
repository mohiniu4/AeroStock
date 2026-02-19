#include "ServerApp.h"

#include <iostream>
#include <memory>

#include "../Common/ProtocolConstants.h"
#include "TcpServer.h"

namespace AeroStock::Server
{
    ServerApp::ServerApp()
        : tcpServer_{ std::make_unique<TcpServer>(AeroStock::Common::ProtocolConstants::DefaultPort) }
    {
    }

    ServerApp::~ServerApp() = default;

    int ServerApp::Run()
    {
        PrintStartupBanner();

        std::cout
            << "Server is starting on port "
            << AeroStock::Common::ProtocolConstants::DefaultPort
            << "..."
            << std::endl;

        tcpServer_->Start();

        std::cout << "Waiting for a client connection..." << std::endl;
        tcpServer_->AcceptAndProcessSingleClient();

        std::cout << "Server run completed." << std::endl;

        return 0;
    }

    void ServerApp::PrintStartupBanner() const
    {
        std::cout << "========================================" << std::endl;
        std::cout << " AeroStock Server" << std::endl;
        std::cout << " Aircraft Parts Inventory Management" << std::endl;
        std::cout << "========================================" << std::endl;
    }
}