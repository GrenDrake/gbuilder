#include <sstream>
#include "gbuilder.h"

void ErrorLogger::add(const std::string &file, int line, int column, const std::string &message) {
    Message msg(file, line, column, message);
    errors.push_back(std::move(msg));
}

std::string ErrorLogger::Message::format() const {
    std::stringstream msg;
    msg << file << ':' << line << ':' << column << ": " << message;
    return msg.str();
}
