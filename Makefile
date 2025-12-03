CXX = g++
CXXFLAGS = -Wall -Wextra -O2 -std=c++17
LIBS = -lboost_program_options

TARGET = server
SOURCES = server.cpp sha256.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET) users.txt server.log

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %..cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Создание users.txt с одним пользователем
users.txt:
	@echo "user:P@ssW0rd" > users.txt
	@echo "Created users.txt"

# Создание лог-файла
server.log:
	@touch server.log
	@chmod 644 server.log
	@echo "Created server.log"

# Запуск сервера
run: all
	@echo "Starting server on port 33333..."
	@./$(TARGET) -d users.txt -l server.log -p 33333

# Очистка
clean:
	rm -f $(OBJECTS) $(TARGET) users.txt server.log
	@echo "Cleaned all generated files"

.PHONY: all run clean
