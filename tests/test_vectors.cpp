#include <cassert>
#include <vector>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <cmath>

float calculateSumOfSquares(const std::vector<float>& vec) {
    float sum = 0.0f;
    for (float val : vec) {
        sum += val * val;
    }
    return sum;
}

uint32_t floatToLittleEndian(float f) {
    uint32_t bits;
    memcpy(&bits, &f, sizeof(float));
    uint8_t* bytes = (uint8_t*)&bits;
    uint32_t result = 0;
    result |= (bytes[0] << 0);
    result |= (bytes[1] << 8);
    result |= (bytes[2] << 16);
    result |= (bytes[3] << 24);
    return result;
}

float littleEndianToFloat(uint32_t val) {
    uint8_t* bytes = (uint8_t*)&val;
    uint32_t bits = 0;
    bits |= (bytes[0] << 0);
    bits |= (bytes[1] << 8);
    bits |= (bytes[2] << 16);
    bits |= (bytes[3] << 24);
    float f;
    memcpy(&f, &bits, sizeof(float));
    return f;
}

void run_vector_tests() {
    int passed = 0;
    int total = 0;
    
    std::cout << "Тесты вычисления векторов:\n";
    
    // Тест 1: Простой вектор
    total++;
    std::vector<float> vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    float result1 = calculateSumOfSquares(vec1);
    if (fabs(result1 - 30.0f) < 0.0001f) {
        std::cout << "  OK  - Простой вектор (1,2,3,4) = " << result1 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Простой вектор: " << result1 << " (ожидалось 30)\n";
    }
    
    // Тест 2: Отрицательные значения
    total++;
    std::vector<float> vec2 = {-1.0f, -2.0f, 3.0f, 4.0f};
    float result2 = calculateSumOfSquares(vec2);
    if (fabs(result2 - 30.0f) < 0.0001f) {
        std::cout << "  OK  - Отрицательные значения = " << result2 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Отрицательные значения: " << result2 << "\n";
    }
    
    // Тест 3: Пустой вектор
    total++;
    std::vector<float> vec3 = {};
    float result3 = calculateSumOfSquares(vec3);
    if (result3 == 0.0f) {
        std::cout << "  OK  - Пустой вектор = 0\n";
        passed++;
    } else {
        std::cout << "  FAIL - Пустой вектор: " << result3 << "\n";
    }
    
    // Тест 4: Один элемент
    total++;
    std::vector<float> vec4 = {5.0f};
    float result4 = calculateSumOfSquares(vec4);
    if (fabs(result4 - 25.0f) < 0.0001f) {
        std::cout << "  OK  - Один элемент (5) = " << result4 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Один элемент: " << result4 << "\n";
    }
    
    // Тест 5: Большие числа
    total++;
    std::vector<float> vec5 = {100.0f, 200.0f, 300.0f};
    float result5 = calculateSumOfSquares(vec5);
    float expected5 = 10000.0f + 40000.0f + 90000.0f;
    if (fabs(result5 - expected5) < 0.1f) {
        std::cout << "  OK  - Большие числа = " << result5 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Большие числа: " << result5 << "\n";
    }
    
    // Тест 6: Дробные числа
    total++;
    std::vector<float> vec6 = {0.5f, 1.5f, 2.5f};
    float result6 = calculateSumOfSquares(vec6);
    float expected6 = 0.25f + 2.25f + 6.25f;
    if (fabs(result6 - expected6) < 0.0001f) {
        std::cout << "  OK  - Дробные числа = " << result6 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Дробные числа: " << result6 << "\n";
    }
    
    // Тест 7: Конвертация little-endian
    total++;
    float original = 123.456f;
    uint32_t le = floatToLittleEndian(original);
    float converted = littleEndianToFloat(le);
    if (fabs(original - converted) < 0.0001f) {
        std::cout << "  OK  - Конвертация endian: " << original << " -> " << converted << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Конвертация endian\n";
    }
    
    // Тест 8: Все нули
    total++;
    std::vector<float> vec8(5, 0.0f);
    float result8 = calculateSumOfSquares(vec8);
    if (result8 == 0.0f) {
        std::cout << "  OK  - Все нули = 0\n";
        passed++;
    } else {
        std::cout << "  FAIL - Все нули: " << result8 << "\n";
    }
    
    // Тест 9: Очень маленькие числа
    total++;
    std::vector<float> vec9 = {0.001f, 0.002f, 0.003f};
    float result9 = calculateSumOfSquares(vec9);
    float expected9 = 0.000001f + 0.000004f + 0.000009f;
    if (fabs(result9 - expected9) < 1e-10f) {
        std::cout << "  OK  - Очень маленькие числа ≈ " << result9 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Очень маленькие числа\n";
    }
    
    // Тест 10: Смешанные положительные и отрицательные
    total++;
    std::vector<float> vec10 = {-2.0f, 0.0f, 2.0f, -4.0f, 4.0f};
    float result10 = calculateSumOfSquares(vec10);
    float expected10 = 4.0f + 0.0f + 4.0f + 16.0f + 16.0f;
    if (fabs(result10 - expected10) < 0.0001f) {
        std::cout << "  OK  - Смешанные значения = " << result10 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - Смешанные значения: " << result10 << "\n";
    }
    
    // Тест 11: Очень большое количество элементов
    total++;
    std::vector<float> vec11;
    for (int i = 0; i < 100; i++) {
        vec11.push_back(1.0f);
    }
    float result11 = calculateSumOfSquares(vec11);
    if (fabs(result11 - 100.0f) < 0.0001f) {
        std::cout << "  OK  - 100 единиц = " << result11 << "\n";
        passed++;
    } else {
        std::cout << "  FAIL - 100 единиц: " << result11 << "\n";
    }
    
    // Тест 12: Конвертация отрицательного числа
    total++;
    float negative = -123.456f;
    uint32_t le_neg = floatToLittleEndian(negative);
    float converted_neg = littleEndianToFloat(le_neg);
    if (fabs(negative - converted_neg) < 0.0001f) {
        std::cout << "  OK  - Конвертация отрицательного числа\n";
        passed++;
    } else {
        std::cout << "  FAIL - Конвертация отрицательного числа\n";
    }
    
    std::cout << "\nИтого векторы: " << passed << "/" << total << " тестов пройдено\n";
}

int main() {
    run_vector_tests();
    return 0;
}
