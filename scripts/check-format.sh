#!/usr/bin/env bash
# Verifies C++ and QML formatting. Exits non-zero and lists the offending files if any are
# unformatted; with --fix it reformats them in place instead.
#
#   scripts/check-format.sh            # files changed vs the merge-base with main
#   scripts/check-format.sh --staged   # files staged for commit
#   scripts/check-format.sh --all      # every tracked source file
#   scripts/check-format.sh --fix      # reformat the changed files in place
#   scripts/check-format.sh --fix --all
#
# qmlformat has no verify mode, so QML is checked by diffing its output against the file. Its
# stdout carries CRLF on Windows while the tree is LF, so the \r is stripped before comparing.
# Note that qmlformat's -n is --normalize, not a dry run: it reorders attributes and drops
# explicitly-written property names. Never use it here.
set -uo pipefail

cd "$(git rev-parse --show-toplevel)" || exit 1

# Anchored at the start so it excludes the vendored rcheevos/discord trees without also
# excluding our own code that happens to sit in a directory of the same name, such as
# libs/firelight/achievements/src/rcheevos.
VENDORED='^(thirdparty/|libs/rcheevos/|libs/discord/|lib/|include/rcheevos/|include/libretro/|include/discord/|build/|_cores/)'

fix=0
scope=--changed
for arg in "$@"; do
  case "$arg" in
    --fix) fix=1 ;;
    --all|--staged|--changed) scope=$arg ;;
    *) echo "usage: $0 [--changed|--staged|--all] [--fix]" >&2; exit 2 ;;
  esac
done

case "$scope" in
  --all)     files=$(git ls-files) ;;
  --staged)  files=$(git diff --cached --name-only --diff-filter=ACMR) ;;
  --changed) files=$(git diff --name-only --diff-filter=ACMR "$(git merge-base HEAD main 2>/dev/null || echo HEAD)") ;;
esac

files=$(echo "$files" | grep -vE "$VENDORED" | grep -E '\.(cpp|hpp|h|qml)$')
[ -z "$files" ] && { echo "check-format: nothing to check"; exit 0; }

if [ "$fix" = 1 ]; then
  cpp=$(echo "$files" | grep -E '\.(cpp|hpp|h)$')
  qml=$(echo "$files" | grep -E '\.qml$')
  [ -n "$cpp" ] && clang-format -i --style=file $cpp
  [ -n "$qml" ] && qmlformat -i $qml
  echo "check-format: reformatted $(echo "$files" | wc -l) file(s)"
  exit 0
fi

bad=""

for f in $files; do
  [ -f "$f" ] || continue

  case "$f" in
    *.qml)
      if ! qmlformat "$f" 2>/dev/null | tr -d '\r' | diff -q - "$f" >/dev/null 2>&1; then
        bad="$bad$f"$'\n'
      fi
      ;;
    *)
      if ! clang-format --style=file --dry-run -Werror "$f" >/dev/null 2>&1; then
        bad="$bad$f"$'\n'
      fi
      ;;
  esac
done

if [ -n "$bad" ]; then
  echo "check-format: these files are not formatted:"
  echo "$bad" | sed '/^$/d;s/^/  /'
  echo
  echo "Fix with:"
  echo "  clang-format -i --style=file <files>   # C++"
  echo "  qmlformat -i <files>                   # QML"
  exit 1
fi

echo "check-format: $(echo "$files" | wc -l) file(s) OK"
