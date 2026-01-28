cmake_minimum_required(VERSION 4.1)
project(SmartDrive)

set(CMAKE_CXX_STANDARD 17)

add_executable(SmartDrive main.cpp
        ValueSource.h
        Logger.h
        Command.h
        Config.h
        RobotData.h
)
