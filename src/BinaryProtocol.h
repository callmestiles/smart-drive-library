//
// Created by dunamis on 28/01/2026.
//

#ifndef SMARTDRIVE_BINARYPROTOCOL_H
#define SMARTDRIVE_BINARYPROTOCOL_H

#include <cstring>
#include "../interfaces/IProtocol.h"
#include "../types/ProtocolTypes.h"
#include "../types/RobotData.h"
#include "../utils/Logger.h"


namespace ProtocolConstants {
    constexpr uint8_t STX = 0x02;
    constexpr uint16_t MAX_PAYLOAD_SIZE = 256;
    constexpr uint16_t PROTOCOL_OVERHEAD = 6; //STX(1) + TYPE(1) + LENGTH(2) + CRC(2)
    constexpr uint16_t MAX_FRAME_SIZE = MAX_PAYLOAD_SIZE + PROTOCOL_OVERHEAD;

    enum class FrameType : uint8_t {
        COMMAND = 0x01,
        DISCOVERY = 0x02,
        TELEMETRY = 0x03,
        SETTINGS = 0x04,
        VALUE_SOURCE = 0x05
    };
}

class BinaryProtocol : public IProtocol {
private:
    uint8_t frameBuffer[ProtocolConstants::MAX_FRAME_SIZE];

    static uint16_t calculateCRC16(const uint8_t *data, const size_t length) {
        uint16_t crc = 0xFFFF;

        for (size_t i = 0; i < length; ++i) {
            crc ^= static_cast<uint16_t>(data[i]) << 8;

            for (uint8_t bit = 0; bit < 8; ++bit) {
                if (crc && 0x8000) {
                    crc = (crc << 1) ^ 0x1021;
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }

    static inline void writeUint16LE(uint8_t *dest, const uint16_t value) {
        dest[0] = value & 0xFF;
        dest[1] = (value >> 8) & 0xFF;
    }

    static inline uint16_t readUint16LE(const uint8_t *src) {
        return static_cast<uint16_t>(src[0]) |
               (static_cast<uint16_t>(src[1]) << 8);
    }

    static inline writeUint32LE(uint8_t *dest, const uint32_t value) {
        dest[0] = value & 0xFF;
        dest[1] = (value >> 8) & 0xFF;
        dest[2] = (value >> 16) & 0xFF;
        dest[3] = (value >> 24) & 0xFF;
    }

    static inline uint32_t readUint32LE(const uint8_t *src) {
        return static_cast<uint32_t>(src[0]) |
               (static_cast<uint32_t>(src[1]) << 8) |
               (static_cast<uint32_t>(src[2]) << 16) |
               (static_cast<uint32_t>(src[3]) << 24);
    }

    size_t buildFrame(ProtocolConstants::FrameType type,
                      const void *payload,
                      const size_t payloadSize
    ) {
        if (payloadSize > ProtocolConstants::MAX_PAYLOAD_SIZE) {
            LOG(LogLevel::ERROR, "Payload exceeds max size");
            return 0;
        }

        size_t offset = 0;

        frameBuffer[offset++] = ProtocolConstants::STX;

        frameBuffer[offset++] = static_cast<uint8_t>(type);

        writeUint16LE(&frameBuffer[offset], static_cast<uint16_t>(payloadSize));
        offset += 2;

        memcpy(&frameBuffer[offset], payload, payloadSize);
        offset += payloadSize;

        const uint16_t crc = calculateCRC16(frameBuffer, offset);
        writeUint16LE(&frameBuffer[offset], crc);
        offset += 2;

        return offset;
    }

    bool parseFrame(const uint8_t *data,
                    size_t dataSize,
                    ProtocolConstants::FrameType expectedType,
                    void *payloadOut,
                    size_t expectedPayloadSize) const {
        if (dataSize < ProtocolConstants::PROTOCOL_OVERHEAD) {
            LOG(LogLevel::ERROR, "Frame too small");
            return false;
        }

        size_t offset = 0;

        if (data[offset++] != ProtocolConstants::STX) {
            LOG(LogLevel::ERROR, "Invalid STX marker");
            return false;
        }

        auto frameType = static_cast<ProtocolConstants::FrameType>(data[offset++]);
        if (frameType != expectedType) {
            LOG(LogLevel::ERROR, "Frame type mismatch");
            return false;
        }

        uint16_t payloadLength = readUint16LE(&data[offset]);
        offset += 2;

        if (dataSize != offset + payloadLength + 2) {
            //+2 is from the crc
            LOG(LogLevel::ERROR, "Invalid frame size");
            return false;
        }

        if (payloadLength != expectedPayloadSize) {
            LOG(LogLevel::ERROR, "Payload size mismatch");
            return false;
        }

        const size_t crcOffset = offset + payloadLength;
        const uint16_t receivedCrc = readUint16LE(&data[crcOffset]);

        if (uint16_t calculatedCRC = calculateCRC16(data, crcOffset); receivedCrc != calculatedCRC) {
            LOG(LogLevel::ERROR, "CRC mismatch");
            return false;
        }

        memcpy(payloadOut, &data[offset], payloadLength);
        return true;
    }

public:
    BinaryProtocol() {
        memset(frameBuffer, 0, sizeof(frameBuffer));
    }

    SerializedData serializeCommand(const Command &cmd) override {
        SerializedData result;
        result.size = buildFrame(ProtocolConstants::FrameType::COMMAND,
                                 &cmd, sizeof(Command));
        if (result.size > 0) {
            memcpy(result.data, frameBuffer, result.size);
        }
        return result;
    }

    bool deserializeCommand(const uint8_t *data, size_t size, Command &cmdOut) override {
        return parseFrame(data, size,
                          ProtocolConstants::FrameType::COMMAND,
                          &cmdOut,
                          sizeof(Command));
    }

    SerializedData serializeDiscovery(const DiscoveryResponse &resp) override {
        SerializedData result;
        result.size = buildFrame(ProtocolConstants::FrameType::DISCOVERY,
                                 &resp, sizeof(DiscoveryResponse));
        if (result.size > 0) {
            memcpy(result.data, frameBuffer, result.size);
        }
        return result;
    }

    bool deserializeDiscovery(const uint8_t *data, size_t size, DiscoveryResponse &respOut) override {
        return parseFrame(data, size,
                          ProtocolConstants::FrameType::DISCOVERY,
                          &respOut,
                          sizeof(DiscoveryResponse));
    }

    SerializedData serializeValue(const ValueSource &value) override {
        SerializedData result;
        result.size = buildFrame(ProtocolConstants::FrameType::VALUE_SOURCE,
                                 &value, sizeof(ValueSource));
        if (result.size > 0) {
            memcpy(result.data, frameBuffer, result.size);
        }
        return result;
    }

    bool deserializeValue(const uint8_t *data, size_t size, ValueSource &valueOut) override {
        return parseFrame(data, size,
                          ProtocolConstants::FrameType::VALUE_SOURCE,
                          &valueOut,
                          sizeof(ValueSource));
    }

    SerializedData serializeTelemetry(const TelemetryData &telemetry) override {
        SerializedData result;
        result.size = buildFrame(ProtocolConstants::FrameType::TELEMETRY,
                                 &telemetry, sizeof(TelemetryData));
        if (result.size > 0) {
            memcpy(result.data, frameBuffer, result.size);
        }
        return result;
    }

    bool deserializeTelemetry(const uint8_t *data, size_t size, TelemetryData &telemetryOut) override {
        return parseFrame(data, size,
                          ProtocolConstants::FrameType::TELEMETRY,
                          &telemetryOut,
                          sizeof(TelemetryData));
    }

    SerializedData serializeSettings(const SettingsData &settings) override {
        SerializedData result;
        result.size = buildFrame(ProtocolConstants::FrameType::SETTINGS, &settings, sizeof(SettingsData));
        if (result.size > 0) {
            memcpy(result.data, frameBuffer, result.size);
        }
        return result;
    }

    bool deserializeSettings(const uint8_t *data, size_t size, SettingsData &settingsOut) override {
        return parseFrame(data, size,
                          ProtocolConstants::FrameType::SETTINGS,
                          &settingsOut,
                          sizeof(SettingsData));
    }

    uint16_t computeIntegrityCode(const uint8_t *data, size_t length) override {
        return calculateCRC16(data, length);
    }
};

#endif //SMARTDRIVE_BINARYPROTOCOL_H
