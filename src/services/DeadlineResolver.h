#pragma once
// Works out the effective deadline for a student and how late a submission is.
#include <string>

#include "data/Repositories.h"
#include "domain/LatePenaltyPolicy.h"

namespace mlms {

struct DeadlineInfo {
    std::string original;
    std::string effective;
    bool individualExtension = false;  // true when the student's own extension is the one that applies
    bool classExtension = false;       // v1.1 (CR1): true when the whole-class extension applies
    std::string extensionReason;
    bool isExtended() const { return individualExtension || classExtension; }
};

class DeadlineResolver {
public:
    DeadlineResolver(IExtensionRepository& extensions, ISubmissionRepository& submissions,
                     const LatePenaltyPolicy& policy)
        : extensions_(extensions), submissions_(submissions), policy_(policy) {}

    // Effective deadline = the LATEST of: the original deadline, the class
    // extension (v1.1) and the student's individual extension. Taking the latest
    // means a class extension can never shorten a student's individual one.
    DeadlineInfo resolve(const Assignment& assignment, int studentId) const;

    // Late days of the latest valid version of a submission (0 = on time).
    int lateDays(const Assignment& assignment, const Submission& submission) const;

private:
    IExtensionRepository& extensions_;
    ISubmissionRepository& submissions_;
    const LatePenaltyPolicy& policy_;
};

}  // namespace mlms
