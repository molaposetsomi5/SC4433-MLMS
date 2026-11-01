#pragma once
// Works out the effective deadline for a student and how late a submission is.
#include <string>

#include "data/Repositories.h"
#include "domain/LatePenaltyPolicy.h"

namespace mlms {

struct DeadlineInfo {
    std::string original;
    std::string effective;
    bool individualExtension = false;
    std::string extensionReason;
};

class DeadlineResolver {
public:
    DeadlineResolver(IExtensionRepository& extensions, ISubmissionRepository& submissions,
                     const LatePenaltyPolicy& policy)
        : extensions_(extensions), submissions_(submissions), policy_(policy) {}

    // Effective deadline = the original deadline, or the student's individual
    // extension if one has been granted.
    DeadlineInfo resolve(const Assignment& assignment, int studentId) const;

    // Late days of the latest valid version of a submission (0 = on time).
    int lateDays(const Assignment& assignment, const Submission& submission) const;

private:
    IExtensionRepository& extensions_;
    ISubmissionRepository& submissions_;
    const LatePenaltyPolicy& policy_;
};

}  // namespace mlms
