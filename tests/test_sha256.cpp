#include <cassert>
#include <cstring>
#include <iostream>
#include "../sha256.hpp"

void run_sha256_tests() {
    int passed = 0;
    int total = 0;
    
    std::cout << "Тесты SHA-256:\n";
    
    // Тест 1: Пустая строка
    total++;
    const char* empty = "";
    uint8_t hash1[32];
    sha256((uint8_t*)empty, 0, hash1);
    const char* expected1 = "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855";
    char hex1[65];
    for (int i = 0; i < 32; i++) sprintf(hex1 + i*2, "%02X", hash1[i]);
    if (strcmp(hex1, expected1) == 0) {
        std::cout << "  OK  - Пустая строка\n";
        passed++;
    } else {
        std::cout << "  FAIL - Пустая строка\n";
    }
    
    // Тест 2: Строка "abc"
    total++;
    const char* abc = "abc";
    uint8_t hash2[32];
    sha256((uint8_t*)abc, 3, hash2);
    const char* expected2 = "BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD";
    char hex2[65];
    for (int i = 0; i < 32; i++) sprintf(hex2 + i*2, "%02X", hash2[i]);
    if (strcmp(hex2, expected2) == 0) {
        std::cout << "  OK  - Строка 'abc'\n";
        passed++;
    } else {
        std::cout << "  FAIL - Строка 'abc'\n";
    }
    
    // Тест 3: Длинная строка
    total++;
    const char* long_text = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    uint8_t hash3[32];
    sha256((uint8_t*)long_text, strlen(long_text), hash3);
    const char* expected3 = "CF5B16A778AF8380036CE59E7B0492370B249B11E8F07A51AFAC45037AFEE9D1";
    char hex3[65];
    for (int i = 0; i < 32; i++) sprintf(hex3 + i*2, "%02X", hash3[i]);
    if (strcmp(hex3, expected3) == 0) {
        std::cout << "  OK  - Длинная строка\n";
        passed++;
    } else {
        std::cout << "  FAIL - Длинная строка\n";
    }
    
    // Тест 4: Консистентность
    total++;
    const char* data = "test data";
    uint8_t hash4[32], hash5[32];
    sha256((uint8_t*)data, strlen(data), hash4);
    sha256((uint8_t*)data, strlen(data), hash5);
    if (memcmp(hash4, hash5, 32) == 0) {
        std::cout << "  OK  - Консистентность вычислений\n";
        passed++;
    } else {
        std::cout << "  FAIL - Консистентность вычислений\n";
    }
    
    // Тест 5: Разные длины данных
    total++;
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
    if (lengths_ok) {
        std::cout << "  OK  - Разные длины входных данных\n";
        passed++;
    } else {
        std::cout << "  FAIL - Разные длины входных данных\n";
    }
    
    // Тест 6: Разные данные дают разные хэши
    total++;
    const char* str1 = "hello";
    const char* str2 = "world";
    uint8_t h1[32], h2[32];
    sha256((uint8_t*)str1, strlen(str1), h1);
    sha256((uint8_t*)str2, strlen(str2), h2);
    if (memcmp(h1, h2, 32) != 0) {
        std::cout << "  OK  - Разные данные -> разные хэши\n";
        passed++;
    } else {
        std::cout << "  FAIL - Разные данные -> разные хэши\n";
    }
    
    // Тест 7: Один символ разницы меняет хэш
    total++;
    const char* str3 = "test1";
    const char* str4 = "test2";
    uint8_t h3[32], h4[32];
    sha256((uint8_t*)str3, strlen(str3), h3);
    sha256((uint8_t*)str4, strlen(str4), h4);
    if (memcmp(h3, h4, 32) != 0) {
        std::cout << "  OK  - Один символ разницы -> другой хэш\n";
        passed++;
    } else {
        std::cout << "  FAIL - Один символ разницы -> другой хэш\n";
    }
    
    // Тест 8: Хэш не нулевой для непустых данных
    total++;
    const char* non_empty = "x";
    uint8_t h5[32];
    sha256((uint8_t*)non_empty, strlen(non_empty), h5);
    bool all_zeros = true;
    for (int i = 0; i < 32; i++) {
        if (h5[i] != 0) {
            all_zeros = false;
            break;
        }
    }
    if (!all_zeros) {
        std::cout << "  OK  - Хэш не нулевой для непустых данных\n";
        passed++;
    } else {
        std::cout << "  FAIL - Хэш не нулевой для непустых данных\n";
    }
    
    std::cout << "\nИтого SHA-256: " << passed << "/" << total << " тестов пройдено\n";
}

int main() {
    run_sha256_tests();
    return 0;
}
