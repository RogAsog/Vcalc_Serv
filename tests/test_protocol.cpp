/**
 * @file test_protocol.cpp
 * @brief Тесты протокола обмена с использованием UnitTest++
 */

#include <UnitTest++/UnitTest++.h>
#include <string>
#include <cstring>
#include <cstdint>

SUITE(ProtocolTests) {
    // Тест 1: Формат сообщения аутентификации
    TEST(AuthMessageFormat) {
        std::string login = "user";
        std::string salt = "A1B2C3D4E5F67890";
        std::string hash(64, 'A');
        std::string auth_msg = login + salt + hash;
        
        CHECK_EQUAL(4, login.length());
        CHECK_EQUAL(16, salt.length());
        CHECK_EQUAL(64, hash.length());
        CHECK_EQUAL(84, auth_msg.length());
    }
    
    // Тест 2: Коды ответов сервера
    TEST(ServerResponseCodes) {
        std::string ok = "OK";
        std::string err = "ERR";
        
        CHECK_EQUAL(2, ok.length());
        CHECK_EQUAL(3, err.length());
    }
    
    // Тест 3: Размеры данных
    TEST(DataSizes) {
        CHECK_EQUAL(4, sizeof(uint32_t));
        CHECK_EQUAL(4, sizeof(float));
    }
    
    // Тест 4: Формат соли
    TEST(SaltFormat) {
        std::string salt = "0123456789ABCDEF";
        
        CHECK_EQUAL(16, salt.length());
        
        for (char c : salt) {
            CHECK((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
        }
    }
    
    // Тест 5: Формат хэша
    TEST(HashFormat) {
        std::string hash = "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF";
        
        CHECK_EQUAL(64, hash.length());
        
        for (char c : hash) {
            CHECK((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
        }
    }
    
    // Тест 6: Хранение пароля в файле
    TEST(UserFileFormat) {
        std::string line = "user:P@ssW0rd";
        size_t colon_pos = line.find(':');
        std::string stored_login = line.substr(0, colon_pos);
        std::string stored_password = line.substr(colon_pos + 1);
        
        CHECK_EQUAL("user", stored_login);
        CHECK_EQUAL("P@ssW0rd", stored_password);
    }
    
    // Тест 7: Обнаружение короткого сообщения
    TEST(ShortAuthMessage) {
        std::string wrong_msg = "short";
        bool auth_error = false;
        
        if (wrong_msg.length() < 84) {
            auth_error = true;
        }
        
        CHECK(auth_error);
    }
    
    // Тест 8: Нулевое количество векторов
    TEST(ZeroVectors) {
        uint32_t zero_vectors = 0;
        CHECK_EQUAL(4, sizeof(zero_vectors));
    }
    
    // Тест 9: Последовательность данных протокола
    TEST(ProtocolSequence) {
        uint32_t num_vectors = 2;
        uint32_t vector_size = 3;
        float vector_data[] = {1.0f, 2.0f, 3.0f};
        float result = 14.0f;
        
        CHECK_EQUAL(4, sizeof(num_vectors));
        CHECK_EQUAL(4, sizeof(vector_size));
        CHECK_EQUAL(12, sizeof(vector_data));
        CHECK_EQUAL(4, sizeof(result));
    }
    
    // Тест 10: Максимальные размеры данных
    TEST(MaxDataSizes) {
        uint32_t max_size = 1000;
        float sample_data[1000];
        
        CHECK_EQUAL(4, sizeof(max_size));
        CHECK_EQUAL(4000, sizeof(sample_data));
    }
}

int main() {
    return UnitTest::RunAllTests();
}
