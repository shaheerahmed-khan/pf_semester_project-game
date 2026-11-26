#!/usr/bin/env bash
# Resync a fork after the upstream repo was history-rewritten.
# Run from inside a clone of your fork, checked out on main:
#   bash scripts/sync-fork.sh
# Optional: --upstream URL --remote NAME --branch NAME --no-rebase --dry-run
set -euo pipefail

UPSTREAM_URL="https://github.com/shaheerahmed-khan/pf_semester_project-game.git"
FORK_REMOTE="origin"
BRANCH="main"
REBASE_OTHER_BRANCHES=1
DRY_RUN=0

usage() {
  cat <<'EOF'
Usage: bash scripts/sync-fork.sh [options]

  --upstream URL   upstream repo to sync from (default: shaheerahmed-khan/pf_semester_project-game)
  --remote NAME    your fork remote (default: origin)
  --branch NAME    branch to reset (default: main)
  --no-rebase      do not rebase/force-push your other local branches
  --dry-run        print the plan without changing anything
  -h, --help       show this help

The script stashes uncommitted work, keeps a backup branch of your old main,
resets main to upstream, force-pushes it to your fork, and optionally rebases
your other local branches onto the rewritten history.
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --upstream) UPSTREAM_URL="$2"; shift 2 ;;
    --remote)   FORK_REMOTE="$2";   shift 2 ;;
    --branch)   BRANCH="$2";        shift 2 ;;
    --no-rebase) REBASE_OTHER_BRANCHES=0; shift ;;
    --dry-run)  DRY_RUN=1;          shift ;;
    -h|--help)  usage; exit 0 ;;
    *) echo "unknown option: $1" >&2; usage; exit 1 ;;
  esac
done

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "error: run this from inside a git clone of your fork" >&2
  exit 1
fi

cd "$(git rev-parse --show-toplevel)"

CURRENT=$(git symbolic-ref --quiet --short HEAD || true)
if [[ -z "$CURRENT" ]]; then
  echo "error: detached HEAD; check out '$BRANCH' first" >&2
  exit 1
fi
if [[ "$CURRENT" != "$BRANCH" ]]; then
  echo "error: checked out on '$CURRENT'; this script must run from '$BRANCH'" >&2
  exit 1
fi

if git remote get-url upstream >/dev/null 2>&1; then
  if [[ "$(git remote get-url upstream)" != "$UPSTREAM_URL" ]]; then
    git remote set-url upstream "$UPSTREAM_URL"
  fi
else
  git remote add upstream "$UPSTREAM_URL"
fi

git fetch --prune "$FORK_REMOTE"
git fetch --prune upstream

UPSTREAM_REF="upstream/$BRANCH"
if ! git rev-parse --verify --quiet "$UPSTREAM_REF" >/dev/null; then
  echo "error: $UPSTREAM_REF not found; check --upstream URL" >&2
  exit 1
fi

NEW_HEAD=$(git rev-parse "$UPSTREAM_REF")
OLD_HEAD=$(git rev-parse HEAD)

if [[ "$DRY_RUN" -eq 1 ]]; then
  echo "dry run:"
  echo "  fork remote:  $FORK_REMOTE"
  echo "  upstream:     $UPSTREAM_URL"
  echo "  reset $BRANCH: $OLD_HEAD -> $NEW_HEAD"
  echo "  force-push $BRANCH to $FORK_REMOTE"
  if [[ "$REBASE_OTHER_BRANCHES" -eq 1 ]]; then
    echo "  rebase other local branches onto $UPSTREAM_REF and force-push them"
  fi
  exit 0
fi

STASHED=0
if [[ -n "$(git status --porcelain)" ]]; then
  echo "stashing uncommitted work (including untracked files)"
  git stash push -u -m "sync-fork auto-stash $(date -u +%Y%m%dT%H%M%SZ)"
  STASHED=1
fi

BACKUP="backup/sync-$(date -u +%Y%m%dT%H%M%SZ)"
git branch "$BACKUP" "$OLD_HEAD"
echo "backup branch for your old $BRANCH: $BACKUP"

FAILED=0

if [[ "$OLD_HEAD" == "$NEW_HEAD" ]]; then
  echo "$BRANCH already matches upstream"
else
  git reset --hard "$NEW_HEAD"
  if git push --force-with-lease "$FORK_REMOTE" "$BRANCH:$BRANCH"; then
    echo "force-pushed $BRANCH to $FORK_REMOTE"
  else
    echo "error: could not force-push $BRANCH; is it protected on your fork?" >&2
    FAILED=1
  fi
fi

if [[ "$REBASE_OTHER_BRANCHES" -eq 1 ]]; then
  for b in $(git for-each-ref --format='%(refname:short)' refs/heads/); do
    case "$b" in
      "$BRANCH"|"$BACKUP"|backup/sync-*) continue ;;
    esac
    if git merge-base --is-ancestor "$b" "$OLD_HEAD"; then
      echo "skip $b: already merged into old $BRANCH"
      continue
    fi
    BASE=$(git merge-base "$b" "$OLD_HEAD" || true)
    if [[ -z "$BASE" ]]; then
      echo "skip $b: unrelated history"
      continue
    fi
    echo "rebasing $b onto $UPSTREAM_REF"
    if git rebase --onto "$UPSTREAM_REF" "$BASE" "$b"; then
      if ! git push --force-with-lease "$FORK_REMOTE" "$b:$b"; then
        echo "warning: could not force-push $b" >&2
        FAILED=1
      fi
    else
      git rebase --abort || true
      echo "warning: rebase of $b failed; it still points at the old history" >&2
      FAILED=1
    fi
  done
fi

git checkout "$BRANCH"

if [[ "$STASHED" -eq 1 ]]; then
  if git stash pop; then
    echo "restored your uncommitted work"
  else
    echo "warning: stash pop conflicted; your stash is still saved. Run 'git stash list'." >&2
    FAILED=1
  fi
fi

echo "done; backup: $BACKUP"
if [[ "$FAILED" -ne 0 ]]; then
  echo "finished with errors — review the messages above" >&2
  exit 1
fi
