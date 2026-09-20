#!/usr/bin/env bash
# ============================================================
#  sync_github.sh — One-click sync between GitHub repo and local PC
#
#  Usage:
#    chmod +x sync_github.sh
#    ./sync_github.sh            # pull (GitHub → local)
#    ./sync_github.sh push       # push (local → GitHub)
#    ./sync_github.sh status     # show diff status only
#    ./sync_github.sh clone      # fresh clone to local folder
#
#  Edit the CONFIG section below before first use.
# ============================================================
set -euo pipefail

# ==================== CONFIG ====================
# Your GitHub username and repo name
GITHUB_USER="haum2002"
REPO_NAME="EWP-Control"

# Your GitHub Personal Access Token (PAT)
# Get one at: https://github.com/settings/tokens (classic, with "repo" scope)
# OR fine-grained with Contents: Read+Write
GITHUB_TOKEN=""   # <-- PASTE YOUR TOKEN HERE  (e.g. github_pat_xxxx...)

# Local folder on your PC where the project lives
# Default: ~/EWP-Control (home directory)
LOCAL_DIR="${HOME}/EWP-Control}"

# Git branch (change if you use a different branch name)
BRANCH="main"

# Git identity (for commits when pushing)
GIT_NAME="haum2002"
GIT_EMAIL="haum2002@users.noreply.github.com"
# ==================== /CONFIG ====================

# Colors
RED='\033[0;31m'; GRN='\033[0;32m'; YLW='\033[1;33m'; BLU='\033[0;34m'; NC='\033[0m'
info()  { echo -e "${BLU}[INFO]${NC}  $*"; }
ok()    { echo -e "${GRN}[OK]${NC}    $*"; }
warn()  { echo -e "${YLW}[WARN]${NC}  $*"; }
err()   { echo -e "${RED}[ERROR]${NC} $*"; }

# ---- Build remote URL ----
if [ -z "$GITHUB_TOKEN" ]; then
    REMOTE="https://github.com/${GITHUB_USER}/${REPO_NAME}.git"
    warn "No GITHUB_TOKEN set — will use public HTTPS (push may prompt for password)."
    warn "Edit this script and paste your token in GITHUB_TOKEN."
else
    REMOTE="https://${GITHUB_TOKEN}@github.com/${GITHUB_USER}/${REPO_NAME}.git"
fi

# ---- Detect OS for path handling ----
OS_TYPE="$(uname -s)"
case "$OS_TYPE" in
    MINGW*|MSYS*|CYGWIN*)  IS_WINDOWS=1 ;;   # Git Bash / MSYS2 on Windows
    *)                     IS_WINDOWS=0 ;;
esac

# ---- Helper: check git installed ----
check_git() {
    if ! command -v git &>/dev/null; then
        err "Git is not installed!"
        if [ "$IS_WINDOWS" = "1" ]; then
            echo "  Download from: https://git-scm.com/download/win"
        elif [ "$OS_TYPE" = "Darwin" ]; then
            echo "  Install with:  xcode-select --install"
        else
            echo "  Install with:  sudo apt install git   (or your distro's package manager)"
        fi
        exit 1
    fi
    ok "Git found: $(git --version)"
}

# ---- ACTION: clone ----
do_clone() {
    check_git
    if [ -d "$LOCAL_DIR" ]; then
        warn "Folder already exists: $LOCAL_DIR"
        read -rp "Overwrite? (this deletes the folder) [y/N] " yn
        case "$yn" in
            y|Y) rm -rf "$LOCAL_DIR" ;;
            *)   echo "Aborted."; exit 0 ;;
        esac
    fi
    info "Cloning ${REPO_NAME} from GitHub to: $LOCAL_DIR"
    git clone -b "$BRANCH" "$REMOTE" "$LOCAL_DIR"
    cd "$LOCAL_DIR"
    git config user.name  "$GIT_NAME"
    git config user.email "$GIT_EMAIL"
    ok "Clone complete! $(find . -type f -not -path './.git/*' | wc -l) files."
    echo
    info "Next: open KiCad, or run firmware tools (python3 tools/verify_release.py)"
}

# ---- ACTION: pull (GitHub → local) ----
do_pull() {
    check_git
    if [ ! -d "$LOCAL_DIR/.git" ]; then
        err "Not a git repo: $LOCAL_DIR"
        info "Run:  $0 clone  to create it first."
        exit 1
    fi
    cd "$LOCAL_DIR"
    info "Syncing GitHub → local (branch: $BRANCH)"

    # Save any local changes (stash)
    if ! git diff --quiet || ! git diff --cached --quiet; then
        warn "You have uncommitted local changes — stashing them."
        git stash push -u -m "auto-stash before pull $(date +%Y%m%d_%H%M%S)"
    fi

    # Fetch + merge
    git fetch origin "$BRANCH" 2>&1 | sed 's/^/  /'
    LOCAL_HASH=$(git rev-parse HEAD)
    REMOTE_HASH=$(git rev-parse "origin/$BRANCH")

    if [ "$LOCAL_HASH" = "$REMOTE_HASH" ]; then
        ok "Already up-to-date — local is in sync with GitHub."
    else
        info "Pulling changes..."
        git merge "origin/$BRANCH" --no-edit 2>&1 | sed 's/^/  /'
        # Count changed files
        CHANGED=$(git diff --stat "$LOCAL_HASH" HEAD | tail -1)
        ok "Synced! Changes: $CHANGED"
    fi

    # Restore stash if any
    if git stash list | head -1 | grep -q "auto-stash"; then
        info "Restoring your stashed local changes..."
        git stash pop 2>&1 | sed 's/^/  /' || warn "Stash pop failed — run 'git stash pop' manually."
    fi
    echo
    ok "Pull complete. Latest commit:"
    git log --oneline -1
}

# ---- ACTION: push (local → GitHub) ----
do_push() {
    check_git
    if [ ! -d "$LOCAL_DIR/.git" ]; then
        err "Not a git repo: $LOCAL_DIR"
        exit 1
    fi
    cd "$LOCAL_DIR"
    info "Syncing local → GitHub (branch: $BRANCH)"

    # Show what will be pushed
    git fetch origin "$BRANCH" 2>/dev/null
    LOCAL_HASH=$(git rev-parse HEAD)
    REMOTE_HASH=$(git rev-parse "origin/$BRANCH" 2>/dev/null || echo "none")

    if [ "$LOCAL_HASH" = "$REMOTE_HASH" ]; then
        ok "No new commits to push — local and GitHub are in sync."
        # Check for uncommitted changes
        if git diff --quiet && git diff --cached --quiet; then
            echo "  (no uncommitted changes either)"
        else
            warn "You have uncommitted changes:"
            git status --short | sed 's/^/  /'
            read -rp "Commit and push these changes? [y/N] " yn
            case "$yn" in
                y|Y)
                    git add -A
                    COMMIT_MSG="Auto-commit: $(date '+%Y-%m-%d %H:%M:%S')"
                    read -rp "Commit message [$COMMIT_MSG]: " custom_msg
                    git commit -m "${custom_msg:-$COMMIT_MSG}"
                    git push "$REMOTE" "$BRANCH" 2>&1 | sed 's/^/  /'
                    ok "Pushed to GitHub!"
                    ;;
                *) echo "Aborted."; exit 0 ;;
            esac
        fi
        return
    fi

    # Push existing commits
    git push "$REMOTE" "$BRANCH" 2>&1 | sed 's/^/  /'
    ok "Pushed to GitHub!"
    git log --oneline -3 | sed 's/^/  /'
}

# ---- ACTION: status ----
do_status() {
    check_git
    if [ ! -d "$LOCAL_DIR/.git" ]; then
        err "Not a git repo: $LOCAL_DIR — run '$0 clone' first."
        exit 1
    fi
    cd "$LOCAL_DIR"
    echo "=== Local Status ==="
    git status --short
    echo
    echo "=== Branch ==="
    git branch --show-current
    echo
    echo "=== Last 3 Commits ==="
    git log --oneline -3
    echo
    git fetch origin "$BRANCH" 2>/dev/null
    AHEAD=$(git rev-list --count "origin/$BRANCH..HEAD" 2>/dev/null || echo "?")
    BEHIND=$(git rev-list --count "HEAD..origin/$BRANCH" 2>/dev/null || echo "?")
    echo "=== Sync Status (vs origin/$BRANCH) ==="
    if [ "$AHEAD" = "0" ] && [ "$BEHIND" = "0" ]; then
        ok "In sync with GitHub."
    else
        warn "Local is $AHEAD commits ahead, $BEHIND behind GitHub."
        if [ "$AHEAD" -gt 0 ] 2>/dev/null; then echo "  → run: $0 push"; fi
        if [ "$BEHIND" -gt 0 ] 2>/dev/null; then echo "  → run: $0 pull (or just: $0)"; fi
    fi
}

# ---- Main ----
ACTION="${1:-pull}"
case "$ACTION" in
    pull)   do_pull ;;
    push)   do_push ;;
    status) do_status ;;
    clone)  do_clone ;;
    *)
        echo "EWP-Control GitHub Sync Script"
        echo "Usage: $0 [pull|push|status|clone]"
        echo
        echo "  pull   (default) — Download GitHub changes to local PC"
        echo "  push           — Upload local changes to GitHub"
        echo "  status         — Show what's different (no changes made)"
        echo "  clone          — Fresh download to a new local folder"
        echo
        echo "Config: edit $0 and set GITHUB_TOKEN, LOCAL_DIR, etc."
        exit 1
        ;;
esac
