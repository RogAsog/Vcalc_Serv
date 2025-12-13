CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2 -I. -Wno-unused-result
LIBS = -lboost_program_options -lUnitTest++

SERVER_SOURCES = server.cpp sha256.cpp
SERVER_OBJ = $(SERVER_SOURCES:.cpp=.o)

DOXYFILE = Doxyfile
DOC_DIR = docs

.PHONY: all run clean doc pdf-doc view-doc view-pdf test test-func test-all help

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

# Модульные тесты
test: tests/test_sha256 tests/test_auth tests/test_vectors tests/test_protocol tests/test_cli
	@echo "======================================="
	@echo "Запуск модульных тестов..."
	@echo "======================================="
	@echo "Тесты SHA256:"
	@./tests/test_sha256
	@echo ""
	@echo "Тесты аутентификации:"
	@./tests/test_auth
	@echo ""
	@echo "Тесты векторов:"
	@./tests/test_vectors
	@echo ""
	@echo "Тесты протокола:"
	@./tests/test_protocol
	@echo ""
	@echo "Тесты CLI:"
	@./tests/test_cli
	@echo "======================================="
	@echo "Модульные тесты завершены"

# Простые функциональные тесты
test-func: tests/test_func
	@echo "======================================="
	@echo "Запуск функциональных тестов..."
	@echo "======================================="
	@./tests/test_func

# Все тесты
test-all: test test-func

# Компиляция тестов - все из исходников напрямую
tests/test_sha256: tests/test_sha256.cpp sha256.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

tests/test_auth: tests/test_auth.cpp sha256.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

tests/test_vectors: tests/test_vectors.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

tests/test_protocol: tests/test_protocol.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

# Компиляция test_cli с флагом TEST_MODE
tests/test_cli: tests/test_cli.cpp server.cpp sha256.cpp
	$(CXX) $(CXXFLAGS) -DTEST_MODE -o $@ $^ $(LIBS)

# Простые функциональные тесты
tests/test_func: tests/test_func.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f $(SERVER_OBJ) server users.txt server.log
	rm -f tests/test_sha256 tests/test_auth tests/test_vectors tests/test_protocol tests/test_cli
	rm -f tests/test_func
	rm -f test*.txt test*.log empty_users.txt 2>/dev/null
	rm -rf $(DOC_DIR)
	@pkill -f './server' 2>/dev/null || true

help:
	@echo "Доступные команды:"
	@echo "  make all        - собрать сервер"
	@echo "  make run        - запустить сервер"
	@echo "  make test       - модульные тесты"
	@echo "  make test-func  - функциональные тесты"
	@echo "  make test-all   - все тесты"
	@echo "  make doc        - документация"
	@echo "  make pdf-doc    - PDF документация"
	@echo "  make view-doc   - открыть HTML документацию"
	@echo "  make view-pdf   - открыть PDF документацию"
	@echo "  make clean      - очистить"
	@echo "  make help       - справка"
