# MongoDB Schema Design для системы бронирования отелей

## Вариант задания
Вариант 13: REST API сервис для бронирования отелей

## Обзор системы
- **Основные сущности**: Пользователи, Отели, Бронирования, Сессии
- **Основные операции**: Регистрация, вход, поиск отелей, управление бронированиями

## Проектирование документной модели

### 1. Коллекция `users`

**Структура документа:**
```json
{
  "_id": ObjectId,
  "login": "john_doe",
  "password": "hashed_password",
  "firstName": "John",
  "lastName": "Doe",
  "createdAt": ISODate,
  "updatedAt": ISODate
}
```

**Обоснование:**
- Используется **embedded design** для хранения всей информации о пользователе в одном документе
- Информация не требует часто обновляться независимо
- Минимизирует количество операций при аутентификации
- Индекс на `login` для быстрого поиска по логину

**Индексы:**
```javascript
db.users.createIndex({ login: 1 }, { unique: true })
```

---

### 2. Коллекция `hotels`

**Структура документа:**
```json
{
  "_id": ObjectId,
  "name": "Hotel Name",
  "city": "Moscow",
  "stars": 5,
  "createdAt": ISODate,
  "updatedAt": ISODate
}
```

**Обоснование:**
- **Embedded design** - информация об отеле самодостаточна
- Не требует часто обновляться
- Город используется часто для фильтрации, поэтому добавлен индекс

**Индексы:**
```javascript
db.hotels.createIndex({ city: 1 })
db.hotels.createIndex({ name: 1 })
```

---

### 3. Коллекция `bookings`

**Структура документа:**
```json
{
  "_id": ObjectId,
  "userId": ObjectId,
  "hotelId": ObjectId,
  "startDate": ISODate,
  "endDate": ISODate,
  "cancelled": false,
  "createdAt": ISODate,
  "updatedAt": ISODate
}
```

**Обоснование:**
- **Reference design** для связи с пользователями и отелями:
  - Бронирование может быть отменено независимо
  - Пользователи могут иметь множество бронирований
  - Отели могут иметь множество бронирований
  - Нет необходимости дублировать данные пользователя/отеля в каждом бронировании
- Хранение ссылок на `userId` и `hotelId` как ObjectId
- Индекс на `userId` для быстрого получения бронирований пользователя
- Индекс на временные диапазоны для проверки конфликтов

**Индексы:**
```javascript
db.bookings.createIndex({ userId: 1 })
db.bookings.createIndex({ hotelId: 1 })
db.bookings.createIndex({ startDate: 1, endDate: 1 })
```

---

### 4. Коллекция `sessions`

**Структура документа:**
```json
{
  "_id": "uuid-token",
  "userId": ObjectId,
  "createdAt": ISODate,
  "expiresAt": ISODate
}
```

**Обоснование:**
- **Reference design** с ссылкой на userId
- Токен хранится как `_id` для быстрого поиска
- TTL индекс для автоматического удаления истекших сессий
- Компактная структура для минимального использования памяти

**Индексы:**
```javascript
db.sessions.createIndex({ userId: 1 })
db.sessions.createIndex({ expiresAt: 1 }, { expireAfterSeconds: 0 })
```

---

## Сравнение Embedded vs References

| Сценарий | Embedded | Reference | Выбор |
|----------|----------|-----------|--------|
| Пользователь + его бронирования | Нет (много-ко-одному) | Да | Reference |
| Бронирование + отель | Нет (много-ко-одному) | Да | Reference |
| Пользователь + его данные | Да (все вместе) | Нет | Embedded |
| Отель + его информация | Да (все вместе) | Нет | Embedded |

### Обоснование выбора:

1. **Embedded для User и Hotel** ✓
   - Данные редко обновляются независимо
   - Компактность и производительность
   - Достаточно читаются вместе

2. **Reference для Bookings** ✓
   - Много-ко-одному отношение (один отель → много бронирований)
   - Бронирования часто изменяются независимо
   - Предотвращает дублирование данных
   - Позволяет отелю/пользователю существовать независимо

---

## CRUD Операции

### Создание (Create)

**Create User:**
```javascript
db.users.insertOne({
  login: "user",
  password: "hash",
  firstName: "John",
  lastName: "Doe",
  createdAt: new Date(),
  updatedAt: new Date()
})
```

**Create Hotel:**
```javascript
db.hotels.insertOne({
  name: "Hotel Name",
  city: "Moscow",
  stars: 5,
  createdAt: new Date(),
  updatedAt: new Date()
})
```

**Create Booking:**
```javascript
db.bookings.insertOne({
  userId: ObjectId("..."),
  hotelId: ObjectId("..."),
  startDate: new Date("2025-06-01"),
  endDate: new Date("2025-06-10"),
  cancelled: false,
  createdAt: new Date(),
  updatedAt: new Date()
})
```

### Чтение (Read)

**Find by login:**
```javascript
db.users.findOne({ login: "user_login" })
```

**Find all hotels:**
```javascript
db.hotels.find({})
```

**Find hotels by city:**
```javascript
db.hotels.find({ city: "Moscow" })
```

**Find user bookings:**
```javascript
db.bookings.find({ userId: ObjectId("...") })
```

### Обновление (Update)

**Update booking status:**
```javascript
db.bookings.updateOne(
  { _id: ObjectId("...") },
  { $set: { cancelled: true, updatedAt: new Date() } }
)
```

### Удаление (Delete)

**Delete session:**
```javascript
db.sessions.deleteOne({ _id: "token" })
```

---

## Масштабируемость

1. **Шардирование** (если понадобится):
   - Шардирование по `userId` в коллекции bookings
   - Шардирование по `city` в коллекции hotels

2. **Оптимизация**:
   - TTL индекс на sessions для автоматической очистки
   - Комплексные индексы на часто используемые комбинации полей

3. **Безопасность**:
   - Пароли всегда хешируются перед сохранением
   - Никогда не возвращаются клиентам

---

## Типы данных MongoDB

| Поле | Тип | Обоснование |
|------|---|----|
| _id | ObjectId | Уникальный идентификатор документа |
| login | String | Текстовое значение |
| password | String | Хешированный пароль |
| firstName, lastName | String | Текстовые значения |
| stars | Number (Int32) | Целое число |
| startDate, endDate | Date | ISO Date для работы с датами |
| cancelled | Boolean | Логическое значение |
| userId, hotelId | ObjectId | Ссылка на другие документы |

