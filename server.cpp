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

// Функция логирования
void logMsg(const string &file, const string &msg) {
    ofstream f(file, ios::app);
    if (!f) return;
    time_t t = time(nullptr);
    tm *tm = localtime(&t);
    f << put_time(tm, "%Y-%m-%d %H:%M:%S") << " | " << msg << endl;
}

// Загрузка пользователей
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

// Проверка аутентификации
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

// Чтение точного количества байт
bool readAll(int sock, void *buf, size_t len) {
    size_t got = 0;
    while (got < len) {
        ssize_t n = read(sock, (char*)buf + got, len - got);
        if (n <= 0) return false;
        got += n;
    }
    return true;
}

// Запись точного количества байт
bool writeAll(int sock, const void *buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = write(sock, (char*)buf + sent, len - sent);
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

// Little-endian конверсия
uint32_t readLittleEndian32(const uint8_t* bytes) {
    return (bytes[0] << 0) | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
}

void writeLittleEndian32(uint32_t value, uint8_t* bytes) {
    bytes[0] = (value >> 0) & 0xFF;
    bytes[1] = (value >> 8) & 0xFF;
    bytes[2] = (value >> 16) & 0xFF;
    bytes[3] = (value >> 24) & 0xFF;
}

// Обработка клиента
void handleClient(int sock, const vector<pair<string,string>> &users, const string &logFile) {
    char clientIP[INET_ADDRSTRLEN];
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    if (getpeername(sock, (sockaddr*)&client_addr, &addr_len) == 0) {
        inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN);
        logMsg(logFile, "Клиент подключен: " + string(clientIP));
    }
    
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
    if (authStr.length() < 84) { 
        writeAll(sock, "ERR", 3); 
        logMsg(logFile, "Неверный формат аутентификации");
        close(sock); 
        return; 
    }
    
    string login = authStr.substr(0, 4);
    string salt = authStr.substr(4, 16);
    string hash = authStr.substr(20, 64);
    
    logMsg(logFile, "Аутентификация: " + login);
    
    if (!checkAuth(login, salt, hash, users)) {
        writeAll(sock, "ERR", 3);
        logMsg(logFile, "Аутентификация отклонена: " + login);
        close(sock);
        return;
    }
    
    writeAll(sock, "OK", 2);
    logMsg(logFile, "Клиент аутентифицирован: " + login);
    
    // Вычисления
    uint8_t buffer[4];
    if (!readAll(sock, buffer, 4)) { 
        logMsg(logFile, "Ошибка чтения количества векторов");
        close(sock); 
        return; 
    }
    
    uint32_t numVectors = readLittleEndian32(buffer);
    logMsg(logFile, "Обработка " + to_string(numVectors) + " векторов");
    
    // Обрабатываем векторы
    for (uint32_t i = 0; i < numVectors; i++) {
        // Читаем размер вектора
        if (!readAll(sock, buffer, 4)) {
            logMsg(logFile, "Ошибка чтения размера вектора");
            close(sock);
            return;
        }
        
        uint32_t vectorSize = readLittleEndian32(buffer);
        
        // Читаем элементы вектора
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
        
        // Отправляем результат
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

int main(int argc, char *argv[]) {
    string userFile = "users.txt";
    string logFile = "server.log";
    int port = 33333;
    
    po::options_description desc("Сервер vcalc");
    desc.add_options()
        ("help,h", "Справка")
        ("database,d", po::value<string>(&userFile)->default_value("users.txt"), "Файл пользователей")
        ("log,l", po::value<string>(&logFile)->default_value("server.log"), "Файл логов")
        ("port,p", po::value<int>(&port)->default_value(33333), "Порт");
    
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (exception &e) {
        cerr << "Ошибка: " << e.what() << endl << desc << endl;
        return 1;
    }
    
    if (vm.count("help")) {
        cout << desc << endl;
        return 0;
    }
    
    // Старт сервера
    logMsg(logFile, "=== Запуск сервера ===");
    logMsg(logFile, "Порт: " + to_string(port));
    
    // Загрузка пользователей
    auto users = loadUsers(userFile);
    if (users.empty()) {
        cerr << "Нет пользователей в " << userFile << endl;
        return 1;
    }
    
    logMsg(logFile, "Загружено пользователей: " + to_string(users.size()));
    
    // Создание сокета
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(sock); return 1; }
    if (listen(sock, 5) < 0) { perror("listen"); close(sock); return 1; }
    
    cout << "Сервер запущен на порту " << port << endl;
    logMsg(logFile, "Сервер запущен, ожидание подключений");
    
    // Основной цикл
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
