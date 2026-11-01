#include "ui/Console.h"

#include <cctype>
#include <cstdlib>

#include "domain/Validation.h"

namespace mlms {

std::string Console::readLine(const std::string& prompt) {
    out_ << prompt;
    out_.flush();
    std::string line;
    if (!std::getline(in_, line)) throw InputClosed{};
    if (!line.empty() && line.back() == '\r') line.pop_back();  // Windows line endings
    return line;
}

namespace {

bool parseInt(const std::string& text, int& value) {
    const std::string t = trim(text);
    if (t.empty() || t.size() > 9) return false;
    std::size_t start = t[0] == '-' ? 1 : 0;
    if (start == t.size()) return false;
    for (std::size_t i = start; i < t.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(t[i]))) return false;
    }
    value = std::atoi(t.c_str());
    return true;
}

bool parseDouble(const std::string& text, double& value) {
    const std::string t = trim(text);
    if (t.empty()) return false;
    char* end = nullptr;
    value = std::strtod(t.c_str(), &end);
    return end != nullptr && *end == '\0';
}

}  // namespace

int Console::readInt(const std::string& prompt) {
    while (true) {
        int value = 0;
        if (parseInt(readLine(prompt), value)) return value;
        error("Please enter a whole number.");
    }
}

std::optional<int> Console::readOptionalInt(const std::string& prompt) {
    while (true) {
        const std::string text = trim(readLine(prompt));
        if (text.empty()) return std::nullopt;
        int value = 0;
        if (parseInt(text, value)) return value;
        error("Please enter a whole number, or leave it blank.");
    }
}

double Console::readDouble(const std::string& prompt) {
    while (true) {
        double value = 0;
        if (parseDouble(readLine(prompt), value)) return value;
        error("Please enter a number (for example 42 or 37.5).");
    }
}

bool Console::confirm(const std::string& prompt) {
    while (true) {
        const std::string answer = trim(readLine(prompt + " (y/n): "));
        if (answer == "y" || answer == "Y") return true;
        if (answer == "n" || answer == "N") return false;
        error("Please answer y or n.");
    }
}

void Console::heading(const std::string& title) {
    out_ << "\n=== " << title << " ===\n";
}

void Console::error(const std::string& message) { out_ << "  [!] " << message << "\n"; }

void Console::success(const std::string& message) { out_ << "  [OK] " << message << "\n"; }

std::string pad(const std::string& text, std::size_t width) {
    if (text.size() >= width) return text.substr(0, width - 1) + " ";
    return text + std::string(width - text.size(), ' ');
}

}  // namespace mlms
