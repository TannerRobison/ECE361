#!/bin/sh
# regress.sh: the HW6 regression test. Runs ./replay on every scenario in
# scenarios/ and compares its trace with the file of the same name in expected/.
# Usage, in hw06/ after make:   sh regress.sh
# Exits with 1 if any trace differs or there are no scenarios, so that
# make test fails.
fail=0
count=0
for scenario in scenarios/*.txt; do
    [ -e "$scenario" ] || break
    count=$((count + 1))
    name=$(basename "$scenario")
    if ./replay "$scenario" | diff - "expected/$name" > /dev/null; then
        echo "PASS  regression $name"
    else
        echo "FAIL  regression $name"
        fail=1
    fi
done
if [ "$count" -eq 0 ]; then
    echo "FAIL  regression: no scenarios in scenarios/"
    fail=1
fi
exit $fail
