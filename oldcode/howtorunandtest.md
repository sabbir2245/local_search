# How to Run and Test A1 / B1 / C1

Each `.l` file is a standalone flex scanner. Build and test them one at a time.

## Requirements

- `flex` (tested with 2.6.4)
- `g++` (C++17)
- B1 needs `general.h` and `symbol_table.cpp` from this directory (pass `-I.`)

## Build

```sh
# A1 - XML tag validator
flex -o a1.c 2205040_A1.l && g++ -o a1 a1.c

# B1 - C tokenizer with octal/hex constants + Lua comments
flex -o b1.c 2205040_B1.l && g++ -o b1 b1.c -I.

# C1 - Python dictionary validator
flex -o c1.c 2205040_C1.l && g++ -o c1 c1.c
```

## Test

### A1 — valid XML (expects: `Valid XML structure`)

```sh
cat > a1_in.xml <<'EOF'
<book>
<title>Compiler Design</title>
<author>A. Aho</author>
</book>
EOF
./a1 a1_in.xml
```

Error case — tag mismatch (expects: `Invalid XML: tag mismatch </author>`):

```sh
cat > a1_bad.xml <<'EOF'
<book>
<title>Compiler Design</author>
</book>
EOF
./a1 a1_bad.xml
```

### B1 — valid constants and Lua comment

Run B1 inside its own directory (it writes `2205040_token.txt` / `2205040_log.txt` in the current directory):

```sh
mkdir -p b1_test && cd b1_test
cat > in.c <<'EOF'
int permission = 0755;
int mask = 0x2Af;
--[[ Lua comment
spanning two lines ]]
EOF
../b1 in.c
cat 2205040_token.txt   # <CONST_OCT, 0755> <CONST_HEX, 0x2Af> ...
grep CONST_OCT 2205040_log.txt
grep CONST_HEX 2205040_log.txt
grep Lua 2205040_log.txt   # Line no 3: Lua multiline comment ending at line 4 found
```

Error case — invalid numbers (expects 3 errors, no number tokens):

```sh
cat > in_bad.c <<'EOF'
int a = 0789;
int b = 0x2G7;
int c = 0X;
EOF
../b1 in_bad.c
grep Error 2205040_log.txt   # Invalid octal constant 0789 / Invalid hexadecimal constant ...
grep -c 'Total errors' 2205040_log.txt
tail -1 2205040_log.txt      # Total errors: 3
```

### C1 — valid dictionary (expects: `Valid Python Dictionary`)

```sh
cat > c1_in.txt <<'EOF'
{
	'name': "alice",
	101: { "city": 'sylhet', "active": True },
	"phone": None
}
EOF
./c1 c1_in.txt
```

Error case — missing pair separator (expects: `Error: missing pair separator`):

```sh
cat > c1_bad.txt <<'EOF'
{ "name": "alice" "city": "dhaka" }
EOF
./c1 c1_bad.txt
```

## Notes

- `A1`/`C1` print their verdict to stdout. `B1` writes `2205040_token.txt` and `2205040_log.txt` in the directory it is run from.
- flex does not accept `/* */` comments in the rules section unless indented; keep such comments indented by at least one space.
- `A1` emits a harmless flex warning `rule cannot be matched` for its dead fallback `.` rule; it can be ignored.
