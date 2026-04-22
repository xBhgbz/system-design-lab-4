#include "auth.h"
#include "storage.h"

std::string AuthHelper::getTokenFromRequest(const Poco::Net::HTTPServerRequest& request) {
    const std::string& auth = request.get("Authorization", "");
    if (auth.substr(0, 7) == "Bearer ") {
        return auth.substr(7);
    }
    return "";
}

bool AuthHelper::isAuthenticated(const Poco::Net::HTTPServerRequest& request, int& userId) {
    std::string token = getTokenFromRequest(request);
    if (token.empty()) return false;
    userId = Storage::instance().validateSession(token);
    return userId != -1;
}