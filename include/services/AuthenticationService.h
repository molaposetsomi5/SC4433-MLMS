
#ifndef AUTHENTICATION_SERVICE_H
#define AUTHENTICATION_SERVICE_H

#include <string>
#include <memory>

#include "repositories/UserRepository.h"

using namespace std;

class AuthenticationService
{
private:
    UserRepository& userRepository;

    // Account lockout settings
    static const int MAX_FAILED_ATTEMPTS = 5;
    static const int LOCKOUT_MINUTES = 15;

public:
    // Constructor
    AuthenticationService(UserRepository& userRepository)
        : userRepository(userRepository)
    {
    }

    // Authenticate a user using their username and password
    unique_ptr<User> login(
        const string& username,
        const string& password
    );
};

#endif
