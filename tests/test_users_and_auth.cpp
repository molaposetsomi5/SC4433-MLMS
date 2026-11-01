// User hierarchy (polymorphism, factory) and AuthService (FR-01, FR-19, NFR-03).
#include "TestApp.h"
#include "domain/Errors.h"

using namespace mlms;
using namespace mlms::test;

TEST(UserFactory, CreatesTheRightSubclassForEachRole) {
    UserRecord r;
    r.role = Role::HoD;
    auto user = UserFactory::create(r);
    EXPECT_NE(dynamic_cast<HeadOfDepartment*>(user.get()), nullptr);
    EXPECT_EQ(user->role(), Role::HoD);
    r.role = Role::Student;
    EXPECT_NE(dynamic_cast<Student*>(UserFactory::create(r).get()), nullptr);
}

TEST(UserPermissions, EachRoleHasOnlyItsOwnPermissions) {
    UserRecord r;
    r.role = Role::Administrator;
    auto adminUser = UserFactory::create(r);
    r.role = Role::Lecturer;
    auto lecturerUser = UserFactory::create(r);
    r.role = Role::HoD;
    auto hodUser = UserFactory::create(r);
    r.role = Role::Student;
    auto studentUser = UserFactory::create(r);

    EXPECT_TRUE(adminUser->hasPermission(Permission::ManageUsers));
    EXPECT_FALSE(adminUser->hasPermission(Permission::MarkSubmission));  // least privilege (C13)
    EXPECT_TRUE(lecturerUser->hasPermission(Permission::MarkSubmission));
    EXPECT_FALSE(lecturerUser->hasPermission(Permission::ReleaseMarks));
    EXPECT_TRUE(hodUser->hasPermission(Permission::ReleaseMarks));
    EXPECT_FALSE(hodUser->hasPermission(Permission::SubmitWork));
    EXPECT_TRUE(studentUser->hasPermission(Permission::SubmitWork));
    EXPECT_FALSE(studentUser->hasPermission(Permission::CreateAssignment));
}

class AuthTest : public LmsTest {};

TEST_F(AuthTest, LoginWithValidCredentialsReturnsRoleUser) {
    auto user = app->auth.login("lecturer", "Password123");
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->role(), Role::Lecturer);
}

TEST_F(AuthTest, WrongPasswordAndUnknownUserGiveSameMessage) {
    std::string wrongPassword;
    std::string unknownUser;
    try {
        app->auth.login("lecturer", "nope");
    } catch (const AuthError& e) {
        wrongPassword = e.what();
    }
    try {
        app->auth.login("ghost", "nope");
    } catch (const AuthError& e) {
        unknownUser = e.what();
    }
    EXPECT_EQ(wrongPassword, unknownUser);
    EXPECT_FALSE(wrongPassword.empty());
}

TEST_F(AuthTest, PasswordsAreStoredAsSaltedArgon2idHashes) {
    auto a = reload(student);
    auto b = reload(student2);
    EXPECT_NE(a->record().passwordHash, "Password123");
    EXPECT_EQ(a->record().passwordHash.rfind("$argon2id$", 0), 0u);
    // same password, different salt -> different hash
    EXPECT_NE(a->record().passwordHash, b->record().passwordHash);
}

TEST_F(AuthTest, AccountLocksAfterFiveFailuresAndUnlocksLater) {
    for (int i = 0; i < 4; ++i) EXPECT_THROW(app->auth.login("student", "bad"), AuthError);
    EXPECT_THROW(app->auth.login("student", "bad"), AuthError);  // 5th -> locked
    EXPECT_THROW(app->auth.login("student", "Password123"), AuthError);  // correct but locked
    clock->advance(16 * 60);
    EXPECT_NO_THROW(app->auth.login("student", "Password123"));
    EXPECT_EQ(reload(student)->record().failedLoginCount, 0);
}

TEST_F(AuthTest, SuccessfulLoginResetsFailedCount) {
    EXPECT_THROW(app->auth.login("student", "bad"), AuthError);
    app->auth.login("student", "Password123");
    EXPECT_EQ(reload(student)->record().failedLoginCount, 0);
}

TEST_F(AuthTest, DeactivatedAccountCannotLogIn) {
    app->users.setActive(*admin, student->id(), false);
    EXPECT_THROW(app->auth.login("student", "Password123"), AuthError);
    app->users.setActive(*admin, student->id(), true);
    EXPECT_NO_THROW(app->auth.login("student", "Password123"));
}

TEST_F(AuthTest, ChangePasswordNeedsCurrentPassword) {
    auto user = reload(student);
    EXPECT_THROW(app->auth.changePassword(*user, "wrong", "NewPassword1"), AuthError);
    EXPECT_THROW(app->auth.changePassword(*user, "Password123", "short"), ValidationError);
    app->auth.changePassword(*user, "Password123", "NewPassword1");
    EXPECT_THROW(app->auth.login("student", "Password123"), AuthError);
    EXPECT_NO_THROW(app->auth.login("student", "NewPassword1"));
}

TEST_F(AuthTest, AdminResetUnlocksAccount) {
    for (int i = 0; i < 5; ++i) EXPECT_THROW(app->auth.login("student", "bad"), AuthError);
    app->users.resetPassword(*admin, student->id(), "Reset12345");
    EXPECT_NO_THROW(app->auth.login("student", "Reset12345"));
}
