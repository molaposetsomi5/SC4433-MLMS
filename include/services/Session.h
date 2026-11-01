
#ifndef SESSION_H
#define SESSION_H

#include <memory>

#include "domain/User.h"

using namespace std;

class Session
{
private:
    unique_ptr<User> currentUser;

public:
    // Constructor
    Session()
    {
        currentUser = nullptr;
    }

    // Start a session for an authenticated user
    void startSession(unique_ptr<User> user);

    // End the current session
    void endSession();

    // Check whether a user is logged in
    bool isAuthenticated() const;

    // Get the currently logged-in user
    User* getCurrentUser() const;
};

#endif
