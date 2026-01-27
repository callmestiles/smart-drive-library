//
// Created by dunamis on 26/01/2026.
//

#ifndef SMARTDRIVE_VALUESOURCE_H
#define SMARTDRIVE_VALUESOURCE_H

#include <cstdint>
#include <cstring>
#include <type_traits>
#include "Logger.h"

using storageAlignment = std::max_align_t;

enum class ValueType : uint16_t {
    EMPTY = 0,
    INT32 = 1,
    UINT16 = 2,
    FLOAT = 3,
    STRING = 4
};

constexpr const char *typeToString(const ValueType t) {
    switch (t) {
        case ValueType::INT32: return "int32";
        case ValueType::UINT16: return "uint16";
        case ValueType::FLOAT: return "float";
        case ValueType::STRING: return "string";
        default: return "empty";
    }
}

class ValueSource {
private:
    ValueType type = ValueType::EMPTY;
    alignas(storageAlignment) uint8_t data[16]{};

public:
    ValueSource() = default;

    //Numerical pack
    template<typename T>
    void pack(T value) {
        static_assert(std::is_arithmetic_v<T>, "Value must be arithmetic. Use string overload for non-numeric types");
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        static_assert(std::is_same_v<T, int32_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, float>,
                      "Invalid type");
        static_assert(sizeof(T) <= 16, "Value too big for 16-byte buffer");

        if constexpr (std::is_same_v<T, int32_t>) {
            type = ValueType::INT32;
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            type = ValueType::UINT16;
        } else if constexpr (std::is_same_v<T, float>) {
            type = ValueType::FLOAT;
        }
        std::memcpy(data, &value, sizeof(T));
    }

    //String pack
    void pack(const char *value) {
        type = ValueType::STRING;
        if (std::strlen(value) >= 16) {
            Logger::log(LogLevel::WARNING, "String too long for 16-byte buffer. Truncating...");
        }
        std::strncpy(reinterpret_cast<char *>(data), value, 15);
        data[15] = '\0';
    }

    template<typename T>
    T unpack() const {
        static_assert(std::is_arithmetic_v<T>, "Use unpackString() for strings");

        if constexpr (std::is_same_v<T, float>) {
            if (type != ValueType::FLOAT) return 0.0f;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            if (type != ValueType::INT32) return 0;
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            if (type != ValueType::UINT16) return 0;
        }

        T result;
        std::memcpy(&result, data, sizeof(T));
        return result;
    }

    const char *unpackString() const { return reinterpret_cast<const char *>(data); }

    ValueType getType() const { return type; }
};
#endif //SMARTDRIVE_VALUESOURCE_H
