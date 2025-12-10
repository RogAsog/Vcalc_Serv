/**
 * @file test_cli.cpp
 * @brief Тест интерфейса командной строки с использованием UnitTest++
 */

#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

// Вспомогательная функция для подавления предупреждения
int safe_system(const char* cmd) {
    return system(cmd);
}

SUITE(CLITests) {
    // Проверяем наличие сервера перед тестами
    struct Setup {
        Setup() {
            if (system("test -f ./server 2>/dev/null") != 0) {
                cerr << "Ошибка: сначала соберите сервер (make all)" << endl;
                exit(1);
            }
            // Убиваем старые процессы сервера
            safe_system("pkill -f './server' > /dev/null 2>&1");
        }
        
        ~Setup() {
            // Очистка после тестов
            safe_system("rm -f test_users.txt users2.txt test.log server2.log 2>/dev/null");
            safe_system("pkill -f './server' > /dev/null 2>&1");
        }
    };
    
    TEST_FIXTURE(Setup, HelpFlag) {
        int result = safe_system("./server --help > /dev/null 2>&1");
        CHECK_EQUAL(0, result);
    }
    
    TEST_FIXTURE(Setup, ShortHelpFlag) {
        int result = safe_system("./server -h > /dev/null 2>&1");
        CHECK_EQUAL(0, result);
    }
    
    TEST_FIXTURE(Setup, InvalidArgument) {
        int result = safe_system("./server --invalid-arg > /dev/null 2>&1");
        CHECK(result != 0);
    }
    
    TEST_FIXTURE(Setup, WithPortOption) {
        // Создаем временный файл пользователей
        int create_result = safe_system("echo 'user:pass' > test_users.txt 2>/dev/null");
        CHECK_EQUAL(0, create_result);
        
        // Запускаем сервер с указанием порта
        int result = safe_system("timeout 0.5 ./server -p 55555 -d test_users.txt -l test.log > /dev/null 2>&1");
        CHECK(result != 0); // timeout должен завершить процесс
    }
    
    TEST_FIXTURE(Setup, DefaultPort) {
        // Создаем временный файл пользователей
        int create_result = safe_system("echo 'user:pass' > users2.txt 2>/dev/null");
        CHECK_EQUAL(0, create_result);
        
        // Запускаем сервер с портом по умолчанию
        int result = safe_system("timeout 0.5 ./server -d users2.txt -l server2.log > /dev/null 2>&1");
        CHECK(result != 0); // timeout должен завершить процесс
    }
    
    TEST_FIXTURE(Setup, DatabaseOption) {
        int result = safe_system("./server --database=test.txt --help > /dev/null 2>&1");
        CHECK_EQUAL(0, result);
    }
    
    TEST_FIXTURE(Setup, LogOption) {
        int result = safe_system("./server --log=test.log --help > /dev/null 2>&1");
        CHECK_EQUAL(0, result);
    }
    
    TEST_FIXTURE(Setup, AllOptions) {
        // Создаем временный файл пользователей
        int create_result = safe_system("echo 'admin:admin123' > all_test_users.txt 2>/dev/null");
        CHECK_EQUAL(0, create_result);
        
        // Проверяем все опции
        int result = safe_system("./server --database=all_test_users.txt --log=all_test.log --port=77777 --help > /dev/null 2>&1");
        CHECK_EQUAL(0, result);
        
        // Очистка
        safe_system("rm -f all_test_users.txt all_test.log 2>/dev/null");
    }
    
    TEST_FIXTURE(Setup, MissingDatabaseFile) {
        // Пытаемся запустить с несуществующим файлом пользователей
        int result = safe_system("./server --database=nonexistent.txt > /dev/null 2>&1");
        CHECK(result != 0);
    }
}

int main() {
    // Проверяем, собран ли сервер
    if (system("test -f ./server 2>/dev/null") != 0) {
        cerr << "Ошибка: сначала соберите сервер (make all)" << endl;
        return 1;
    }
    
    return UnitTest::RunAllTests();
}
