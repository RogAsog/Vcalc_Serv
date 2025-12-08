CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
LIBS = -lboost_program_options

SERVER_SOURCES = server.cpp sha256.cpp
SERVER_OBJ = $(SERVER_SOURCES:.cpp=.o)

DOXYFILE = Doxyfile
DOC_DIR = docs

.PHONY: all run clean doc simple-doc view-doc test help

all: server users.txt server.log
	@echo "Сервер собран"

server: $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

users.txt:
	@echo "user:P@ssW0rd" > users.txt

server.log:
	@touch server.log

run: all
	@./server -d users.txt -l server.log -p 33333

doc:
	@mkdir -p $(DOC_DIR)
	doxygen $(DOXYFILE)
	@echo "Документация создана: $(DOC_DIR)/html/index.html"

simple-doc:
	@echo "Создание простой документации..."
	@mkdir -p $(DOC_DIR)/simple
	@echo '<!DOCTYPE html>' > $(DOC_DIR)/simple/index.html
	@echo '<html>' >> $(DOC_DIR)/simple/index.html
	@echo '<head>' >> $(DOC_DIR)/simple/index.html
	@echo '    <meta charset="UTF-8">' >> $(DOC_DIR)/simple/index.html
	@echo '    <title>Документация vcalc</title>' >> $(DOC_DIR)/simple/index.html
	@echo '</head>' >> $(DOC_DIR)/simple/index.html
	@echo '<body>' >> $(DOC_DIR)/simple/index.html
	@echo '    <h1>Vcalc Server - документация</h1>' >> $(DOC_DIR)/simple/index.html
	@echo '    <p>Курсовая работа: сервер аутентификации и вычислений</p>' >> $(DOC_DIR)/simple/index.html
	@echo '    <p>Автор: [Ваше имя], Группа: [Номер], 2024</p>' >> $(DOC_DIR)/simple/index.html
	@echo '</body>' >> $(DOC_DIR)/simple/index.html
	@echo '</html>' >> $(DOC_DIR)/simple/index.html
	@echo "Простая документация создана: $(DOC_DIR)/simple/index.html"

view-doc:
	@if [ -f "$(DOC_DIR)/html/index.html" ]; then \
		xdg-open "$(DOC_DIR)/html/index.html" 2>/dev/null || echo "Откройте: $(DOC_DIR)/html/index.html"; \
	elif [ -f "$(DOC_DIR)/simple/index.html" ]; then \
		xdg-open "$(DOC_DIR)/simple/index.html" 2>/dev/null || echo "Откройте: $(DOC_DIR)/simple/index.html"; \
	else \
		echo "Сначала создайте документацию: make doc или make simple-doc"; \
	fi

test: tests/test_sha256 tests/test_auth tests/test_cli
	@echo "Запуск модульных тестов..."
	@./tests/test_sha256
	@./tests/test_auth
	@echo ""
	@echo "Запуск тестов интерфейса командной строки..."
	@./tests/test_cli

tests/test_sha256: tests/test_sha256.cpp sha256.cpp sha256.hpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_sha256.cpp sha256.cpp

tests/test_auth: tests/test_auth.cpp sha256.cpp sha256.hpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_auth.cpp sha256.cpp

tests/test_cli: tests/test_cli.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_cli.cpp

clean:
	rm -f $(SERVER_OBJ) server users.txt server.log
	rm -f tests/test_sha256 tests/test_auth tests/test_cli
	rm -rf $(DOC_DIR)

help:
	@echo "Доступные команды:"
	@echo "  make all        - собрать сервер"
	@echo "  make run        - запустить сервер"
	@echo "  make test       - запустить все тесты"
	@echo "  make doc        - создать документацию Doxygen"
	@echo "  make simple-doc - создать простую документацию"
	@echo "  make view-doc   - открыть документацию"
	@echo "  make clean      - очистить проект"
	@echo "  make help       - показать эту справку"
