/**
 * @file test_auth.cpp
 * @brief Тесты аутентификации с использованием UnitTest++
 */

#include <UnitTest++/UnitTest++.h>
#include <vector>
#include <string>
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

SUITE(AuthTests) {
    std::vector<std::pair<std::string,std::string>> users = {
        {"user", "P@ssW0rd"},
        {"admin", "Admin123"},
        {"test", "Test456"}
    };
    
    // Тест 1: Правильная аутентификация
    TEST(CorrectAuthentication) {
        std::string login = "user";
        std::string salt = "A1B2C3D4E5F67890";
        std::string password = "P@ssW0rd";
        std::string data = salt + password;
        
        uint8_t digest[32];
        sha256((uint8_t*)data.c_str(), data.size(), digest);
        
        char hex[65];
        for (int i = 0; i < 32; i++) {
            sprintf(hex + i*2, "%02X", digest[i]);
        }
        
        CHECK(checkAuth(login, salt, std::string(hex), users));
    }
    
    // Тест 2: Неправильный пароль
    TEST(WrongPassword) {
        std::string wrong_hash = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
        CHECK(!checkAuth("user", "A1B2C3D4E5F67890", wrong_hash, users));
    }
    
    // Тест 3: Несуществующий пользователь
    TEST(NonExistentUser) {
        std::string dummy_hash = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
        CHECK(!checkAuth("nonexistent", "A1B2C3D4E5F67890", dummy_hash, users));
    }
    
    // Тест 4: Пустая база пользователей
    TEST(EmptyUserDatabase) {
        std::vector<std::pair<std::string,std::string>> empty_users;
        std::string dummy_hash = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
        CHECK(!checkAuth("user", "A1B2C3D4E5F67890", dummy_hash, empty_users));
    }
    
    // Тест 5: Несколько пользователей
    TEST(MultipleUsers) {
        std::vector<std::pair<std::string,std::string>> multi_users = {
            {"alice", "password1"},
            {"bob", "password2"},
            {"charlie", "password3"}
        };
        
        bool all_correct = true;
        for (const auto& [l, p] : multi_users) {
            std::string s = "SALT1234567890AB";
            std::string d = s + p;
            uint8_t dig[32];
            sha256((uint8_t*)d.c_str(), d.size(), dig);
            char h[65];
            for (int i = 0; i < 32; i++) {
                sprintf(h + i*2, "%02X", dig[i]);
            }
            if (!checkAuth(l, s, h, multi_users)) {
                all_correct = false;
                break;
            }
        }
        CHECK(all_correct);
    }
    
    // Тест 6: Формат соли
    TEST(SaltFormat) {
        std::string salt = "0123456789ABCDEF";
        CHECK_EQUAL(16, salt.length());
        
        for (char c : salt) {
            CHECK((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
        }
    }
    
    // Тест 7: Формат хэша
    TEST(HashFormat) {
        std::string hash = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
        CHECK_EQUAL(64, hash.length());
        
        for (char c : hash) {
            CHECK((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
        }
    }
    
    // Тест 8: Разная соль для одного пароля
    TEST(DifferentSaltDifferentHash) {
        std::string pass = "mypassword";
        std::string salt1 = "1111111111111111";
        std::string salt2 = "2222222222222222";
        
        std::string d1 = salt1 + pass;
        std::string d2 = salt2 + pass;
        
        uint8_t dig1[32], dig2[32];
        sha256((uint8_t*)d1.c_str(), d1.size(), dig1);
        sha256((uint8_t*)d2.c_str(), d2.size(), dig2);
        
        CHECK(memcmp(dig1, dig2, 32) != 0);
    }
    
    // Тест 9: Одинаковая соль и пароль дают одинаковый хэш
    TEST(SameInputSameHash) {
        std::string pass = "password123";
        std::string salt = "3333333333333333";
        std::string d1 = salt + pass;
        std::string d2 = salt + pass;
        
        uint8_t dig1[32], dig2[32];
        sha256((uint8_t*)d1.c_str(), d1.size(), dig1);
        sha256((uint8_t*)d2.c_str(), d2.size(), dig2);
        
        CHECK_ARRAY_EQUAL(dig1, dig2, 32);
    }
    
    // Тест 10: Чувствительность к регистру пароля
    TEST(CaseSensitivePassword) {
        std::string pass_lower = "password";
        std::string pass_upper = "PASSWORD";
        std::string salt = "4444444444444444";
        
        std::string d1 = salt + pass_lower;
        std::string d2 = salt + pass_upper;
        
        uint8_t dig1[32], dig2[32];
        sha256((uint8_t*)d1.c_str(), d1.size(), dig1);
        sha256((uint8_t*)d2.c_str(), d2.size(), dig2);
        
        CHECK(memcmp(dig1, dig2, 32) != 0);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
