/**
 * @file server.cpp
 * @brief Сервер аутентификации и вычислений vcalc
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <ctime>
#include <iomanip>
#include "sha256.hpp"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

void logMsg(const string &file, const string &msg) {
    ofstream f(file, ios::app);
    if (!f) return;
    time_t t = time(nullptr);
    tm *tm = localtime(&t);
    f << put_time(tm, "%Y-%m-%d %H:%M:%S") << " | " << msg << endl;
}

vector<pair<string,string>> loadUsers(const string &file) {
    vector<pair<string,string>> users;
    ifstream f(file);
    string line;
    while (getline(f, line)) {
        size_t p = line.find(':');
        if (p != string::npos)
            users.push_back({line.substr(0, p), line.substr(p + 1)});
    }
    return users;
}

bool checkAuth(const string &login, const string &salt, const string &hash, 
               const vector<pair<string,string>> &users) {
    for (const auto &[l, p] : users) {
        if (l == login) {
            string data = salt + p;
            uint8_t digest[32];
            sha256((uint8_t*)data.c_str(), data.size(), digest);
            
            char hex[65];
            for (int i = 0; i < 32; i++) sprintf(hex + i*2, "%02X", digest[i]);
            hex[64] = '\0';
            
            return string(hex) == hash;
        }
    }
    return false;
}

bool readAll(int sock, void *buf, size_t len) {
    size_t got = 0;
    while (got < len) {
        ssize_t n = read(sock, (char*)buf + got, len - got);
        if (n <= 0) return false;
        got += n;
    }
    return true;
}

bool writeAll(int sock, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(sock, (char*)buf + sent, len - sent);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

uint32_t readLittleEndian32(const uint8_t* bytes) {
    return (bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

void writeLittleEndian32(uint32_t value, uint8_t* bytes) {
    bytes[0] = (value >> 0) & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
}

/**
 * @brief Проверяет, является ли строка шестнадцатеричной
 */
bool isHexString(const string& str) {
    for (char c : str) {
        if (!((c >= '0' && c <= '9') || 
              (c >= 'A' && c <= 'F') || 
              (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Парсит строку аутентификации, поддерживая оба формата
 * @param authStr Строка аутентификации
 * @param login [out] Извлеченный логин
 * @param salt [out] Извлеченная соль (16 hex символов)
 * @param hash [out] Извлеченный хэш (64 hex символа)
 * @return true если успешно, false если ошибка
 */
bool parseAuthString(const string& authStr, string& login, string& salt, string& hash) {
    // Подсчитываем количество двоеточий
    size_t colonCount = 0;
    for (char c : authStr) {
        if (c == ':') colonCount++;
    }
    
    //логин:соль:хэш
    if (colonCount == 2) {
        size_t firstColon = authStr.find(':');
        size_t secondColon = authStr.find(':', firstColon + 1);
        
        if (firstColon == string::npos || secondColon == string::npos || 
            firstColon == 0 || secondColon == firstColon + 1) {
            return false;
        }
        
        login = authStr.substr(0, firstColon);
        salt = authStr.substr(firstColon + 1, secondColon - firstColon - 1);
        hash = authStr.substr(secondColon + 1);
    }
    //логин(4) + соль(16) + хэш(64)
    else if (colonCount == 0 && authStr.length() == 84) {
        login = authStr.substr(0, 4);
        salt = authStr.substr(4, 16);
        hash = authStr.substr(20, 64);
    }
    else {
        return false;
    }
    
    // Проверка форматов
    if (login.empty() || salt.length() != 16 || hash.length() != 64) {
        return false;
    }
    
    // Проверка hex формата
    if (!isHexString(salt) || !isHexString(hash)) {
        return false;
    }
    
    return true;
}

void handleClient(int sock, const vector<pair<string,string>> &users, const string &logFile) {
    logMsg(logFile, "Клиент подключен");
    
    // Аутентификация
    char auth[256];
    ssize_t n = read(sock, auth, sizeof(auth)-1);
    if (n <= 0) { 
        logMsg(logFile, "Ошибка чтения аутентификации");
        close(sock); 
        return; 
    }
    auth[n] = '\0';
    
    string authStr(auth);
    
    // Парсинг строки аутентификации
    string login, salt, hash;
    if (!parseAuthString(authStr, login, salt, hash)) {
        writeAll(sock, "ERR", 3); 
        logMsg(logFile, "Неверный формат аутентификации: " + 
                        (authStr.length() > 50 ? authStr.substr(0, 50) + "..." : authStr));
        close(sock); 
        return; 
    }
    
    // Логируем формат (новый или старый)
    size_t colonCount = 0;
    for (char c : authStr) if (c == ':') colonCount++;
    string format = (colonCount == 2) ? "новый (логин:соль:хэш)" : 
                   (colonCount == 0 && authStr.length() == 84) ? "старый (логин4+соль16+хэш64)" : "неизвестный";
    
    logMsg(logFile, "Аутентификация: " + login + " (формат: " + format + ")");
    
    if (!checkAuth(login, salt, hash, users)) {
        writeAll(sock, "ERR", 3);
        logMsg(logFile, "Аутентификация отклонена: " + login);
        close(sock);
        return;
    }
    
    writeAll(sock, "OK", 2);
    logMsg(logFile, "Клиент аутентифицирован: " + login);
    
    // Обработка векторов
    uint8_t buffer[4];
    if (!readAll(sock, buffer, 4)) { 
        logMsg(logFile, "Ошибка чтения количества векторов");
        close(sock); 
        return; 
    }
    
    uint32_t numVectors = readLittleEndian32(buffer);
    
    for (uint32_t i = 0; i < numVectors; i++) {
        if (!readAll(sock, buffer, 4)) {
            logMsg(logFile, "Ошибка чтения размера вектора");
            close(sock);
            return;
        }
        
        uint32_t vectorSize = readLittleEndian32(buffer);
        float sum = 0.0f;
        
        for (uint32_t j = 0; j < vectorSize; j++) {
            if (!readAll(sock, buffer, 4)) {
                logMsg(logFile, "Ошибка чтения данных вектора");
                close(sock);
                return;
            }
            
            uint32_t val = readLittleEndian32(buffer);
            float f;
            memcpy(&f, &val, sizeof(float));
            sum += f * f;
        }
        
        logMsg(logFile, "Вектор " + to_string(i+1) + ": сумма квадратов = " + to_string(sum));
        
        uint32_t resultBits;
        memcpy(&resultBits, &sum, sizeof(float));
        uint8_t resultBuffer[4];
        writeLittleEndian32(resultBits, resultBuffer);
        
        if (!writeAll(sock, resultBuffer, 4)) {
            logMsg(logFile, "Ошибка отправки результата");
            close(sock);
            return;
        }
    }
    
    logMsg(logFile, "Вычисления завершены для " + to_string(numVectors) + " векторов");
    close(sock);
}

// Основная логика сервера
#ifndef TEST_MODE
int main(int argc, char *argv[]) {
#else
extern "C" int main_server(int argc, char *argv[]) {
#endif
    string userFile = "users.txt";
    string logFile = "server.log";
    int port = 33333;
    
    po::options_description desc("Сервер vcalc v1.0\n\nИспользование: server [options]\n\nДоступные опции");
    desc.add_options()
        ("help,h", "Показать справку")
        ("database,d", po::value<string>(&userFile)->default_value("users.txt"), "Файл с базой пользователей")
        ("log,l", po::value<string>(&logFile)->default_value("server.log"), "Файл логов")
        ("port,p", po::value<int>(&port)->default_value(33333), "Порт сервера");
    
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (exception &e) {
        #ifdef TEST_MODE
        return 1;
        #else
        cerr << "Ошибка: " << e.what() << endl << desc << endl;
        return 1;
        #endif
    }
    
    if (vm.count("help")) {
        #ifdef TEST_MODE
        return 0;
        #else
        cout << desc << endl;
        return 0;
        #endif
    }
    
    if (port <= 0 || port > 65535) {
        #ifdef TEST_MODE
        return 1;
        #else
        cerr << "Ошибка: Порт должен быть в диапазоне 1-65535" << endl;
        logMsg(logFile, "ОШИБКА: Неверный порт " + to_string(port));
        return 1;
        #endif
    }
    
    #ifndef TEST_MODE
    logMsg(logFile, "=== Запуск сервера ===");
    #endif
    
    auto users = loadUsers(userFile);
    if (users.empty()) {
        #ifdef TEST_MODE
        return 1;
        #else
        cerr << "Ошибка: Нет пользователей" << endl;
        return 1;
        #endif
    }
    
    #ifdef TEST_MODE
    return 0;
    #endif
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { 
        perror("Ошибка сокета"); 
        return 1; 
    }
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { 
        perror("Ошибка привязки"); 
        close(sock); 
        return 1; 
    }
    
    if (listen(sock, 5) < 0) { 
        perror("Ошибка прослушивания"); 
        close(sock); 
        return 1; 
    }
    
    cout << "Сервер запущен на порту " << port << endl;
    
    while (true) {
        sockaddr_in client;
        socklen_t len = sizeof(client);
        int clientSock = accept(sock, (sockaddr*)&client, &len);
        if (clientSock < 0) continue;
        handleClient(clientSock, users, logFile);
    }
    
    close(sock);
    return 0;
}
