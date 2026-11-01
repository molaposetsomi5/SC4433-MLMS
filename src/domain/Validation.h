#pragma once
// Small input-validation helpers shared by the services.
#include <cstdio>
#include <string>

#include "domain/Errors.h"

namespace mlms {

inline std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

inline std::string requireText(const std::string& value, const std::string& fieldName) {
    std::string cleaned = trim(value);
    if (cleaned.empty()) throw ValidationError(fieldName + " is required");
    return cleaned;
}

inline void requireMarkInRange(double mark, double maxMark) {
    if (mark < 0 || mark > maxMark) {
        char buffer[96];
        std::snprintf(buffer, sizeof(buffer), "Mark must be between 0 and %.2f", maxMark);
        throw ValidationError(buffer);
    }
}

inline std::string formatMark(double value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.2f", value);
    return buffer;
}

}  // namespace mlms
