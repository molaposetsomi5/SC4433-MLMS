#include "security/PasswordHasher.h"

#include <sodium.h>

#include "domain/Errors.h"

namespace mlms {

namespace {

void initialiseSodium() {
    static const bool ready = sodium_init() >= 0;
    if (!ready) throw MlmsError("libsodium could not be initialised");
}

}  // namespace

SodiumPasswordHasher::SodiumPasswordHasher()
    : SodiumPasswordHasher(crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) {}

SodiumPasswordHasher::SodiumPasswordHasher(unsigned long long opsLimit, std::size_t memLimit)
    : opsLimit_(opsLimit), memLimit_(memLimit) {
    initialiseSodium();
}

SodiumPasswordHasher SodiumPasswordHasher::fastForTesting() {
    return SodiumPasswordHasher(crypto_pwhash_OPSLIMIT_MIN, crypto_pwhash_MEMLIMIT_MIN);
}

std::string SodiumPasswordHasher::hash(const std::string& password) const {
    char out[crypto_pwhash_STRBYTES];
    // crypto_pwhash_str uses Argon2id with a fresh random salt
    if (crypto_pwhash_str(out, password.c_str(), password.size(), opsLimit_, memLimit_) != 0) {
        throw MlmsError("Password hashing failed (out of memory)");
    }
    return out;
}

bool SodiumPasswordHasher::verify(const std::string& password, const std::string& storedHash) const {
    return crypto_pwhash_str_verify(storedHash.c_str(), password.c_str(), password.size()) == 0;
}

}  // namespace mlms
