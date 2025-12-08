/**
 * @file sha256.hpp
 * @brief Заголовочный файл для реализации SHA-256
 */

#pragma once
#include <cstdint>
#include <cstddef>

/**
 * @brief Вычисляет хэш SHA-256 для данных
 * @param data Указатель на входные данные
 * @param len Длина данных в байтах
 * @param out Массив для записи результата (32 байта)
 */
void sha256(const uint8_t *data, size_t len, uint8_t out[32]);
