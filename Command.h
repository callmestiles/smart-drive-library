//
// Created by dunamis on 27/01/2026.
//

#ifndef SMARTDRIVE_COMMAND_H
#define SMARTDRIVE_COMMAND_H
#include <cstdint>

#pragma pack(push, 1)
struct Command {
    uint16_t command_type;
    float w,x,y,z;
    int16_t s,t,u,v;
};
#pragma pack(pop)

static_assert(sizeof(Command) == 26, "Command must be exactly 26 bytes");
#endif //SMARTDRIVE_COMMAND_H