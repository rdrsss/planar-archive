#!/bin/sh
# Reviewer SQL probe for V5-02-R15-A1-C01. Usage: sh sql_c01_review.sh <repo root> <scratch dir>
# Extracts the ```sql block of docs/vernacular/execution-contract.md, runs it in sqlite3 with foreign keys on,
# prints object counts, then runs each probe statement on a fresh copy and prints accepted or the error.
R="$1"; S="$2"
F="$R/docs/vernacular/execution-contract.md"
start=$(grep -n '^```sql$' "$F" | head -1 | cut -d: -f1)
end=$(awk -v s="$start" 'NR>s && /^```$/ {print NR; exit}' "$F")
sed -n "$((start+1)),$((end-1))p" "$F" > "$S/format3.sql"
echo "sqlite3 version: $(sqlite3 --version | cut -d' ' -f1)"
echo "sql block: execution-contract.md lines $((start+1)) to $((end-1)), $(grep -c '' "$S/format3.sql") lines"
rm -f "$S/base.db"
{ echo 'PRAGMA foreign_keys=ON;'; cat "$S/format3.sql"; } | sqlite3 -bail "$S/base.db"; echo "schema load exit: $?"
q(){ sqlite3 "$S/base.db" "$1"; }
echo "PRAGMA foreign_keys after ON in session: $(sqlite3 "$S/base.db" 'PRAGMA foreign_keys=ON; PRAGMA foreign_keys;')"
echo "tables: $(q "SELECT count(*) FROM sqlite_schema WHERE type='table' AND name NOT LIKE 'sqlite_%'")"
echo "strict tables: $(q "SELECT count(*) FROM pragma_table_list WHERE schema='main' AND type='table' AND name NOT LIKE 'sqlite_%' AND strict=1")"
echo "triggers: $(q "SELECT count(*) FROM sqlite_schema WHERE type='trigger'")"
echo "explicit indexes: $(q "SELECT group_concat(name, ', ') FROM sqlite_schema WHERE type='index' AND sql IS NOT NULL")"
echo "foreign_key_check rows: $(sqlite3 "$S/base.db" 'PRAGMA foreign_keys=ON; PRAGMA foreign_key_check;' | grep -c '')"
n=0
p(){ n=$((n+1)); cp "$S/base.db" "$S/t.db"; out=$(sqlite3 "$S/t.db" "PRAGMA foreign_keys=ON; INSERT INTO records VALUES(1,1,1,zeroblob(16),x'00'); $2" 2>&1); rc=$?; if [ $rc -eq 0 ]; then got=accepted; else got="refused: $(echo "$out" | sed 's/^Error: stepping, //')"; fi; echo "[$n] expect $1 | $3 | $got"; }
p refuse "INSERT INTO role_state VALUES('j','r',1,'running',0,NULL,1,1);" "role_state running, run_id NULL"
p accept "INSERT INTO role_state VALUES('j','r',1,'running',1,5,1,1);" "role_state running, run_id 5, drifted 1"
p refuse "INSERT INTO role_state VALUES('j','r',1,'ready',0,5,1,1);" "role_state ready, run_id 5"
p refuse "INSERT INTO role_state VALUES('j','r',1,'ready',1,NULL,1,1);" "role_state ready, drifted 1"
p refuse "INSERT INTO role_state VALUES('j','r',1,'blocked',1,NULL,1,1);" "role_state blocked, drifted 1"
p refuse "INSERT INTO role_state VALUES('j','r',1,'done',1,NULL,1,1);" "role_state done, drifted 1"
p refuse "INSERT INTO role_state VALUES('j','r',1,'stale',0,7,1,1);" "role_state stale, run_id 7"
p refuse "INSERT INTO role_state VALUES('j','r',1,'retired',0,3,1,1);" "role_state retired, run_id 3"
p accept "INSERT INTO role_state VALUES('j','r',1,'stale',0,NULL,1,1);" "role_state stale, run_id NULL, drifted 0 (control)"
p refuse "INSERT INTO role_state VALUES('j','r',1,'ready',0,NULL,1,1); UPDATE role_state SET state='running';" "role_state update ready to running keeping run_id NULL"
p refuse "INSERT INTO role_state VALUES('j','r',1,'running',1,5,1,1); UPDATE role_state SET state='done';" "role_state update running+drift to done keeping run_id and drifted"
p refuse "INSERT INTO transitions VALUES(1,1,'j','r',1,NULL,'ready',NULL,1,'activated');" "transitions after ready, after_drifted 1"
p refuse "INSERT INTO transitions VALUES(1,1,'j','r',1,'running','done',0,1,'completed');" "transitions after done, after_drifted 1"
p accept "INSERT INTO transitions VALUES(1,1,'j','r',1,'ready','running',0,1,'claimed');" "transitions after running, after_drifted 1"
p refuse "INSERT INTO transitions VALUES(1,1,'j','r',1,'ready','running',1,0,'claimed');" "transitions before ready, before_drifted 1"
p accept "INSERT INTO transitions VALUES(1,1,'j','r',1,'running','done',1,0,'completed');" "transitions before running, before_drifted 1"
p refuse "INSERT INTO transitions VALUES(1,1,'j','r',1,NULL,'ready',0,0,'activated');" "transitions before NULL, before_drifted 0"
p refuse "INSERT INTO transitions VALUES(1,1,'j','r',1,'ready','blocked',NULL,0,'prerequisite_unavailable');" "transitions before ready, before_drifted NULL"
p refuse "INSERT INTO transitions VALUES(1,0,'j','r',1,NULL,'ready',NULL,0,'activated');" "transitions ordinal 0"
p accept "INSERT INTO transitions VALUES(1,1,'j','r',1,NULL,'ready',NULL,0,'activated');" "transitions ordinal 1"
p accept "INSERT INTO evidence VALUES(1,1,1,'w','c',1,1,'pass','anything',NULL,NULL,0,NULL,0,0);" "evidence pass, reason 'anything' (D100: no SQL CHECK, reader-verified only)"
p accept "INSERT INTO evidence VALUES(1,1,1,'w','c',1,1,'fail','bogus',NULL,NULL,0,NULL,0,0);" "evidence fail, reason 'bogus' (D100: no SQL CHECK)"
p refuse "INSERT INTO evidence VALUES(1,1,1,'w','c',1,1,'pass',NULL,NULL,NULL,0,NULL,0,0);" "evidence reason NULL"
p refuse "INSERT INTO evidence VALUES(1,1,1,'w','c',1,1,'skipped','timeout',NULL,NULL,0,NULL,0,0);" "evidence outcome skipped"
p refuse "INSERT INTO evidence VALUES(1,1,1,'w','c',99,1,'fail','timeout',NULL,NULL,0,NULL,0,0);" "evidence definition_id 99 (no record)"
echo "probe statements: $n"
