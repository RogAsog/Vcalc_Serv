CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
LIBS = -lboost_program_options

SERVER_SOURCES = server.cpp sha256.cpp
SERVER_OBJ = $(SERVER_SOURCES:.cpp=.o)

all: server users.txt server.log

server: $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

users.txt:
	@echo "user:P@ssW0rd" > users.txt

server.log:
	@touch server.log

run: all
	@echo "Запуск сервера..."
	@./server -d users.txt -l server.log -p 33333

tests/test_sha256: tests/test_sha256.cpp sha256.cpp sha256.hpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_sha256.cpp sha256.cpp

tests/test_auth: tests/test_auth.cpp sha256.cpp sha256.hpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_auth.cpp sha256.cpp

tests/test_vectors: tests/test_vectors.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_vectors.cpp

tests/test_protocol: tests/test_protocol.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_protocol.cpp

build-tests: tests/test_sha256 tests/test_auth tests/test_vectors tests/test_protocol
	@echo "Тесты скомпилированы"

test: build-tests
	@echo "Запуск модульных тестов..."
	@echo "========================================"
	@./tests/test_sha256
	@echo "========================================"
	@./tests/test_auth
	@echo "========================================"
	@./tests/test_vectors
	@echo "========================================"
	@./tests/test_protocol
	@echo "========================================"
	@echo "Все тесты завершены"

clean:
	rm -f $(SERVER_OBJ) server users.txt server.log
	rm -f tests/test_sha256 tests/test_auth tests/test_vectors tests/test_protocol
	@echo "Очищены все сгенерированные файлы"

help:
	@echo "Доступные команды:"
	@echo "  make all    - собрать сервер"
	@echo "  make run    - собрать и запустить сервер"
	@echo "  make test   - собрать и запустить все тесты"
	@echo "  make clean  - очистить всё"
	@echo "  make help   - справка"

.PHONY: all run clean test build-tests help
