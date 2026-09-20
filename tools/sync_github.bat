@echo off
REM ============================================================
REM  sync_github.bat - One-click sync GitHub <-> local PC (Windows)
REM
REM  Usage:
REM    sync_github.bat           - pull (GitHub -> local)
REM    sync_github.bat push      - push (local -> GitHub)
REM    sync_github.bat status    - show diff status only
REM    sync_github.bat clone     - fresh clone to local folder
REM
REM  Edit CONFIG below before first use.
REM ============================================================
setlocal enabledelayedexpansion

REM ==================== CONFIG ====================
set "GITHUB_USER=haum2002"
set "REPO_NAME=EWP-Control"
set "GITHUB_TOKEN="
set "LOCAL_DIR=%USERPROFILE%\EWP-Control"
set "BRANCH=main"
set "GIT_NAME=haum2002"
set "GIT_EMAIL=haum2002@users.noreply.github.com"
REM ==================== /CONFIG ====================

REM Check git installed
where git >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git is not installed!
    echo   Download from: https://git-scm.com/download/win
    pause
    exit /b 1
)

REM Build remote URL
if "%GITHUB_TOKEN%"=="" (
    set "REMOTE=https://github.com/!GITHUB_USER!/!REPO_NAME!.git"
    echo [WARN] No GITHUB_TOKEN set - push may prompt for password.
    echo [WARN] Edit this script and paste your token in GITHUB_TOKEN.
) else (
    set "REMOTE=https://!GITHUB_TOKEN!@github.com/!GITHUB_USER!/!REPO_NAME!.git"
)

set "ACTION=%~1"
if "%ACTION%"=="" set "ACTION=pull"

if /i "%ACTION%"=="clone" goto :clone
if /i "%ACTION%"=="pull" goto :pull
if /i "%ACTION%"=="push" goto :push
if /i "%ACTION%"=="status" goto :status
goto :usage

:clone
if exist "%LOCAL_DIR%" (
    echo [WARN] Folder already exists: %LOCAL_DIR%
    set /p "yn=Overwrite (deletes folder)? [y/N]: "
    if /i not "!yn!"=="y" exit /b 0
    rmdir /s /q "%LOCAL_DIR%"
)
echo [INFO] Cloning %REPO_NAME% from GitHub to: %LOCAL_DIR%
git clone -b %BRANCH% %REMOTE% "%LOCAL_DIR%"
cd /d "%LOCAL_DIR%"
git config user.name "%GIT_NAME%"
git config user.email "%GIT_EMAIL%"
echo [OK] Clone complete!
goto :end

:pull
if not exist "%LOCAL_DIR%\.git" (
    echo [ERROR] Not a git repo: %LOCAL_DIR%
    echo [INFO] Run: %0 clone  to create it first.
    pause
    exit /b 1
)
cd /d "%LOCAL_DIR%"
echo [INFO] Syncing GitHub -^> local (branch: %BRANCH%)
REM Stash uncommitted changes
git diff --quiet 2>nul
set "HAS_DIFF=%errorlevel%"
git diff --cached --quiet 2>nul
set /a "HAS_DIFF=%HAS_DIFF% + %errorlevel%"
if !HAS_DIFF! gtr 0 (
    echo [WARN] You have uncommitted local changes - stashing.
    git stash push -u -m "auto-stash before pull"
)
git fetch origin %BRANCH% 2>&1
git merge origin/%BRANCH% --no-edit 2>&1
echo [OK] Pull complete.
git log --oneline -1
REM Restore stash
git stash list 2>nul | findstr "auto-stash" >nul
if !errorlevel! equ 0 (
    echo [INFO] Restoring stashed changes...
    git stash pop 2>&1
)
goto :end

:push
if not exist "%LOCAL_DIR%\.git" (
    echo [ERROR] Not a git repo: %LOCAL_DIR%
    pause
    exit /b 1
)
cd /d "%LOCAL_DIR%"
echo [INFO] Syncing local -^> GitHub (branch: %BRANCH%)
git add -A
git diff --cached --quiet 2>nul
if errorlevel 1 (
    for /f "tokens=*" %%t in ('powershell -command "Get-Date -Format 'yyyy-MM-dd HH:mm:ss'"') do set "TIMESTAMP=%%t"
    set "MSG=Auto-commit: !TIMESTAMP!"
    set /p "custom=Commit message [!MSG!]: "
    if not "!custom!"=="" set "MSG=!custom!"
    git commit -m "!MSG!"
) else (
    echo [INFO] No new changes to commit.
)
git push %REMOTE% %BRANCH% 2>&1
echo [OK] Push complete.
git log --oneline -3
goto :end

:status
if not exist "%LOCAL_DIR%\.git" (
    echo [ERROR] Not a git repo: %LOCAL_DIR% - run %0 clone first.
    exit /b 1
)
cd /d "%LOCAL_DIR%"
echo === Local Status ===
git status --short
echo.
echo === Last 3 Commits ===
git log --oneline -3
echo.
git fetch origin %BRANCH% 2>nul
for /f %%a in ('git rev-list --count origin/%BRANCH%..HEAD 2^>nul') do set /a "AHEAD=%%a"
for /f %%b in ('git rev-list --count HEAD..origin/%BRANCH% 2^>nul') do set /a "BEHIND=%%b"
if "%AHEAD%"=="0" if "%BEHIND%"=="0" (
    echo [OK] In sync with GitHub.
) else (
    echo [WARN] Local is %AHEAD% ahead, %BEHIND% behind GitHub.
    if %AHEAD% gtr 0 echo   - run: %0 push
    if %BEHIND% gtr 0 echo   - run: %0 pull
)
goto :end

:usage
echo EWP-Control GitHub Sync Script
echo Usage: %0 [pull^|push^|status^|clone]
echo.
echo   pull  (default) - Download GitHub changes to local PC
echo   push          - Upload local changes to GitHub
echo   status        - Show what's different (no changes made)
echo   clone         - Fresh download to a new local folder
echo.
echo Config: edit %0 and set GITHUB_TOKEN, LOCAL_DIR, etc.

:end
if defined ACTION if /i "%ACTION%"=="pull" if /i not "%~1"=="pull" echo.
pause
