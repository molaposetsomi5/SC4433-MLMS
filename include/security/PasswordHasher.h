
#ifndef PASSWORD_HASHER_H
#define PASSWORD_HASHER_H

#include <string>

using namespace std;

class PasswordHasher
{
public:
    // Generate an Argon2id password hash
    static string hashPassword(const string& password);

    // Check whether a password matches a stored hash
    static bool verifyPassword(const string& password,
        const string& storedHash);
};

#endif
