cmake_minimum_required(VERSION 4.1)
project(SmartDrive)

set(CMAKE_CXX_STANDARD 17)

add_executable(SmartDrive src/main.cpp
        types/ValueSource.h
        utils/Logger.h
        Config.h
        types/RobotData.h
        src/BinaryProtocol.h
        interfaces/IProtocol.h
        types/ProtocolTypes.h
        constants/ProtocolConstants.h
)
