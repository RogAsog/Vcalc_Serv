/**
 * @file test_cli.cpp
 * @brief Тест интерфейса командной строки с использованием UnitTest++
 */

#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>

// Объявление функции main_server из server.cpp
extern "C" int main_server(int argc, char* argv[]);

using namespace std;

// Вспомогательная функция для создания тестовых файлов
void create_test_file(const string& filename, const string& content) {
    ofstream file(filename);
    file << content;
    file.close();
}

// Вспомогательная функция для преобразования vector<string> в char* argv[]
vector<char*> create_argv(const vector<string>& args) {
    vector<char*> argv;
    argv.push_back(const_cast<char*>("server"));  // argv[0]
    
    for (const auto& arg : args) {
        char* arg_copy = new char[arg.size() + 1];
        strcpy(arg_copy, arg.c_str());
        argv.push_back(arg_copy);
    }
    
    argv.push_back(nullptr);  // NULL terminator
    return argv;
}

// Вспомогательная функция для очистки argv
void cleanup_argv(vector<char*>& argv) {
    for (size_t i = 1; i < argv.size() - 1; i++) {
        delete[] argv[i];
    }
}

SUITE(CLITests) {
    
    struct Setup {
        Setup() {
            // Создаем тестовые файлы пользователей
            create_test_file("test_users.txt", "user:pass\nadmin:admin123\n");
        }
        
        ~Setup() {
            // Удаляем временные файлы
            remove("test_users.txt");
            remove("empty_users.txt");
            remove("test.log");
            remove("server.log");
        }
    };
    
    // Тест 1: Опция --help
    TEST_FIXTURE(Setup, TestHelpFlag) {
        vector<string> args = {"--help"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK_EQUAL(0, result);
    }
    
    // Тест 2: Опция -h (краткая справка)
    TEST_FIXTURE(Setup, TestShortHelpFlag) {
        vector<string> args = {"-h"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK_EQUAL(0, result);
    }
    
    // Тест 3: Неизвестный аргумент
    TEST_FIXTURE(Setup, TestInvalidArgument) {
        vector<string> args = {"--invalid-arg"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK(result != 0);
    }
    
    // Тест 4: Неверный порт (строка)
    TEST_FIXTURE(Setup, TestInvalidPortString) {
        vector<string> args = {"-p", "not_a_number", "-d", "test_users.txt"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK(result != 0);
    }
    
    // Тест 5: Неверный порт (ноль)
    TEST_FIXTURE(Setup, TestInvalidPortZero) {
        vector<string> args = {"-p", "0", "-d", "test_users.txt"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK(result != 0);
    }
    
    // Тест 6: Неверный порт (слишком большой)
    TEST_FIXTURE(Setup, TestInvalidPortTooLarge) {
        vector<string> args = {"-p", "70000", "-d", "test_users.txt"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK(result != 0);
    }
    
    // Тест 7: Корректный порт с --help
    TEST_FIXTURE(Setup, TestValidPortWithHelp) {
        vector<string> args = {"-p", "8080", "-d", "test_users.txt", "--help"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK_EQUAL(0, result);
    }
    
    // Тест 8: Отсутствующий файл пользователей
    TEST_FIXTURE(Setup, TestMissingDatabaseFile) {
        vector<string> args = {"--database=nonexistent.txt"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK(result != 0);
    }
    
    // Тест 9: Пустой файл пользователей
    TEST_FIXTURE(Setup, TestEmptyDatabaseFile) {
        create_test_file("empty_users.txt", "");
        
        vector<string> args = {"-d", "empty_users.txt"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK(result != 0);
        
        remove("empty_users.txt");
    }
    
    // Тест 10: Все опции с --help
    TEST_FIXTURE(Setup, TestAllOptionsWithHelp) {
        vector<string> args = {"--database=test_users.txt", "--log=test.log", "--port=8080", "--help"};
        vector<char*> argv = create_argv(args);
        int result = main_server(args.size() + 1, argv.data());
        cleanup_argv(argv);
        CHECK_EQUAL(0, result);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
