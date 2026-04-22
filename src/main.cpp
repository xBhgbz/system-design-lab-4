#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Util/ServerApplication.h>
#include "handlers.h"
#include <iostream>

class HotelService : public Poco::Util::ServerApplication {
public:
    int main(const std::vector<std::string>& args) {
        Poco::Net::ServerSocket socket(8080);
        Poco::Net::HTTPServerParams::Ptr params = new Poco::Net::HTTPServerParams();
        params->setMaxQueued(100);
        params->setMaxThreads(4);
        
        Poco::Net::HTTPServer server(new RequestHandlerFactory(), socket, params);
        server.start();
        std::cout << "Server started on port 8080" << std::endl;
        waitForTerminationRequest();
        server.stopAll();
        return Application::EXIT_OK;
    }
};

POCO_SERVER_MAIN(HotelService)