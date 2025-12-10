CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -I.
LIBS = -lboost_program_options -lUnitTest++

SERVER_SOURCES = server.cpp sha256.cpp
SERVER_OBJ = $(SERVER_SOURCES:.cpp=.o)

DOXYFILE = Doxyfile
DOC_DIR = docs

.PHONY: all run clean doc pdf-doc view-doc view-pdf test help

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

pdf-doc: doc
	@if [ -d "$(DOC_DIR)/latex" ]; then \
		echo "Создание PDF документации..."; \
		cd $(DOC_DIR)/latex; \
		sed -i '/\\usepackage{textgreek}/d' refman.tex 2>/dev/null || true; \
		sed -i 's/≈/\$\\approx\$/g' *.tex 2>/dev/null || true; \
		sed -i 's/Σ/\$\\Sigma\$/g' *.tex 2>/dev/null || true; \
		sed -i 's/σ/\$\\sigma\$/g' *.tex 2>/dev/null || true; \
		pdflatex -interaction=nonstopmode refman.tex > /dev/null 2>&1; \
		pdflatex -interaction=nonstopmode refman.tex > /dev/null 2>&1; \
		echo "PDF документация создана: $(DOC_DIR)/latex/refman.pdf"; \
	else \
		echo "LaTeX документация не сгенерирована. Проверьте Doxyfile."; \
	fi

view-doc:
	@if [ -f "$(DOC_DIR)/html/index.html" ]; then \
		xdg-open "$(DOC_DIR)/html/index.html" 2>/dev/null || echo "Откройте: $(DOC_DIR)/html/index.html"; \
	else \
		echo "Сначала создайте документацию: make doc"; \
	fi

view-pdf:
	@if [ -f "$(DOC_DIR)/latex/refman.pdf" ]; then \
		xdg-open "$(DOC_DIR)/latex/refman.pdf" 2>/dev/null || echo "Откройте: $(DOC_DIR)/latex/refman.pdf"; \
	else \
		echo "PDF документация не найдена. Сначала создайте: make pdf-doc"; \
	fi

# Компиляция и запуск всех тестов
test: tests/test_sha256 tests/test_auth tests/test_vectors tests/test_protocol tests/test_cli
	@echo "======================================="
	@echo "Запуск UnitTest++ тестов..."
	@echo "======================================="
	@echo "1. SHA256 тесты:"
	@./tests/test_sha256
	@echo "======================================="
	@echo "2. Аутентификация тесты:"
	@./tests/test_auth
	@echo "======================================="
	@echo "3. Векторы тесты:"
	@./tests/test_vectors
	@echo "======================================="
	@echo "4. Протокол тесты:"
	@./tests/test_protocol
	@echo "======================================="
	@echo "5. CLI тесты:"
	@./tests/test_cli
	@echo "======================================="
	@echo "Все тесты завершены"

# SHA256 тесты
tests/test_sha256: tests/test_sha256.cpp sha256.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_sha256.cpp sha256.cpp $(LIBS)

# Аутентификация тесты
tests/test_auth: tests/test_auth.cpp sha256.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_auth.cpp sha256.cpp $(LIBS)

# Векторы тесты
tests/test_vectors: tests/test_vectors.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_vectors.cpp $(LIBS)

# Протокол тесты
tests/test_protocol: tests/test_protocol.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_protocol.cpp $(LIBS)

# CLI тесты - ТЕПЕРЬ С UnitTest++
tests/test_cli: tests/test_cli.cpp
	$(CXX) $(CXXFLAGS) -o $@ tests/test_cli.cpp $(LIBS)  # Добавил $(LIBS)

clean:
	rm -f $(SERVER_OBJ) server users.txt server.log
	rm -f tests/test_sha256 tests/test_auth tests/test_vectors tests/test_protocol tests/test_cli
	rm -rf $(DOC_DIR)

help:
	@echo "Доступные команды:"
	@echo "  make all        - собрать сервер"
	@echo "  make run        - запустить сервер"
	@echo "  make test       - собрать и запустить все UnitTest++ тесты"
	@echo "  make doc        - создать HTML документацию Doxygen"
	@echo "  make pdf-doc    - создать PDF документацию (нужен doc)"
	@echo "  make view-doc   - открыть HTML документацию"
	@echo "  make view-pdf   - открыть PDF документацию"
	@echo "  make clean      - очистить проект"
	@echo "  make help       - показать эту справку"
