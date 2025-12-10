/**
 * @file test_vectors.cpp
 * @brief Тесты вычисления векторов с использованием UnitTest++
 */

#include <UnitTest++/UnitTest++.h>
#include <vector>
#include <cstring>
#include <cmath>
#include <cstdint>  // Добавил этот include

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

SUITE(VectorTests) {
    // Тест 1: Простой вектор
    TEST(SimpleVector) {
        std::vector<float> vec = {1.0f, 2.0f, 3.0f, 4.0f};
        float result = calculateSumOfSquares(vec);
        CHECK_CLOSE(30.0f, result, 0.0001f);
    }
    
    // Тест 2: Отрицательные значения
    TEST(NegativeValues) {
        std::vector<float> vec = {-1.0f, -2.0f, 3.0f, 4.0f};
        float result = calculateSumOfSquares(vec);
        CHECK_CLOSE(30.0f, result, 0.0001f);
    }
    
    // Тест 3: Пустой вектор
    TEST(EmptyVector) {
        std::vector<float> vec = {};
        float result = calculateSumOfSquares(vec);
        CHECK_EQUAL(0.0f, result);
    }
    
    // Тест 4: Один элемент
    TEST(SingleElement) {
        std::vector<float> vec = {5.0f};
        float result = calculateSumOfSquares(vec);
        CHECK_CLOSE(25.0f, result, 0.0001f);
    }
    
    // Тест 5: Большие числа
    TEST(LargeNumbers) {
        std::vector<float> vec = {100.0f, 200.0f, 300.0f};
        float result = calculateSumOfSquares(vec);
        float expected = 10000.0f + 40000.0f + 90000.0f;
        CHECK_CLOSE(expected, result, 0.1f);
    }
    
    // Тест 6: Дробные числа
    TEST(FractionalNumbers) {
        std::vector<float> vec = {0.5f, 1.5f, 2.5f};
        float result = calculateSumOfSquares(vec);
        float expected = 0.25f + 2.25f + 6.25f;
        CHECK_CLOSE(expected, result, 0.0001f);
    }
    
    // Тест 7: Конвертация endian
    TEST(EndianConversion) {
        float original = 123.456f;
        uint32_t le = floatToLittleEndian(original);
        float converted = littleEndianToFloat(le);
        CHECK_CLOSE(original, converted, 0.0001f);
    }
    
    // Тест 8: Все нули
    TEST(AllZeros) {
        std::vector<float> vec(5, 0.0f);
        float result = calculateSumOfSquares(vec);
        CHECK_EQUAL(0.0f, result);
    }
    
    // Тест 9: Очень маленькие числа
    TEST(VerySmallNumbers) {
        std::vector<float> vec = {0.001f, 0.002f, 0.003f};
        float result = calculateSumOfSquares(vec);
        float expected = 0.000001f + 0.000004f + 0.000009f;
        CHECK_CLOSE(expected, result, 1e-10f);
    }
    
    // Тест 10: Смешанные значения
    TEST(MixedPositiveNegative) {
        std::vector<float> vec = {-2.0f, 0.0f, 2.0f, -4.0f, 4.0f};
        float result = calculateSumOfSquares(vec);
        float expected = 4.0f + 0.0f + 4.0f + 16.0f + 16.0f;
        CHECK_CLOSE(expected, result, 0.0001f);
    }
    
    // Тест 11: Много элементов
    TEST(ManyElements) {
        std::vector<float> vec;
        for (int i = 0; i < 100; i++) {
            vec.push_back(1.0f);
        }
        float result = calculateSumOfSquares(vec);
        CHECK_CLOSE(100.0f, result, 0.0001f);
    }
    
    // Тест 12: Конвертация отрицательного числа
    TEST(NegativeEndianConversion) {
        float negative = -123.456f;
        uint32_t le = floatToLittleEndian(negative);
        float converted = littleEndianToFloat(le);
        CHECK_CLOSE(negative, converted, 0.0001f);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
