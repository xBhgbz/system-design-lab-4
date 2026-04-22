cmake_minimum_required(VERSION 3.10)
project(HotelBookingAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Poco REQUIRED COMPONENTS
    Foundation
    Net
    Util
    JSON
    Crypto
    MongoDB
)

add_executable(api
    src/main.cpp
    src/storage.cpp
    src/handlers.cpp
    src/auth.cpp
)

target_link_libraries(api PRIVATE
    Poco::Foundation
    Poco::Net
    Poco::Util
    Poco::JSON
    Poco::Crypto
    Poco::MongoDB
)

target_include_directories(api PRIVATE
    include
)