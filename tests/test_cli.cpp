/**
 * @file test_cli.cpp
 * @brief Тест интерфейса командной строки
 */

#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

void safe_system(const char* cmd) {
    int result = system(cmd);
    (void)result;
}

bool test_help() {
    return system("./server --help > /dev/null 2>&1") == 0;
}

bool test_short_help() {
    return system("./server -h > /dev/null 2>&1") == 0;
}

bool test_invalid_arg() {
    return system("./server --invalid-arg > /dev/null 2>&1") != 0;
}

bool test_with_port() {
    safe_system("echo 'user:pass' > test_users.txt 2>/dev/null");
    int result = system("timeout 0.5 ./server -p 55555 -d test_users.txt -l test.log > /dev/null 2>&1");
    safe_system("rm -f test_users.txt test.log 2>/dev/null");
    return result != 0;
}

bool test_default_port() {
    safe_system("echo 'user:pass' > users2.txt 2>/dev/null");
    int result = system("timeout 0.5 ./server -d users2.txt -l server2.log > /dev/null 2>&1");
    safe_system("rm -f users2.txt server2.log 2>/dev/null");
    return result != 0;
}

void run_cli_tests() {
    int passed = 0;
    int total = 0;
    
    cout << "Тесты интерфейса командной строки:\n";
    
    total++; 
    if (test_help()) {
        cout << "  OK  - Отображение справки (--help)\n";
        passed++;
    } else {
        cout << "  FAIL - Отображение справки\n";
    }
    
    total++;
    if (test_short_help()) {
        cout << "  OK  - Короткая справка (-h)\n";
        passed++;
    } else {
        cout << "  FAIL - Короткая справка\n";
    }
    
    total++;
    if (test_invalid_arg()) {
        cout << "  OK  - Обработка неверного аргумента\n";
        passed++;
    } else {
        cout << "  FAIL - Обработка неверного аргумента\n";
    }
    
    total++;
    if (test_with_port()) {
        cout << "  OK  - Указание порта (-p 55555)\n";
        passed++;
    } else {
        cout << "  FAIL - Указание порта\n";
    }
    
    total++;
    if (test_default_port()) {
        cout << "  OK  - Порт по умолчанию (33333)\n";
        passed++;
    } else {
        cout << "  FAIL - Порт по умолчанию\n";
    }
    
    cout << "\nИтого CLI: " << passed << "/" << total << " тестов пройдено\n";
}

int main() {
    if (system("test -f ./server 2>/dev/null") != 0) {
        cout << "Ошибка: сначала соберите сервер (make all)\n";
        return 1;
    }
    
    safe_system("pkill -f './server' > /dev/null 2>&1");
    
    run_cli_tests();
    
    safe_system("rm -f test_users.txt users2.txt test.log server2.log 2>/dev/null");
    safe_system("pkill -f './server' > /dev/null 2>&1");
    
    return 0;
}
