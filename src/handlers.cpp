#include "handlers.h"
#include "storage.h"
#include "auth.h"
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/StreamCopier.h>
#include <Poco/Dynamic/Var.h>  
#include <Poco/URI.h>
#include <sstream>

using namespace Poco::Net;
using namespace Poco::JSON;

void sendJSON(HTTPServerResponse& response, HTTPResponse::HTTPStatus status, const Poco::Dynamic::Var& obj) {
    response.setStatusAndReason(status);
    response.setContentType("application/json");
    response.setChunkedTransferEncoding(true);
    std::ostream& out = response.send();
    Poco::JSON::Stringifier::stringify(obj, out);
}

void sendError(HTTPServerResponse& response, HTTPResponse::HTTPStatus status, const std::string& msg) {
    Object::Ptr err = new Object();
    err->set("error", msg);
    sendJSON(response, status, err);
}

Object::Ptr parseBody(HTTPServerRequest& request) {
    std::istream& in = request.stream();
    std::stringstream buffer;
    Poco::StreamCopier::copyStream(in, buffer);
    Parser parser;
    return parser.parse(buffer.str()).extract<Object::Ptr>();
}

std::string getQuery(const HTTPServerRequest& request, std::string key) {
    Poco::URI uri(request.getURI());
    Poco::URI::QueryParameters params = uri.getQueryParameters();

    std::string paramValue = "";
    for (const auto& param : params){
        if (param.first == key){
            paramValue = param.second;
            break;
        }
    }

    return paramValue;
}

void RegisterHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    if (request.getMethod() != HTTPRequest::HTTP_POST) {
        sendError(response, HTTPResponse::HTTP_METHOD_NOT_ALLOWED, "Only POST allowed");
        return;
    }
    try {
        Object::Ptr json = parseBody(request);
        std::string login = json->getValue<std::string>("login");
        std::string pass = json->getValue<std::string>("password");
        std::string fn = json->getValue<std::string>("firstName");
        std::string ln = json->getValue<std::string>("lastName");

        int id = Storage::instance().createUser(login, pass, fn, ln);
        if (id == -1) {
            sendError(response, HTTPResponse::HTTP_CONFLICT, "User exists");
            return;
        }
        Object::Ptr res = new Object();
        res->set("id", id);
        res->set("login", login);
        sendJSON(response, HTTPResponse::HTTP_CREATED, res);
    } catch (...) {
        sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "Invalid JSON");
    }
}

void LoginHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    if (request.getMethod() != HTTPRequest::HTTP_POST) {
        sendError(response, HTTPResponse::HTTP_METHOD_NOT_ALLOWED, "Only POST allowed");
        return;
    }
    try {
        Object::Ptr json = parseBody(request);
        std::string login = json->getValue<std::string>("login");
        std::string pass = json->getValue<std::string>("password");

        User* u = Storage::instance().authenticateUser(login, pass);
        if (!u) {
            sendError(response, HTTPResponse::HTTP_UNAUTHORIZED, "Invalid credentials");
            return;
        }

        std::string token = Storage::instance().createSession(u->id);
        Object::Ptr res = new Object();
        res->set("token", token);
        sendJSON(response, HTTPResponse::HTTP_OK, res);
    } catch (...) {
        sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "Invalid JSON");
    }
}

void UsersHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    // Поиск по логину или маске
    std::string login = getQuery(request, "login");
    std::string mask = getQuery(request, "name_mask");

    if (!login.empty()) {
        User* u = Storage::instance().findUserByLogin(login);
        if (!u) {
            sendError(response, HTTPResponse::HTTP_NOT_FOUND, "User not found");
            return;
        }
        Object::Ptr res = new Object();
        res->set("id", u->id);
        res->set("login", u->login);
        res->set("firstName", u->firstName);
        res->set("lastName", u->lastName);
        sendJSON(response, HTTPResponse::HTTP_OK, res);
    } else if (!mask.empty()) {
        std::vector<User> users = Storage::instance().findUsersByMask(mask);
        Array::Ptr arr = new Array();
        for (const auto& u : users) {
            Object::Ptr obj = new Object();
            obj->set("id", u.id);
            obj->set("login", u.login);
            obj->set("firstName", u.firstName);
            obj->set("lastName", u.lastName);
            arr->add(obj);
        }
        sendJSON(response, HTTPResponse::HTTP_OK, arr);
    } else {
        sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "Provide login or name_mask");
    }
}

void HotelsHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    response.set("Access-Control-Allow-Origin", "*");
    
    if (request.getMethod() == HTTPRequest::HTTP_OPTIONS) {
        response.setStatus(HTTPResponse::HTTP_OK);
        response.send();
        return;
    }

    if (request.getMethod() == HTTPRequest::HTTP_POST) {
        int userId = 0;
        if (!AuthHelper::isAuthenticated(request, userId)) {
            sendError(response, HTTPResponse::HTTP_UNAUTHORIZED, "Auth required");
            return;
        }
        try {
            Object::Ptr json = parseBody(request);
            std::string name = json->getValue<std::string>("name");
            std::string city = json->getValue<std::string>("city");
            int stars = json->getValue<int>("stars");
            int id = Storage::instance().createHotel(name, city, stars);
            Object::Ptr res = new Object();
            res->set("id", id);
            sendJSON(response, HTTPResponse::HTTP_CREATED, res);
        } catch (...) {
            sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "Invalid JSON");
        }
    } else if (request.getMethod() == HTTPRequest::HTTP_GET) {
        std::string city = getQuery(request, "city");
        std::vector<Hotel> hotels;
        if (!city.empty()) {
            hotels = Storage::instance().findHotelsByCity(city);
        } else {
            hotels = Storage::instance().getAllHotels();
        }
        Array::Ptr arr = new Array();
        for (const auto& h : hotels) {
            Object::Ptr obj = new Object();
            obj->set("id", h.id);
            obj->set("name", h.name);
            obj->set("city", h.city);
            obj->set("stars", h.stars);
            arr->add(obj);
        }
        sendJSON(response, HTTPResponse::HTTP_OK, arr);
    } else {
        sendError(response, HTTPResponse::HTTP_METHOD_NOT_ALLOWED, "GET or POST only");
    }
}

void BookingsHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    response.set("Access-Control-Allow-Origin", "*");
    if (request.getMethod() == HTTPRequest::HTTP_OPTIONS) {
        response.setStatus(HTTPResponse::HTTP_OK);
        response.send();
        return;
    }

    int userId = 0;
    if (!AuthHelper::isAuthenticated(request, userId)) {
        sendError(response, HTTPResponse::HTTP_UNAUTHORIZED, "Auth required");
        return;
    }

    if (request.getMethod() == HTTPRequest::HTTP_POST) {
        try {
            Object::Ptr json = parseBody(request);
            int hotelId = json->getValue<int>("hotelId");
            std::string start = json->getValue<std::string>("startDate");
            std::string end = json->getValue<std::string>("endDate");
            std::string id = Storage::instance().createBooking(userId, hotelId, start, end);
            if (id.empty()) {
                sendError(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, "Failed to create booking");
                return;
            }
            Object::Ptr res = new Object();
            res->set("id", id);
            sendJSON(response, HTTPResponse::HTTP_CREATED, res);
        } catch (...) {
            sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "Invalid JSON");
        }
    } else if (request.getMethod() == HTTPRequest::HTTP_GET) {
        std::vector<Booking> bookings = Storage::instance().getUserBookings(userId);
        Array::Ptr arr = new Array();
        for (const auto& b : bookings) {
            Object::Ptr obj = new Object();
            obj->set("id", b.id);
            obj->set("hotelId", b.hotelId);
            obj->set("startDate", b.startDate);
            obj->set("endDate", b.endDate);
            obj->set("cancelled", b.cancelled);
            arr->add(obj);
        }
        sendJSON(response, HTTPResponse::HTTP_OK, arr);
    } else if (request.getMethod() == HTTPRequest::HTTP_DELETE) {
        std::string uri = request.getURI();
        size_t pos = uri.find_last_of('/');
        if (pos != std::string::npos && pos < uri.length() - 1) {
            std::string bookingId = uri.substr(pos + 1);
            try {
                if (Storage::instance().cancelBooking(bookingId)) {
                    sendJSON(response, HTTPResponse::HTTP_NO_CONTENT, new Object());
                } else {
                    sendError(response, HTTPResponse::HTTP_NOT_FOUND, "Booking not found");
                }
            } catch (...) {
                sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "Invalid booking ID");
            }
        } else {
             sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "ID required");
        }
    } else {
        sendError(response, HTTPResponse::HTTP_METHOD_NOT_ALLOWED, "GET, POST, DELETE");
    }
}

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request) {
    std::string uri = request.getURI();
    std::string method = request.getMethod();

    if (uri.find("/api/users/register") == 0) return new RegisterHandler();
    if (uri.find("/api/users/login") == 0) return new LoginHandler();
    if (uri.find("/api/users") == 0) return new UsersHandler();
    if (uri.find("/api/hotels") == 0) return new HotelsHandler();
    if (uri.find("/api/bookings") == 0) return new BookingsHandler();
    
    return nullptr;
}