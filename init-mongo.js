db = db.getSiblingDB('hotel_booking');

// Гsers collection with validation
db.createCollection('users', {
  validator: {
    $jsonSchema: {
      bsonType: 'object',
      required: ['login', 'password', 'firstName', 'lastName'],
      properties: {
        _id: { bsonType: 'objectId' },
        login: {
          bsonType: 'string',
          description: 'User login (unique)',
          minLength: 3,
          maxLength: 50
        },
        password: {
          bsonType: 'string',
          description: 'Hashed password'
        },
        firstName: {
          bsonType: 'string',
          description: 'User first name',
          minLength: 1,
          maxLength: 50
        },
        lastName: {
          bsonType: 'string',
          description: 'User last name',
          minLength: 1,
          maxLength: 50
        },
        createdAt: { bsonType: 'date' },
        updatedAt: { bsonType: 'date' }
      }
    }
  }
});

db.users.createIndex({ login: 1 }, { unique: true });

// Hotels collection with validation
db.createCollection('hotels', {
  validator: {
    $jsonSchema: {
      bsonType: 'object',
      required: ['name', 'city', 'stars'],
      properties: {
        _id: { bsonType: 'objectId' },
        name: {
          bsonType: 'string',
          description: 'Hotel name',
          minLength: 1,
          maxLength: 100
        },
        city: {
          bsonType: 'string',
          description: 'City where hotel is located',
          minLength: 1,
          maxLength: 50
        },
        stars: {
          bsonType: 'int',
          description: 'Hotel star rating',
          minimum: 1,
          maximum: 5
        },
        createdAt: { bsonType: 'date' },
        updatedAt: { bsonType: 'date' }
      }
    }
  }
});

db.hotels.createIndex({ city: 1 });
db.hotels.createIndex({ name: 1 });

// Bookings collection with validation
db.createCollection('bookings', {
  validator: {
    $jsonSchema: {
      bsonType: 'object',
      required: ['userId', 'hotelId', 'startDate', 'endDate'],
      properties: {
        _id: { bsonType: 'objectId' },
        userId: {
          bsonType: 'int',
          description: 'User ID'
        },
        hotelId: {
          bsonType: 'int',
          description: 'Hotel ID'
        },
        startDate: {
          bsonType: 'string',
          description: 'Booking start date (YYYY-MM-DD)'
        },
        endDate: {
          bsonType: 'string',
          description: 'Booking end date (YYYY-MM-DD)'
        },
        cancelled: {
          bsonType: 'bool',
          description: 'Whether booking is cancelled'
        },
        createdAt: { bsonType: 'date' },
        updatedAt: { bsonType: 'date' }
      }
    }
  }
});

db.bookings.createIndex({ userId: 1 });
db.bookings.createIndex({ hotelId: 1 });
db.bookings.createIndex({ startDate: 1, endDate: 1 });

db.createCollection('sessions');
db.sessions.createIndex({ expiresAt: 1 }, { expireAfterSeconds: 0 });

db.users.insertMany([
  {
    login: 'user1',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=', // 'pass'
    firstName: 'Ivan',
    lastName: 'Ivanov',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user2',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Petr',
    lastName: 'Petrov',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user3',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Anna',
    lastName: 'Sidorova',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user4',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Olga',
    lastName: 'Smirnova',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user5',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Sergey',
    lastName: 'Volkov',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user6',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Alex',
    lastName: 'Kuznetsov',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user7',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Maria',
    lastName: 'Popova',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user8',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Dmitry',
    lastName: 'Sokolov',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user9',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Elena',
    lastName: 'Lebedeva',
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    login: 'user10',
    password: 'alP6BiBs3jtaFY8EcjQ5MzAqoJBMOWsGsEVSEIGRf/0=',
    firstName: 'Max',
    lastName: 'Morozov',
    createdAt: new Date(),
    updatedAt: new Date()
  }
]);

db.hotels.insertMany([
  {
    name: 'Hilton',
    city: 'Moscow',
    stars: NumberInt(5),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Marriott',
    city: 'Moscow',
    stars: NumberInt(5),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Ibis',
    city: 'Moscow',
    stars: NumberInt(3),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Radisson',
    city: 'SPB',
    stars: NumberInt(5),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Nevsky Hotel',
    city: 'SPB',
    stars: NumberInt(4),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Budget Inn',
    city: 'Kazan',
    stars: NumberInt(2),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Grand Hotel',
    city: 'Kazan',
    stars: NumberInt(5),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'City Stay',
    city: 'Sochi',
    stars: NumberInt(3),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Sea View',
    city: 'Sochi',
    stars: NumberInt(4),
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    name: 'Comfort Hotel',
    city: 'Sochi',
    stars: NumberInt(3),
    createdAt: new Date(),
    updatedAt: new Date()
  }
]);

db.bookings.insertMany([
  {
    userId: NumberInt(1),
    hotelId: NumberInt(1),
    startDate: '2026-05-01',
    endDate: '2026-05-05',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(2),
    hotelId: NumberInt(2),
    startDate: '2026-05-02',
    endDate: '2026-05-06',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(3),
    hotelId: NumberInt(3),
    startDate: '2026-05-03',
    endDate: '2026-05-07',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(4),
    hotelId: NumberInt(4),
    startDate: '2026-05-04',
    endDate: '2026-05-08',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(5),
    hotelId: NumberInt(5),
    startDate: '2026-05-05',
    endDate: '2026-05-09',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(6),
    hotelId: NumberInt(6),
    startDate: '2026-05-06',
    endDate: '2026-05-10',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(7),
    hotelId: NumberInt(7),
    startDate: '2026-05-07',
    endDate: '2026-05-11',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(8),
    hotelId: NumberInt(8),
    startDate: '2026-05-08',
    endDate: '2026-05-12',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(9),
    hotelId: NumberInt(9),
    startDate: '2026-05-09',
    endDate: '2026-05-13',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  },
  {
    userId: NumberInt(10),
    hotelId: NumberInt(10),
    startDate: '2026-05-10',
    endDate: '2026-05-14',
    cancelled: false,
    createdAt: new Date(),
    updatedAt: new Date()
  }
]);
