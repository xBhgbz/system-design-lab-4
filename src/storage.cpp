#include "storage.h"
#include <Poco/UUIDGenerator.h>
#include <Poco/Version.h>
#include <Poco/MongoDB/Connection.h>
#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/Cursor.h>
#include <Poco/MongoDB/QueryRequest.h>
#include <Poco/MongoDB/InsertRequest.h>
#include <Poco/MongoDB/DeleteRequest.h>
#include <Poco/MongoDB/UpdateRequest.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/Base64Encoder.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace Poco::MongoDB;

static std::string hashPassword(const std::string& password) {
    Poco::Crypto::DigestEngine engine("SHA256");
    engine.update(password.data(), password.size());
    const auto& digest = engine.digest();
    
    std::ostringstream oss;
    Poco::Base64Encoder encoder(oss);
    encoder.write(reinterpret_cast<const char*>(&digest[0]), digest.size());
    encoder.close();
    
    return oss.str();
}

static bool verifyPassword(const std::string& password, const std::string& hash) {
    std::string computedHash = hashPassword(password);
    return computedHash == hash;
}

// Helper function to safely get string from document
static std::string getStringFromDoc(const Document::Ptr& doc, const std::string& field) {
    if (!doc) return "";
    auto elem = doc->get(field);
    return elem ? elem->toString() : "";
}

// Helper function to safely get int from document
static int getIntFromDoc(const Document::Ptr& doc, const std::string& field) {
    if (!doc) return 0;
    auto elem = doc->get(field);
    if (elem) {
        try {
            return std::stoi(elem->toString());
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

// Helper function to safely get bool from document
static bool getBoolFromDoc(const Document::Ptr& doc, const std::string& field) {
    if (!doc) return false;
    auto elem = doc->get(field);
    if (elem) {
        std::string val = elem->toString();
        return val == "true" || val == "1";
    }
    return false;
}

Storage& Storage::instance() {
    static Storage s;
    return s;
}

Storage::Storage() {
    try {
        _connection = std::make_unique<Connection>();
        _connection->connect("mongodb", 27017);
        _database = std::make_unique<Database>("hotel_booking");
        std::cerr << "MongoDB connected successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "MongoDB connection error: " << e.what() << std::endl;
    }
}

int Storage::createUser(const std::string& login, const std::string& pass, const std::string& fn, const std::string& ln) {
    try {
        // Check if user exists
        QueryRequest query("hotel_booking.users");
        query.selector().add("login", login);
        
        ResponseMessage response;
        _connection->sendRequest(query, response);

        if (response.documents().size() > 0) {
            std::cout << "User already exists" << std::endl;
            return -1;
        }

        InsertRequest insert("hotel_booking.users");
        Document& doc = insert.addNewDocument();
        doc.add("login", login);
        doc.add("password", hashPassword(pass));
        doc.add("firstName", fn);
        doc.add("lastName", ln);
        doc.add("createdAt", Poco::Timestamp().epochTime());
        doc.add("updatedAt", Poco::Timestamp().epochTime());
        
        _connection->sendRequest(insert);
        
        return std::hash<std::string>{}(login) % INT_MAX;
    } catch (const std::exception& e) {
        std::cerr << "Error creating user: " << e.what() << std::endl;
        return -1;
    }
}

User* Storage::findUserByLogin(const std::string& login) {
    static User u;
    u.id = 0;
    
    try {
        QueryRequest query("hotel_booking.users");
        query.selector().add("login", login);
        
        ResponseMessage response;
        _connection->sendRequest(query, response);

        std::cout << "Finding user by login: " << login << std::endl;
        std::cout << "Documents found: " << response.documents().size() << std::endl;
        
        if (response.documents().empty()) {
            std::cout << "User not found: " << login << std::endl;
            return nullptr;
        }
        
        auto found = response.documents()[0];
        
        u.id = std::hash<std::string>{}(login) % INT_MAX;
        u.login = login;
        u.password = getStringFromDoc(found, "password");
        u.firstName = getStringFromDoc(found, "firstName");
        u.lastName = getStringFromDoc(found, "lastName");
        
        return &u;
    } catch (const std::exception& e) {
        std::cerr << "Error finding user: " << e.what() << std::endl;
        return nullptr;
    }
}

User* Storage::authenticateUser(const std::string& login, const std::string& password) {
    User* user = findUserByLogin(login);
    std::cout << "Authenticating user: " << login << std::endl;
    std::cout << "User found: " << (user ? "yes" : "no") << std::endl;
    if (!user) return nullptr;
    
    std::cout << "Verifying password for user: " << login << std::endl;
    std::cout << "Password hash in DB: " << user->password << std::endl;
    std::cout << "Computed hash for input password: " << hashPassword(password) << std::endl;

    if (verifyPassword(password, user->password)) {
        return user;
    }
    
    return nullptr;
}

std::vector<User> Storage::findUsersByMask(const std::string& mask) {
    std::vector<User> res;
    
    try {
        QueryRequest query("hotel_booking.users");
        
        ResponseMessage response;
        _connection->sendRequest(query, response);
        
        for (const auto& doc : response.documents()) {
            std::string firstName = getStringFromDoc(doc, "firstName");
            std::string lastName = getStringFromDoc(doc, "lastName");
            std::string login = getStringFromDoc(doc, "login");
            
            if (firstName.find(mask) != std::string::npos || 
                lastName.find(mask) != std::string::npos ||
                (firstName + " " + lastName).find(mask) != std::string::npos) {
                
                User u;
                u.id = std::hash<std::string>{}(login) % INT_MAX;
                u.login = login;
                u.password = getStringFromDoc(doc, "password");
                u.firstName = firstName;
                u.lastName = lastName;
                res.push_back(u);
            }
        }
        
        return res;
    } catch (const std::exception& e) {
        std::cerr << "Error finding users by mask: " << e.what() << std::endl;
        return res;
    }
}

User* Storage::getUserById(int id) {
    static User u;
    u.id = 0;
    return nullptr;
}

int Storage::createHotel(const std::string& name, const std::string& city, int stars) {
    try {
        InsertRequest insert("hotel_booking.hotels");
        Document& doc = insert.addNewDocument();
        doc.add("name", name);
        doc.add("city", city);
        doc.add("stars", stars);
        doc.add("createdAt", Poco::Timestamp().epochTime());
        doc.add("updatedAt", Poco::Timestamp().epochTime());
        
        _connection->sendRequest(insert);
        
        return std::hash<std::string>{}(name) % INT_MAX;
    } catch (const std::exception& e) {
        std::cerr << "Error creating hotel: " << e.what() << std::endl;
        return -1;
    }
}

std::vector<Hotel> Storage::getAllHotels() {
    std::vector<Hotel> res;
    
    try {
        QueryRequest query("hotel_booking.hotels");
        
        ResponseMessage response;
        _connection->sendRequest(query, response);
        
        for (const auto& doc : response.documents()) {
            Hotel h;
            
            std::string name = getStringFromDoc(doc, "name");
            h.id = std::hash<std::string>{}(name) % INT_MAX;
            h.name = name;
            h.city = getStringFromDoc(doc, "city");
            h.stars = getIntFromDoc(doc, "stars");
            
            res.push_back(h);
        }
        
        return res;
    } catch (const std::exception& e) {
        std::cerr << "Error getting all hotels: " << e.what() << std::endl;
        return res;
    }
}

std::vector<Hotel> Storage::findHotelsByCity(const std::string& city) {
    std::vector<Hotel> res;
    
    try {
        QueryRequest query("hotel_booking.hotels");
        query.selector().add("city", city);
        
        ResponseMessage response;
        _connection->sendRequest(query, response);
        
        for (const auto& doc : response.documents()) {
            Hotel h;
            
            std::string name = getStringFromDoc(doc, "name");
            h.id = std::hash<std::string>{}(name) % INT_MAX;
            h.name = name;
            h.city = city;
            h.stars = getIntFromDoc(doc, "stars");
            
            res.push_back(h);
        }
        
        return res;
    } catch (const std::exception& e) {
        std::cerr << "Error finding hotels by city: " << e.what() << std::endl;
        return res;
    }
}

int Storage::createBooking(int userId, int hotelId, const std::string& start, const std::string& end) {
    try {
        InsertRequest insert("hotel_booking.bookings");
        Document& doc = insert.addNewDocument();
        doc.add("userId", userId);
        doc.add("hotelId", hotelId);
        doc.add("startDate", start);
        doc.add("endDate", end);
        doc.add("cancelled", false);
        doc.add("createdAt", Poco::Timestamp().epochTime());
        doc.add("updatedAt", Poco::Timestamp().epochTime());
        
        _connection->sendRequest(insert);
        
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error creating booking: " << e.what() << std::endl;
        return -1;
    }
}

std::vector<Booking> Storage::getUserBookings(int userId) {
    std::vector<Booking> res;
    
    try {
        QueryRequest query("hotel_booking.bookings");
        query.selector().add("userId", userId);
        
        ResponseMessage response;
        _connection->sendRequest(query, response);
        
        for (const auto& doc : response.documents()) {
            Booking b;
            
            // Generate ID from the document's _id
            std::string idStr = getStringFromDoc(doc, "_id");
            b.id = idStr.empty() ? 0 : std::hash<std::string>{}(idStr) % INT_MAX;
            b.userId = userId;
            b.hotelId = getIntFromDoc(doc, "hotelId");
            b.startDate = getStringFromDoc(doc, "startDate");
            b.endDate = getStringFromDoc(doc, "endDate");
            b.cancelled = getBoolFromDoc(doc, "cancelled");
            
            res.push_back(b);
        }
        
        return res;
    } catch (const std::exception& e) {
        std::cerr << "Error getting user bookings: " << e.what() << std::endl;
        return res;
    }
}

bool Storage::cancelBooking(int bookingId) {
    try {
        // This needs proper implementation with booking ID lookup
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error cancelling booking: " << e.what() << std::endl;
        return false;
    }
}

std::string Storage::createSession(int userId) {
    try {
        Poco::UUID uuid = Poco::UUIDGenerator::defaultGenerator().createRandom();
        std::string token = uuid.toString();
        
        InsertRequest insert("hotel_booking.sessions");
        Document& doc = insert.addNewDocument();
        doc.add("_id", token);
        doc.add("userId", userId);
        doc.add("createdAt", Poco::Timestamp().epochTime());
        
        _connection->sendRequest(insert);
        
        return token;
    } catch (const std::exception& e) {
        std::cerr << "Error creating session: " << e.what() << std::endl;
        return "";
    }
}

int Storage::validateSession(const std::string& token) {
    try {
        QueryRequest query("hotel_booking.sessions");
        query.selector().add("_id", token);
        
        ResponseMessage response;
        _connection->sendRequest(query, response);
        
        if (response.documents().empty()) {
            return -1;
        }
        
        auto found = response.documents()[0];
        return getIntFromDoc(found, "userId");
    } catch (const std::exception& e) {
        std::cerr << "Error validating session: " << e.what() << std::endl;
        return -1;
    }
}

void Storage::destroySession(const std::string& token) {
    try {
        DeleteRequest remove("hotel_booking.sessions");
        remove.selector().add("_id", token);
        
        ResponseMessage response;
        _connection->sendRequest(remove, response);
    } catch (const std::exception& e) {
        std::cerr << "Error destroying session: " << e.what() << std::endl;
    }
}