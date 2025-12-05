#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include "../sha256.hpp"

bool checkAuth(const std::string& login, const std::string& salt, 
               const std::string& hash, 
               const std::vector<std::pair<std::string,std::string>>& users) {
    for (const auto& [l, p] : users) {
        if (l == login) {
            std::string data = salt + p;
            uint8_t digest[32];
            sha256((uint8_t*)data.c_str(), data.size(), digest);
            
            char hex[65];
            for (int i = 0; i < 32; i++) {
                sprintf(hex + i*2, "%02X", digest[i]);
            }
            hex[64] = '\0';
            
            return std::string(hex) == hash;
        }
    }
    return false;
}

void run_auth_tests() {
    int passed = 0;
    int total = 0;
    
    std::cout << "Тесты аутентификации:\n";
    
    std::vector<std::pair<std::string,std::string>> users = {
        {"user", "P@ssW0rd"},
        {"admin", "Admin123"},
        {"test", "Test456"}
    };
    
    // Тест 1: Правильная аутентификация
    total++;
    std::string login = "user";
    std::string salt = "A1B2C3D4E5F67890";
    std::string password = "P@ssW0rd";
    std::string data = salt + password;
    uint8_t digest[32];
    sha256((uint8_t*)data.c_str(), data.size(), digest);
    char hex[65];
    for (int i = 0; i < 32; i++) sprintf(hex + i*2, "%02X", digest[i]);
    std::string correct_hash(hex);
    if (checkAuth(login, salt, correct_hash, users)) {
        std::cout << "  OK  - Правильные учетные данные\n";
        passed++;
    } else {
        std::cout << "  FAIL - Правильные учетные данные\n";
    }
    
    // Тест 2: Неправильный пароль
    total++;
    std::string wrong_hash = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
    if (!checkAuth("user", "A1B2C3D4E5F67890", wrong_hash, users)) {
        std::cout << "  OK  - Неправильный пароль\n";
        passed++;
    } else {
        std::cout << "  FAIL - Неправильный пароль\n";
    }
    
    // Тест 3: Несуществующий пользователь
    total++;
    if (!checkAuth("nonexistent", "A1B2C3D4E5F67890", wrong_hash, users)) {
        std::cout << "  OK  - Несуществующий пользователь\n";
        passed++;
    } else {
        std::cout << "  FAIL - Несуществующий пользователь\n";
    }
    
    // Тест 4: Пустая база пользователей
    total++;
    std::vector<std::pair<std::string,std::string>> empty_users;
    if (!checkAuth("user", "A1B2C3D4E5F67890", wrong_hash, empty_users)) {
        std::cout << "  OK  - Пустая база пользователей\n";
        passed++;
    } else {
        std::cout << "  FAIL - Пустая база пользователей\n";
    }
    
    // Тест 5: Несколько пользователей
    total++;
    bool all_correct = true;
    std::vector<std::pair<std::string,std::string>> multi_users = {
        {"alice", "password1"},
        {"bob", "password2"},
        {"charlie", "password3"}
    };
    for (const auto& [l, p] : multi_users) {
        std::string s = "SALT1234567890AB";
        std::string d = s + p;
        uint8_t dig[32];
        sha256((uint8_t*)d.c_str(), d.size(), dig);
        char h[65];
        for (int i = 0; i < 32; i++) sprintf(h + i*2, "%02X", dig[i]);
        if (!checkAuth(l, s, h, multi_users)) {
            all_correct = false;
            break;
        }
    }
    if (all_correct) {
        std::cout << "  OK  - Несколько пользователей в базе\n";
        passed++;
    } else {
        std::cout << "  FAIL - Несколько пользователей в базе\n";
    }
    
    // Тест 6: Формат соли (16 шестнадцатеричных символов)
    total++;
    std::string salt_test = "0123456789ABCDEF";
    bool salt_ok = true;
    if (salt_test.length() != 16) salt_ok = false;
    for (char c : salt_test) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            salt_ok = false;
            break;
        }
    }
    if (salt_ok) {
        std::cout << "  OK  - Формат соли (16 hex символов)\n";
        passed++;
    } else {
        std::cout << "  FAIL - Формат соли\n";
    }
    
    // Тест 7: Формат хэша (64 шестнадцатеричных символа)
    total++;
    std::string hash_test = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
    bool hash_ok = true;
    if (hash_test.length() != 64) hash_ok = false;
    for (char c : hash_test) {
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            hash_ok = false;
            break;
        }
    }
    if (hash_ok) {
        std::cout << "  OK  - Формат хэша (64 hex символа)\n";
        passed++;
    } else {
        std::cout << "  FAIL - Формат хэша\n";
    }
    
    // Тест 8: Разная соль для одного пароля
    total++;
    std::string pass = "mypassword";
    std::string salt1 = "1111111111111111";
    std::string salt2 = "2222222222222222";
    std::string d1 = salt1 + pass;
    std::string d2 = salt2 + pass;
    uint8_t dig1[32], dig2[32];
    sha256((uint8_t*)d1.c_str(), d1.size(), dig1);
    sha256((uint8_t*)d2.c_str(), d2.size(), dig2);
    char h1[65], h2[65];
    for (int i = 0; i < 32; i++) {
        sprintf(h1 + i*2, "%02X", dig1[i]);
        sprintf(h2 + i*2, "%02X", dig2[i]);
    }
    if (strcmp(h1, h2) != 0) {
        std::cout << "  OK  - Разная соль дает разные хэши\n";
        passed++;
    } else {
        std::cout << "  FAIL - Разная соль дает одинаковые хэши\n";
    }
    
    // Тест 9: Одинаковая соль и пароль дают одинаковый хэш
    total++;
    std::string pass2 = "password123";
    std::string salt3 = "3333333333333333";
    std::string d3 = salt3 + pass2;
    std::string d4 = salt3 + pass2;
    uint8_t dig3[32], dig4[32];
    sha256((uint8_t*)d3.c_str(), d3.size(), dig3);
    sha256((uint8_t*)d4.c_str(), d4.size(), dig4);
    char h3[65], h4[65];
    for (int i = 0; i < 32; i++) {
        sprintf(h3 + i*2, "%02X", dig3[i]);
        sprintf(h4 + i*2, "%02X", dig4[i]);
    }
    if (strcmp(h3, h4) == 0) {
        std::cout << "  OK  - Одинаковые данные дают одинаковый хэш\n";
        passed++;
    } else {
        std::cout << "  FAIL - Одинаковые данные дают разный хэш\n";
    }
    
    // Тест 10: Чувствительность к регистру пароля
    total++;
    std::string pass_lower = "password";
    std::string pass_upper = "PASSWORD";
    std::string salt4 = "4444444444444444";
    std::string d5 = salt4 + pass_lower;
    std::string d6 = salt4 + pass_upper;
    uint8_t dig5[32], dig6[32];
    sha256((uint8_t*)d5.c_str(), d5.size(), dig5);
    sha256((uint8_t*)d6.c_str(), d6.size(), dig6);
    char h5[65], h6[65];
    for (int i = 0; i < 32; i++) {
        sprintf(h5 + i*2, "%02X", dig5[i]);
        sprintf(h6 + i*2, "%02X", dig6[i]);
    }
    if (strcmp(h5, h6) != 0) {
        std::cout << "  OK  - Разный регистр пароля дает разные хэши\n";
        passed++;
    } else {
        std::cout << "  FAIL - Разный регистр пароля дает одинаковые хэши\n";
    }
    
    std::cout << "\nИтого аутентификация: " << passed << "/" << total << " тестов пройдено\n";
}

int main() {
    run_auth_tests();
    return 0;
}
