# MLMS Installation Guide (for the school technician)

Time needed: about 20-30 minutes. No internet connection is needed after the files are copied.

## 1. What you need
- One school computer running Windows 10/11 or Ubuntu 22.04/24.04.
- The release folder `mlms-v1.x` (from the USB drive or the GitHub release), or the source code.
- Administrator rights on the computer for the install step only.

## 2a. Install on Windows
1. Install **MSYS2** from the installer on the USB drive (`msys2-x86_64-*.exe`), keep the default folder `C:\msys64`.
2. Open **"MSYS2 UCRT64"** from the Start menu and run:
   ```
   pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-pkgconf mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-libsodium mingw-w64-ucrt-x86_64-gtest
   ```
3. Go to the source folder and build:
   ```
   cd /c/MLMS
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ctest --test-dir build
   ```
   The last line must end with `100% tests passed`.
4. Create the data folder `C:\MLMS\data` and start the program once:
   ```
   ./build/mlms.exe --db /c/MLMS/data/mlms.db
   ```
5. Create a desktop shortcut whose target is
   `C:\msys64\ucrt64\bin\mintty.exe -e C:\MLMS\build\mlms.exe --db C:\MLMS\data\mlms.db`.

## 2b. Install on Ubuntu
```
sudo apt install build-essential cmake pkg-config libsqlite3-dev libsodium-dev libgtest-dev
cd ~/MLMS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build                     # must say 100% tests passed
sudo cmake --install build                 # copies mlms to /usr/local/bin
sudo mkdir -p /var/lib/mlms && sudo chown $USER /var/lib/mlms
mlms --db /var/lib/mlms/mlms.db
```

## 3. First start
On an empty database the program asks for the **administrator** username, full name and password
(at least 8 characters). Write these down and store them safely. Then log in as the administrator and:
1. Create the departments (menu 6).
2. Create HoD, lecturer and student accounts (menu 1).
3. Create courses (menu 8), assign lecturers (menu 10) and enrol students (menu 11).

## 4. Important limits
- v1.x is designed for **one computer**. Do not put the database file on a shared network drive -
  SQLite file locking over a network is unreliable and the file can be damaged.
- The program uses the computer's clock for deadlines. Make sure the date, time and time zone are correct.

## 5. Backup (do this every Friday and before any upgrade)
1. Make sure nobody is using MLMS (everyone has logged out and the window is closed).
2. Copy the database file to the backup drive with the date in the name, e.g.
   - Windows: `copy C:\MLMS\data\mlms.db E:\backup\mlms-2026-11-20.db`
   - Ubuntu: `cp /var/lib/mlms/mlms.db /media/backup/mlms-2026-11-20.db`
3. Keep at least the last four weekly copies.

## 6. Restore
1. Close MLMS.
2. Rename the damaged file (e.g. `mlms.db` -> `mlms-broken.db`).
3. Copy the chosen backup to the data folder and rename it to `mlms.db`.
4. Start MLMS and log in as the administrator. Check that users, assignments and the audit log are there.

## 7. Upgrading (e.g. v1.0 -> v1.1)
1. Take a backup (section 5).
2. Replace the program files with the new version and rebuild (section 2).
3. Start the program with the same `--db` file. The database is upgraded automatically
   (the schema version is stored inside the file); existing data is kept.

## 8. Troubleshooting
| Message | What to do |
|---|---|
| `Could not open database` | Check the folder exists and the user may write to it. |
| `database is locked` | Another copy of MLMS is open. Close it and try again. |
| `Account locked ... Try again after` | Wait 15 minutes, or the administrator resets the password (menu 5). |
| Build cannot find `libsodium` or `sqlite3` | Repeat the package install step (2a.2 or 2b). |
