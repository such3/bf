#!/usr/bin/env bash
# tests/run_tests.sh
# Runs every program in tests/programs and compares stdout and exit status.
# Usage: tests/run_tests.sh [path-to-bf]

BF="${1:-./bf}"
PROGRAMS=tests/programs
EXPECTED=tests/expected
LIMIT=5          # seconds; protects the run from programs that never stop

pass=0
fail=0
out=$(mktemp)
trap 'rm -f "$out"' EXIT

# check NAME YES_OR_NO DETAIL
check() {
    if [ "$2" = yes ]; then
        echo "PASS  $1"
        pass=$((pass + 1))
    else
        echo "FAIL  $1  $3"
        fail=$((fail + 1))
    fi
}

# Programs with expected files.
for prog in "$PROGRAMS"/*.bf; do
    name=$(basename "$prog" .bf)

    want_out="$EXPECTED/$name.out"
    [ -f "$want_out" ] || want_out=/dev/null      # no file means: expect no output

    want_status=0
    [ -f "$EXPECTED/$name.status" ] && want_status=$(cat "$EXPECTED/$name.status")

    args=()
    [ -f "$EXPECTED/$name.args" ] && read -r -a args < "$EXPECTED/$name.args"

    timeout "$LIMIT" "$BF" "$prog" "${args[@]}" > "$out" 2> /dev/null
    got_status=$?

    if [ "$got_status" = "$want_status" ] && cmp -s "$out" "$want_out"; then
        check "$name" yes
    else
        check "$name" no "(exit $got_status, wanted $want_status)"
    fi
done

# Cases that have no program file.
# expect_status DESCRIPTION WANTED_STATUS COMMAND...
expect_status() {
    desc=$1; want=$2; shift 2
    timeout "$LIMIT" "$@" > /dev/null 2>&1
    got=$?
    if [ "$got" = "$want" ]; then
        check "$desc" yes
    else
        check "$desc" no "(exit $got, wanted $want)"
    fi
}
expect_status "no arguments" 1 "$BF"
expect_status "missing file" 1 "$BF" "$PROGRAMS/does_not_exist.bf"

echo
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]      # the script's own exit status: 0 only if nothing failed
