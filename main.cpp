#include <iostream>
#include "ValueSource.h"

void pcLogHandler(LogLevel level, const char *message) {
    const char* label = (level == LogLevel::WARNING) ? "[WARN]: ": "[LOG] ";
    std::cout << label << message << std::endl;
}

int main() {
    Logger::setCallback(pcLogHandler);
    ValueSource vs;

    vs.pack<int32_t>(10);
    std::cout << "Type: " << typeToString(vs.getType()) << " Value: " << vs.unpack<float>() << std::endl;

    vs.packString("1234567890123456789");
    if (vs.getType() == ValueType::STRING) {
        std::cout << "Type: string Value: " << vs.unpackString() << std::endl;
    }
    return 0;
}