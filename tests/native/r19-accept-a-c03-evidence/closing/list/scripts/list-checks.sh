#!/bin/zsh
# Closing commit-list checks; run from the repository root. Writes closing/list/*.
R=$PWD; SCR=$1; L=tests/native/r19-accept-a-c03-evidence/COMMIT-LIST.txt; O=tests/native/r19-accept-a-c03-evidence/closing/list
mkdir -p $O/raw; export PYTHONDONTWRITEBYTECODE=1
TMPU=$(dirname $(getconf DARWIN_USER_TEMP_DIR)); TMPU_ID=$(echo $TMPU | awk -F/ '{print $(NF-1)"/"$NF}')
python3 -B tests/native/r19-accept-support/run_acceptance.py commit-list --list $L --repo-root . > $O/raw/commit-list.stdout 2> $O/raw/commit-list.stderr; echo $? > $O/commit-list.exit; grep -oE '[0-9]+ entries' $O/raw/commit-list.stdout > $O/commit-list.count
python3 -B scripts/redact-local-paths.py --repo-path $R --home $HOME check $(cat $L) > $O/raw/redact-check.stdout 2> $O/raw/redact-check.stderr; echo $? > $O/redact-check.exit
grep -v -e '^src/' -e '^tests/native/r19-accept-support/' $L > $SCR/closing/scan-paths.txt
ST=$SCR/closing/scan-stage; rm -rf -- "$SCR/closing/scan-stage"; mkdir -p $ST/tree
while IFS= read -r f; do mkdir -p "$ST/tree/$(dirname $f)"; cp -p "$f" "$ST/tree/$f"; done < $SCR/closing/scan-paths.txt
python3 -B tests/native/r19-accept-support/run_acceptance.py scan --root $ST/tree --repo-path $R --home $HOME --scratch $SCR --temp-root $TMPU --private-temp-root /private$TMPU --export-root $SCR/r19-candidate-c01-rev2/export > $O/raw/scan.stdout 2> $O/raw/scan.stderr; echo $? > $O/scan.exit; sed -e 's|^HOST-PATH tree/||' $O/raw/scan.stdout | sort -u > $O/scan-hits.txt
xargs grep -nE '[/]var[/]folders[/][a-z0-9_]{2}[/]|[/]private[/]var[/]folders[/]|[/]private[/]tmp[/]claude|[/]tmp[/]claude' < $SCR/closing/scan-paths.txt > $O/raw/grep-c1.out; echo $? > $O/grep-c1.exit; cut -d: -f1 $O/raw/grep-c1.out | sort -u > $O/grep-c1-files.txt
xargs grep -nE -- '-[U]sers-|claude-[5]01|aa194b3[a]-' < $SCR/closing/scan-paths.txt > $O/raw/grep-c2.out; echo $? > $O/grep-c2.exit; cut -d: -f1 $O/raw/grep-c2.out | sort -u > $O/grep-c2-files.txt
xargs grep -nF -- "$TMPU_ID" < $SCR/closing/scan-paths.txt > $O/raw/grep-c3.out; echo $? > $O/grep-c3.exit; cut -d: -f1 $O/raw/grep-c3.out | sort -u > $O/grep-c3-files.txt
python3 -B $SCR/pin_grep.py $L > $O/raw/pin-grep.out; echo $? > $O/pin-grep.exit; cut -d: -f1,2 $O/raw/pin-grep.out > $O/pin-grep.txt
wc -l < $L | tr -d ' ' > $O/list-count.txt; wc -l < $SCR/closing/scan-paths.txt | tr -d ' ' > $O/scan-path-count.txt
for x in commit-list redact-check scan grep-c1 grep-c2 grep-c3 pin-grep; do echo "$x exit $(cat $O/$x.exit)"; done
