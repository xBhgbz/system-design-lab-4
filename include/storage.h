#pragma once

#include <Poco/MongoDB/MongoDB.h>
#include <Poco/MongoDB/Connection.h>
#include <Poco/MongoDB/Database.h>
#include <Poco/MongoDB/Cursor.h>
#include <Poco/Dynamic/Var.h>
#include <memory>
#include <vector>
#include <map>
#include <string>

struct User {
    int id;
    std::string login;
    std::string password;
    std::string firstName;
    std::string lastName;
};

struct Hotel {
    int id;
    std::string name;
    std::string city;
    int stars;
};

struct Booking {
    std::string id;
    int userId;
    int hotelId;
    std::string startDate;
    std::string endDate;
    bool cancelled;
};

class Storage {
public:
    static Storage& instance();

    Storage(); 

    int createUser(const std::string& login, const std::string& pass, const std::string& fn, const std::string& ln);

    User* findUserByLogin(const std::string& login);
    User* authenticateUser(const std::string& login, const std::string& password);
    std::vector<User> findUsersByMask(const std::string& mask);
    User* getUserById(int id);

    int createHotel(const std::string& name, const std::string& city, int stars);
    std::vector<Hotel> getAllHotels();
    std::vector<Hotel> findHotelsByCity(const std::string& city);

    std::string createBooking(int userId, int hotelId, const std::string& start, const std::string& end);

    std::vector<Booking> getUserBookings(int userId);
    bool cancelBooking(const std::string& bookingId);

    std::string createSession(int userId);
    int validateSession(const std::string& token);
    void destroySession(const std::string& token);

private:
    std::unique_ptr<Poco::MongoDB::Connection> _connection;
    std::unique_ptr<Poco::MongoDB::Database> _database;
};