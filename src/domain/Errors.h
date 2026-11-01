#pragma once
// Exception hierarchy used across the layers. The console catches MlmsError
// so that bad input or a database problem never crashes the program.
#include <stdexcept>
#include <string>

namespace mlms {

class MlmsError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Bad or missing input (blank field, malformed date, mark out of range ...)
class ValidationError : public MlmsError {
public:
    using MlmsError::MlmsError;
};

// A business rule was broken (resubmitting after the deadline, releasing an unapproved mark ...)
class RuleViolation : public MlmsError {
public:
    using MlmsError::MlmsError;
};

// The user's role or resource scope does not allow the operation.
class PermissionError : public MlmsError {
public:
    using MlmsError::MlmsError;
};

// A referenced record does not exist.
class NotFoundError : public MlmsError {
public:
    using MlmsError::MlmsError;
};

// Login failed, account inactive or locked.
class AuthError : public MlmsError {
public:
    using MlmsError::MlmsError;
};

// SQLite reported an error.
class DatabaseError : public MlmsError {
public:
    using MlmsError::MlmsError;
};

}  // namespace mlms
