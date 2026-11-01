#include "services/AuthService.h"

#include "domain/Errors.h"
#include "domain/Validation.h"

namespace mlms {

namespace {
const char* const kInvalidLogin = "Invalid username or password";
}

AuthService::AuthService(IUserRepository& users, const IPasswordHasher& hasher, const IClock& clock,
                         AuthSettings settings)
    : users_(users), hasher_(hasher), clock_(clock), settings_(settings) {}

std::shared_ptr<User> AuthService::login(const std::string& username, const std::string& password) {
    auto user = users_.findByUsername(trim(username));
    if (!user) throw AuthError(kInvalidLogin);

    UserRecord& rec = user->record();
    if (!rec.active) throw AuthError("This account has been deactivated. Contact the administrator.");

    const Seconds now = clock_.now();
    if (rec.lockedUntil && now < toSeconds(*rec.lockedUntil)) {
        throw AuthError("Account locked after too many failed attempts. Try again after " + *rec.lockedUntil);
    }

    if (!hasher_.verify(password, rec.passwordHash)) {
        rec.failedLoginCount += 1;
        if (rec.failedLoginCount >= settings_.maxFailedAttempts) {
            rec.lockedUntil = formatDateTime(now + static_cast<Seconds>(settings_.lockMinutes) * 60);
            rec.failedLoginCount = 0;
            users_.update(rec);
            throw AuthError("Too many failed attempts. Account locked until " + *rec.lockedUntil);
        }
        users_.update(rec);
        throw AuthError(kInvalidLogin);
    }

    if (rec.failedLoginCount != 0 || rec.lockedUntil) {
        rec.failedLoginCount = 0;
        rec.lockedUntil.reset();
        users_.update(rec);
    }
    return user;
}

void AuthService::validateNewPassword(const std::string& password) const {
    if (password.size() < settings_.minPasswordLength) {
        throw ValidationError("Password must be at least " + std::to_string(settings_.minPasswordLength) +
                              " characters long");
    }
}

void AuthService::changePassword(User& user, const std::string& currentPassword, const std::string& newPassword) {
    if (!hasher_.verify(currentPassword, user.record().passwordHash)) {
        throw AuthError("Current password is incorrect");
    }
    validateNewPassword(newPassword);
    user.record().passwordHash = hasher_.hash(newPassword);
    users_.update(user.record());
}

}  // namespace mlms
