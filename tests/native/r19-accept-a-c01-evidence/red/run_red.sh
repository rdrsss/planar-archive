#!/bin/bash
# red-check runner: usage run_red.sh
cd <repo>
R=tests/native/r19-accept-a-c01-evidence/red
S=tests/native/r19-accept-support
: > $R/outcomes.tsv
run_ct(){ n="$1"; f="$R/raw/$(echo "$n"|tr -c 'A-Za-z0-9\n' '_').log"; esc=$(printf '%s' "$n" | sed 's/[][\.*^$()+?{}|]/\\&/g')
 { echo "CMD: ctest --test-dir build/rewrite-debug -R '$esc' --no-tests=error --output-on-failure"; ctest --test-dir build/rewrite-debug -R "$esc" --no-tests=error --output-on-failure; echo "EXIT: $?"; } > "$f" 2>&1
 printf '%s\t%s\n' "$n" "$f" >> $R/outcomes.tsv; }
run_py(){ mod=$1; cls=$2; t=$3; f="$R/raw/${cls}.${t}.log"
 { echo "CMD: (cd $S && python3 -m unittest $mod.$cls.$t)"; (cd $S && python3 -m unittest -v $mod.$cls.$t); echo "EXIT: $?"; } > "$f" 2>&1
 printf '%s.%s\t%s\n' "$cls" "$t" "$f" >> $R/outcomes.tsv; }
while IFS=, read -r kind a b c; do
 case $kind in ct) run_ct "$a";; py) run_py "$a" "$b" "$c";; esac
done < $R/tests.csv
