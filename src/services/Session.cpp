
#include "services/Session.h"

// Start a session for an authenticated user
void Session::startSession(unique_ptr<User> user)
{
    currentUser = move(user);
}


// End the current session
void Session::endSession()
{
    currentUser.reset();
}


// Check whether a user is logged in
bool Session::isAuthenticated() const
{
    return currentUser != nullptr;
}


// Get the currently logged-in user
User* Session::getCurrentUser() const
{
    return currentUser.get();
}
