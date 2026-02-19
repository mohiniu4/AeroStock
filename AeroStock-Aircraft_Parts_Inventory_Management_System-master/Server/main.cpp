#include "ServerApp.h"

#include <exception>
#include <iostream>

// Entry point for the AeroStock server application.
// The main function creates the top-level server app object and
// handles any fatal exceptions that escape the normal runtime flow.
int main()
{
    try
    {
        AeroStock::Server::ServerApp app;
        return app.Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Fatal server error: " << exception.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Fatal server error: unknown exception." << std::endl;
        return 1;
    }
}