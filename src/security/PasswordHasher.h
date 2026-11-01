#pragma once
// Password hashing abstraction. The real implementation uses libsodium's
// Argon2id (crypto_pwhash_str), which generates a random salt per password
// and stores the salt and parameters inside the hash string.
#include <cstddef>
#include <string>

namespace mlms {

class IPasswordHasher {
public:
    virtual ~IPasswordHasher() = default;
    virtual std::string hash(const std::string& password) const = 0;
    virtual bool verify(const std::string& password, const std::string& storedHash) const = 0;
};

class SodiumPasswordHasher final : public IPasswordHasher {
public:
    // Default = libsodium "interactive" limits. Tests pass the minimum limits to run fast.
    SodiumPasswordHasher();
    SodiumPasswordHasher(unsigned long long opsLimit, std::size_t memLimit);

    static SodiumPasswordHasher fastForTesting();

    std::string hash(const std::string& password) const override;
    bool verify(const std::string& password, const std::string& storedHash) const override;

private:
    unsigned long long opsLimit_;
    std::size_t memLimit_;
};

}  // namespace mlms
