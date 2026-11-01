#include "domain/DateTime.h"

#include <cctype>
#include <cstdio>
#include <ctime>

#include "domain/Errors.h"

namespace mlms {

namespace {

// Days since 1970-01-01 for a civil date (H. Hinnant's algorithm).
std::int64_t daysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2 ? 1 : 0;
    const std::int64_t era = (y >= 0 ? y : y - 399) / 400;
    const auto yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<std::int64_t>(doe) - 719468;
}

void civilFromDays(std::int64_t z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const auto doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const std::int64_t yy = static_cast<std::int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y = static_cast<int>(yy + (m <= 2 ? 1 : 0));
}

bool isLeapYear(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int daysInMonth(int y, int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeapYear(y)) return 29;
    return days[m - 1];
}

int number(const std::string& s, std::size_t pos, std::size_t len) {
    int value = 0;
    for (std::size_t i = pos; i < pos + len; ++i) value = value * 10 + (s[i] - '0');
    return value;
}

// '9' = digit, anything else must match exactly
bool matchesPattern(const std::string& text) {
    static const std::string pattern = "9999-99-99 99:99:99";
    if (text.size() != 16 && text.size() != 19) return false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        const bool wantDigit = pattern[i] == '9';
        const bool isDigit = std::isdigit(static_cast<unsigned char>(text[i])) != 0;
        if (wantDigit != isDigit || (!wantDigit && text[i] != pattern[i])) return false;
    }
    return true;
}

bool fieldsInRange(int year, int month, int day, int hour, int minute, int second) {
    if (year < 1970 || month < 1 || month > 12) return false;
    if (day < 1 || day > daysInMonth(year, month)) return false;
    return hour <= 23 && minute <= 59 && second <= 59;
}

}  // namespace

bool parseDateTime(const std::string& text, Seconds& out) {
    if (!matchesPattern(text)) return false;
    const int year = number(text, 0, 4);
    const int month = number(text, 5, 2);
    const int day = number(text, 8, 2);
    const int hour = number(text, 11, 2);
    const int minute = number(text, 14, 2);
    const int second = text.size() == 19 ? number(text, 17, 2) : 0;
    if (!fieldsInRange(year, month, day, hour, minute, second)) return false;

    out = daysFromCivil(year, static_cast<unsigned>(month), static_cast<unsigned>(day)) * kSecondsPerDay +
          Seconds{hour} * 3600 + Seconds{minute} * 60 + second;
    return true;
}

Seconds toSeconds(const std::string& text) {
    Seconds value = 0;
    if (!parseDateTime(text, value)) {
        throw ValidationError("Invalid date/time '" + text + "'. Use YYYY-MM-DD HH:MM");
    }
    return value;
}

std::string formatDateTime(Seconds value) {
    std::int64_t days = value / kSecondsPerDay;
    std::int64_t rest = value % kSecondsPerDay;
    if (rest < 0) {
        rest += kSecondsPerDay;
        --days;
    }
    int y = 0;
    unsigned m = 0;
    unsigned d = 0;
    civilFromDays(days, y, m, d);
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02u-%02u %02d:%02d:%02d", y, m, d,
                  static_cast<int>(rest / 3600), static_cast<int>((rest % 3600) / 60),
                  static_cast<int>(rest % 60));
    return buffer;
}

std::string normaliseDateTime(const std::string& text) { return formatDateTime(toSeconds(text)); }

Seconds SystemClock::now() const {
    const std::time_t t = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    return daysFromCivil(local.tm_year + 1900, static_cast<unsigned>(local.tm_mon + 1),
                         static_cast<unsigned>(local.tm_mday)) *
               kSecondsPerDay +
           Seconds{local.tm_hour} * 3600 + Seconds{local.tm_min} * 60 + local.tm_sec;
}

}  // namespace mlms
