#!/bin/bash
BASE="http://localhost:8080"

echo -e "\n\n1. Register User"
RES=$(curl -s -X POST $BASE/api/users/register -H "Content-Type: application/json" -d '{"login":"user11","password":"pass","firstName":"Ivan","lastName":"Ivanov"}')
echo $RES

echo -e "\n\n2. Login"
curl -s -X POST $BASE/api/users/login -H "Content-Type: application/json" -d '{"login":"user11","password":"pass"}'
TOKEN=$(curl -s -X POST $BASE/api/users/login -H "Content-Type: application/json" -d '{"login":"user11","password":"pass"}' | grep -o '"token":"[^"]*"' | cut -d'"' -f4)
echo "Token: $TOKEN"

echo -e "\n\n3. Create Hotel"
curl -s -X POST $BASE/api/hotels -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"name":"Grand Hotel","city":"Moscow","stars":5}'

echo -e "\n\n4. Get Hotels"
curl -s $BASE/api/hotels

echo -e "\n\n5. Search Hotels by City"
curl -s "$BASE/api/hotels?city=Moscow"

echo -e "\n\n6. Create Booking"
BOOK_RES=$(curl -s -X POST $BASE/api/bookings -H "Content-Type: application/json" -H "Authorization: Bearer $TOKEN" -d '{"hotelId":1,"startDate":"2023-10-01","endDate":"2023-10-10"}')
echo $BOOK_RES
# Извлекаем ID из ответа {"id":число}
BOOKING_ID=$(echo $BOOK_RES | grep -o '"id":[0-9]*' | cut -d':' -f2)
echo "Booking ID: $BOOKING_ID"

echo -e "\n\n7. Get Bookings"
curl -s $BASE/api/bookings -H "Authorization: Bearer $TOKEN"

echo -e "\n\n8. Cancel Booking"
# Используем извлечённый ID
curl -s -X DELETE $BASE/api/bookings/$BOOKING_ID -H "Authorization: Bearer $TOKEN"


echo -e "\n\n8.1 Get Bookings"
curl -s $BASE/api/bookings -H "Authorization: Bearer $TOKEN"

echo -e "\n\n9. Search User by Mask"
curl -s "$BASE/api/users?name_mask=Ivan" -H "Authorization: Bearer $TOKEN"