#pragma once
// AuthenticationService (FR-01, FR-19, NFR-03).
#include <cstddef>
#include <memory>
#include <string>

#include "data/Repositories.h"
#include "domain/DateTime.h"
#include "security/PasswordHasher.h"

namespace mlms {

// Lockout values agreed for v1.0 (design review item in section 9).
struct AuthSettings {
    int maxFailedAttempts = 5;
    int lockMinutes = 15;
    std::size_t minPasswordLength = 8;
};

class AuthService {
public:
    AuthService(IUserRepository& users, const IPasswordHasher& hasher, const IClock& clock,
                AuthSettings settings = AuthSettings());

    std::shared_ptr<User> login(const std::string& username, const std::string& password);
    void changePassword(User& user, const std::string& currentPassword, const std::string& newPassword);
    void validateNewPassword(const std::string& password) const;
    const AuthSettings& settings() const { return settings_; }

private:
    IUserRepository& users_;
    const IPasswordHasher& hasher_;
    const IClock& clock_;
    AuthSettings settings_;
};

}  // namespace mlms
