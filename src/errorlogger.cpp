#include <sstream>
#include "gbuilder.h"

void ErrorLogger::add(Type type, const Origin &origin, const std::string &message) {
    switch(type) {
        case Error: ++theErrorCount; break;
        case Warning: ++theWarningCount; break;
        case Notice: break;
    }
    Message msg(type, origin, message);
    errors.push_back(std::move(msg));
}

std::string ErrorLogger::Message::format() const {
    std::stringstream msg;
    switch(type) {
        case Error: msg << "ERROR "; break;
        case Warning: msg << "WARNING "; break;
        case Notice: msg << "NOTICE "; break;
    }
    msg << origin.file << ':' << origin.line << ':' << origin.column << ": " << message;
    return msg.str();
}
