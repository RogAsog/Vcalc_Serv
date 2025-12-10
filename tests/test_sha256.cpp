/**
 * @file test_sha256.cpp
 * @brief Тесты SHA-256 с использованием UnitTest++
 */

#include <UnitTest++/UnitTest++.h>
#include <cstring>
#include "../sha256.hpp"

SUITE(SHA256Tests) {
    // Тест 1: Пустая строка
    TEST(EmptyString) {
        const char* empty = "";
        uint8_t hash[32];
        sha256((uint8_t*)empty, 0, hash);
        
        const char* expected = "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855";
        char hex[65];
        for (int i = 0; i < 32; i++) {
            sprintf(hex + i*2, "%02X", hash[i]);
        }
        
        CHECK_EQUAL(std::string(expected), std::string(hex));
    }
    
    // Тест 2: Строка "abc"
    TEST(ABCString) {
        const char* abc = "abc";
        uint8_t hash[32];
        sha256((uint8_t*)abc, 3, hash);
        
        const char* expected = "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD";
        char hex[65];
        for (int i = 0; i < 32; i++) {
            sprintf(hex + i*2, "%02X", hash[i]);
        }
        
        CHECK_EQUAL(std::string(expected), std::string(hex));
    }
    
    // Тест 3: Длинная строка
    TEST(LongString) {
        const char* long_text = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
        uint8_t hash[32];
        sha256((uint8_t*)long_text, strlen(long_text), hash);
        
        const char* expected = "CF5B16A778AF8380036CE59E7B0492370B249B11E8F07A51AFAC45037AFEE9D1";
        char hex[65];
        for (int i = 0; i < 32; i++) {
            sprintf(hex + i*2, "%02X", hash[i]);
        }
        
        CHECK_EQUAL(std::string(expected), std::string(hex));
    }
    
    // Тест 4: Консистентность
    TEST(Consistency) {
        const char* data = "test data";
        uint8_t hash1[32], hash2[32];
        
        sha256((uint8_t*)data, strlen(data), hash1);
        sha256((uint8_t*)data, strlen(data), hash2);
        
        CHECK_ARRAY_EQUAL(hash1, hash2, 32);
    }
    
    // Тест 5: Разные длины данных
    TEST(DifferentLengths) {
        bool lengths_ok = true;
        const char* test_strings[] = {"a", "ab", "abc", "abcd", "abcde", "abcdef"};
        
        for (const char* str : test_strings) {
            uint8_t h[32];
            sha256((uint8_t*)str, strlen(str), h);
            
            bool all_zero = true;
            for (int i = 0; i < 32; i++) {
                if (h[i] != 0) {
                    all_zero = false;
                    break;
                }
            }
            
            if (all_zero) {
                lengths_ok = false;
                break;
            }
        }
        
        CHECK(lengths_ok);
    }
    
    // Тест 6: Разные данные дают разные хэши
    TEST(DifferentDataDifferentHash) {
        const char* str1 = "hello";
        const char* str2 = "world";
        uint8_t hash1[32], hash2[32];
        
        sha256((uint8_t*)str1, strlen(str1), hash1);
        sha256((uint8_t*)str2, strlen(str2), hash2);
        
        CHECK(memcmp(hash1, hash2, 32) != 0);
    }
    
    // Тест 7: Один символ разницы меняет хэш
    TEST(OneCharacterDifference) {
        const char* str1 = "test1";
        const char* str2 = "test2";
        uint8_t hash1[32], hash2[32];
        
        sha256((uint8_t*)str1, strlen(str1), hash1);
        sha256((uint8_t*)str2, strlen(str2), hash2);
        
        CHECK(memcmp(hash1, hash2, 32) != 0);
    }
    
    // Тест 8: Хэш не нулевой для непустых данных
    TEST(HashNotEmpty) {
        const char* non_empty = "x";
        uint8_t hash[32];
        
        sha256((uint8_t*)non_empty, strlen(non_empty), hash);
        
        bool all_zero = true;
        for (int i = 0; i < 32; i++) {
            if (hash[i] != 0) {
                all_zero = false;
                break;
            }
        }
        
        CHECK(!all_zero);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
