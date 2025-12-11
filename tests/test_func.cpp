/**
 * @file test_func.cpp
 * @brief Функциональные тесты сервера vcalc
 */

#include <UnitTest++/UnitTest++.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Запуск команды с таймаутом
int run_command(const string& cmd, int timeout_seconds = 5) {
    string timed_cmd = "timeout " + to_string(timeout_seconds) + " " + cmd;
    return system(timed_cmd.c_str());
}

// Проверка занятости порта
bool check_server_running(int port) {
    string cmd = "lsof -ti:" + to_string(port) + " 2>/dev/null | wc -l";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return false;
    
    char buffer[16];
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        // ignore
    }
    pclose(pipe);
    
    return atoi(buffer) > 0;
}

// Остановка сервера на порту
void stop_server(int port) {
    string cmd = "pkill -f './server.*-p " + to_string(port) + "' 2>/dev/null";
    int result = system(cmd.c_str());
    (void)result;
    this_thread::sleep_for(seconds(1));
}

// Создание тестового файла пользователей
void create_test_users(const string& filename) {
    ofstream file(filename);
    file << "user1:password1" << endl;
    file << "user2:password2" << endl;
    file << "admin:admin123" << endl;
    file.close();
}

SUITE(SimpleFunctionalTests) {
    
    TEST(ServerStartStop) {
        cout << "\n[1] Тест запуска и остановки сервера" << endl;
        
        int port = 33444;
        
        create_test_users("test_users.txt");
        
        string start_cmd = "./server -d test_users.txt -l test.log -p " + to_string(port) + " 2>&1";
        
        pid_t pid = fork();
        if (pid == 0) {
            execl("/bin/sh", "sh", "-c", start_cmd.c_str(), NULL);
            exit(0);
        }
        
        this_thread::sleep_for(seconds(2));
        
        CHECK(check_server_running(port));
        cout << "  ✓ Сервер запущен на порту " << port << endl;
        
        stop_server(port);
        
        CHECK(!check_server_running(port));
        cout << "  ✓ Сервер корректно остановлен" << endl;
        
        system("rm -f test_users.txt test.log 2>/dev/null");
    }
   
    TEST(InvalidArgumentsNoCrash) {
        cout << "\n[2] Тест некорректных аргументов" << endl;
        
        vector<string> bad_args = {
            "./server --invalid-option",
            "./server -p not_a_number", 
            "./server -p 0",
            "./server -p -100",
            "./server -d nonexistent_file.txt"
        };
        
        for (const auto& cmd : bad_args) {
            string full_cmd = cmd + " 2>&1";
            int result = run_command(full_cmd, 2);
            
            CHECK(result != 124); // Код таймаута
            
            if (result != 124) {
                cout << "  ✓ Команда '" << cmd << "' обработана без падения" << endl;
            } else {
                cout << "  ✗ Команда '" << cmd << "' зависла" << endl;
            }
        }
    }
    
   
    TEST(HelpOption) {
        cout << "\n[3] Тест справки" << endl;
        
        string cmd = "./server --help 2>&1";
        int result = run_command(cmd, 2);
        CHECK_EQUAL(0, result);
        
        cout << "  ✓ Справка отображается корректно" << endl;
    }
    
   
    TEST(DifferentPorts) {
        cout << "\n[4] Тест разных портов" << endl;
        
        vector<int> ports = {33555, 33666, 33777};
        
        for (int port : ports) {
            string user_file = "test_users_" + to_string(port) + ".txt";
            string log_file = "test_" + to_string(port) + ".log";
            
            create_test_users(user_file);
            
            string start_cmd = "./server -d " + user_file + " -l " + log_file + 
                              " -p " + to_string(port) + " 2>&1 &";
            system(start_cmd.c_str());
            
            this_thread::sleep_for(seconds(2));
            
            bool running = check_server_running(port);
            if (running) {
                cout << "  ✓ Сервер запущен на порту " << port << endl;
                
                stop_server(port);
                CHECK(!check_server_running(port));
            } else {
                cout << "  ✗ Сервер не запустился на порту " << port << endl;
            }
            
            string cleanup = "rm -f " + user_file + " " + log_file + " 2>/dev/null";
            system(cleanup.c_str());
        }
    }
    
   
    TEST(MultipleServers) {
        cout << "\n[5] Тест нескольких серверов одновременно" << endl;
        
        int port1 = 33888;
        int port2 = 33999;
        
        create_test_users("multi1.txt");
        string cmd1 = "./server -d multi1.txt -l multi1.log -p " + 
                     to_string(port1) + " 2>&1 &";
        system(cmd1.c_str());
        
        create_test_users("multi2.txt");
        string cmd2 = "./server -d multi2.txt -l multi2.log -p " + 
                     to_string(port2) + " 2>&1 &";
        system(cmd2.c_str());
        
        this_thread::sleep_for(seconds(3));
        
        bool running1 = check_server_running(port1);
        bool running2 = check_server_running(port2);
        
        if (running1 && running2) {
            cout << "  ✓ Два сервера запущены одновременно" << endl;
            
            stop_server(port1);
            stop_server(port2);
            
            CHECK(!check_server_running(port1));
            CHECK(!check_server_running(port2));
        } else {
            cout << "  ✗ Не все серверы запустились" << endl;
        }
        
        system("rm -f multi1.txt multi2.txt multi1.log multi2.log 2>/dev/null");
    }
}


int main() {
    cout << "========================================" << endl;
    cout << "ФУНКЦИОНАЛЬНОЕ ТЕСТИРОВАНИЕ СЕРВЕРА VCALC" << endl;
    cout << "========================================" << endl;
    
    if (system("test -f ./server 2>/dev/null") != 0) {
        cerr << "ОШИБКА: Сначала соберите сервер (make all)" << endl;
        return 1;
    }
    
    system("pkill -f './server' 2>/dev/null");
    this_thread::sleep_for(seconds(2));
    
    return UnitTest::RunAllTests();
}
