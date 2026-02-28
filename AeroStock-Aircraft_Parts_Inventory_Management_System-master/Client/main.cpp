#include "ClientApp.h"

#include <exception>
#include <iostream>

// Start the client app and handle fatal exceptions in one place.
int main()
{
    try
    {
        AeroStock::Client::ClientApp app;
        return app.Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Fatal client error: " << exception.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Fatal client error: unknown exception." << std::endl;
        return 1;
    }
}