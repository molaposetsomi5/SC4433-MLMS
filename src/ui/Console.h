#pragma once
// Input/output helpers for the menu-driven console. Reading from a stream
// (instead of std::cin directly) lets the system tests script a whole session.
#include <iostream>
#include <optional>
#include <string>

namespace mlms {

// Thrown when the input stream ends (e.g. Ctrl+D / end of a script).
struct InputClosed {};

class Console {
public:
    Console(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

    std::string readLine(const std::string& prompt);
    int readInt(const std::string& prompt);
    std::optional<int> readOptionalInt(const std::string& prompt);  // blank = none
    double readDouble(const std::string& prompt);
    bool confirm(const std::string& prompt);

    std::ostream& out() { return out_; }
    void heading(const std::string& title);
    void error(const std::string& message);
    void success(const std::string& message);

private:
    std::istream& in_;
    std::ostream& out_;
};

// Left-aligns text in a fixed-width column (cuts it if too long).
std::string pad(const std::string& text, std::size_t width);

}  // namespace mlms
