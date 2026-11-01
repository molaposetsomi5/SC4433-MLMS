
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <bcrypt.h>

#include "security/PasswordHasher.h"
#include "argon2.h"

#include <stdexcept>

// Generate an Argon2id password hash
string PasswordHasher::hashPassword(const string& password)
{
    // Generate a unique random salt
    unsigned char salt[16];

    NTSTATUS status = BCryptGenRandom(
        nullptr,
        salt,
        sizeof(salt),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );

    if (!BCRYPT_SUCCESS(status))
    {
        throw runtime_error("Could not generate password salt.");
    }

    // Store the encoded password hash
    char encodedHash[256];

    int result = argon2id_hash_encoded(
        2,                      // Number of iterations
        19456,                  // Memory in KiB
        1,                      // Parallelism
        password.c_str(),
        password.length(),
        salt,
        sizeof(salt),
        32,                     // Hash length
        encodedHash,
        sizeof(encodedHash)
    );

    if (result != ARGON2_OK)
    {
        throw runtime_error("Password hashing failed.");
    }

    return string(encodedHash);
}


// Verify a password against its stored hash
bool PasswordHasher::verifyPassword(
    const string& password,
    const string& storedHash)
{
    int result = argon2id_verify(
        storedHash.c_str(),
        password.c_str(),
        password.length()
    );

    return result == ARGON2_OK;
}
