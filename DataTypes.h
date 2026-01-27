//
// Created by dunamis on 27/01/2026.
//

#ifndef SMARTDRIVE_DATATYPES_H
#define SMARTDRIVE_DATATYPES_H

#include <cstdint>
#include "ValueSource.h"

#pragma pack(push, 1)
struct SettingsData : public ValueSource {
    uint16_t settingsID;
};

struct TelemetryData : public ValueSource {
    uint16_t sourceID;
    uint32_t timestamp;
};
#pragma pack(pop)

static_assert(sizeof(TelemetryData) == 24, "TelemetryData must be exactly 24 bytes");
#endif //SMARTDRIVE_DATATYPES_H