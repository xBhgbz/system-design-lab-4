#pragma once
#include <Poco/Net/HTTPServerRequest.h>
#include <string>

class AuthHelper {
public:
    static std::string getTokenFromRequest(const Poco::Net::HTTPServerRequest& request);
    static bool isAuthenticated(const Poco::Net::HTTPServerRequest& request, int& userId);
};