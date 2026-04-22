FROM ubuntu:22.04

# Установка зависимостей
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    telnet \
    libpoco-dev \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Рабочая директория
WORKDIR /app

# Копирование файлов проекта
COPY CMakeLists.txt .
COPY src ./src
COPY include ./include

# Создание директории сборки и компиляция
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make -j4

WORKDIR /app/build

EXPOSE 8080

# Запуск исполняемого файла
CMD ["./api"]