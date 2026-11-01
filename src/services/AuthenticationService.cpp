
#include "services/AuthenticationService.h"

#include "security/PasswordHasher.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// Convert a time point into a UTC timestamp
static string convertToUtcTimestamp(
    chrono::system_clock::time_point timePoint)
{
    time_t currentTime =
        chrono::system_clock::to_time_t(timePoint);

    tm utcTime{};

    gmtime_s(&utcTime, &currentTime);

    stringstream timestamp;

    timestamp << put_time(&utcTime, "%Y-%m-%d %H:%M:%S");

    return timestamp.str();
}


// Get the current UTC timestamp
static string getCurrentUtcTimestamp()
{
    return convertToUtcTimestamp(
        chrono::system_clock::now()
    );
}


// Calculate the lockout expiry time
static string getLockoutExpiryTime(int minutes)
{
    chrono::system_clock::time_point expiryTime =
        chrono::system_clock::now() + chrono::minutes(minutes);

    return convertToUtcTimestamp(expiryTime);
}


// Authenticate a user
unique_ptr<User> AuthenticationService::login(
    const string& username,
    const string& password)
{
    // Find the user in the database
    unique_ptr<User> user =
        userRepository.findByUsername(username);

    // Reject unknown usernames
    if (user == nullptr)
    {
        return nullptr;
    }

    // Reject inactive accounts
    if (!user->isActive())
    {
        return nullptr;
    }

    // Check whether the account is currently locked
    string currentTime = getCurrentUtcTimestamp();

    string lockedUntil = user->getLockedUntil();

    if (!lockedUntil.empty())
    {
        if (currentTime < lockedUntil)
        {
            return nullptr;
        }

        // The previous lockout has expired
        userRepository.updateLoginState(
            user->getUserId(),
            0,
            ""
        );

        user->setFailedLoginCount(0);
        user->setLockedUntil("");
    }

    // Verify the supplied password
    bool passwordCorrect = PasswordHasher::verifyPassword(
        password,
        user->getPasswordHash()
    );

    if (!passwordCorrect)
    {
        // Increase the failed login count
        int failedAttempts = user->getFailedLoginCount() + 1;

        string newLockedUntil = "";

        // Lock the account after five consecutive failures
        if (failedAttempts >= MAX_FAILED_ATTEMPTS)
        {
            newLockedUntil =
                getLockoutExpiryTime(LOCKOUT_MINUTES);
        }

        // Save the failed attempt in the database
        bool updated = userRepository.updateLoginState(
            user->getUserId(),
            failedAttempts,
            newLockedUntil
        );

        if (!updated)
        {
            return nullptr;
        }

        return nullptr;
    }

    // Reset failed attempts after a successful password check
    bool updated = userRepository.updateLoginState(
        user->getUserId(),
        0,
        ""
    );

    if (!updated)
    {
        return nullptr;
    }

    user->setFailedLoginCount(0);
    user->setLockedUntil("");

    return user;
}
