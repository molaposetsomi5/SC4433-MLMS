# Demonstration video

**Video link:** _paste your link here after uploading (YouTube "Unlisted" or Google Drive "Anyone with the link")_

Length: about 5-8 minutes.

## What the video shows (recording script)
1. **Build and tests (1 min)** - in the MSYS2 UCRT64 terminal: `cmake --build build` then `ctest --test-dir build` -> "100% tests passed, 0 tests failed out of 128".
2. **Start with demo data** - `./build/mlms.exe --db video.db --seed-demo`.
3. **Student (stu1)** - option 1 (assignments list), option 2 submit Lab 1 -> status LATE with the number of days late. Option 3 shows no released marks yet.
4. **Lecturer (lec.molefi)** - option 4 view submissions for assignment 1, option 5 mark it (raw 40) -> penalty and final mark shown, "Awaiting HoD moderation".
5. **HoD (hod.cs)** - option 1 pending marks, option 2 approve, option 5 release assignment 1, option 6 audit log (who, when, old value, new value, reason).
6. **Student (stu1)** - option 3 now shows the mark, late penalty and feedback.
7. **CR1 (v1.1)** - lecturer option 8 extends Lab 2 for the whole class; student stu2 option 1 shows "extended for the whole class (was ...)".
8. **GitHub** - show the repository, the green CI run in the Actions tab and the v1.0 / v1.1 releases.

All demo passwords: `Password123`.
