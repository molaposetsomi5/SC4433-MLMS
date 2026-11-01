#pragma once
// Timestamps are stored as text in the form "YYYY-MM-DD HH:MM:SS" using the
// school computer's local clock (see SRS assumption on the host system clock).
// For calculations they are converted to a count of seconds.
#include <cstdint>
#include <string>

namespace mlms {

using Seconds = std::int64_t;

constexpr Seconds kSecondsPerDay = 86400;

// Accepts "YYYY-MM-DD HH:MM" or "YYYY-MM-DD HH:MM:SS". Returns false if malformed.
bool parseDateTime(const std::string& text, Seconds& out);

// Same as parseDateTime but throws ValidationError on bad input.
Seconds toSeconds(const std::string& text);

// Always produces "YYYY-MM-DD HH:MM:SS".
std::string formatDateTime(Seconds value);

// Normalises user input such as "2026-11-02 10:00" to the stored format.
std::string normaliseDateTime(const std::string& text);

// Clock abstraction so that tests can control "now" (Dependency Injection).
class IClock {
public:
    virtual ~IClock() = default;
    virtual Seconds now() const = 0;
    std::string nowText() const { return formatDateTime(now()); }
};

class SystemClock : public IClock {
public:
    Seconds now() const override;
};

class FixedClock : public IClock {
public:
    explicit FixedClock(Seconds start) : now_(start) {}
    Seconds now() const override { return now_; }
    void set(Seconds value) { now_ = value; }
    void advance(Seconds delta) { now_ += delta; }

private:
    Seconds now_;
};

}  // namespace mlms
