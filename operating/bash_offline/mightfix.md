# Might-fix issues

## Issue 1 — Path traversal / arbitrary file write

`bvcs add` does not sanitize file paths. Running

```
bvcs add ../../etc/cronjob
```

stages the literal path `../../etc/cronjob`. When `do_commit` runs:

```bash
dest="$new_snap_dir/$staged_file"
mkdir -p "$(dirname "$dest")"
cp "$staged_file" "$dest"
```

`dest` resolves *outside* `.bvcs/objects/` — the snapshot escapes the object store. On `restore`, the reverse happens:

```bash
cp "$snap_path" "$file"
```

The raw user-supplied path is used as the destination, enabling arbitrary file overwrite anywhere the user has write permission.

**How to fix:** Normalize paths in `add_files` before the already-staged check and before writing to `$STAGING`:
- Strip leading `./` prefix.
- Reject paths containing `..` or starting with `/`, or resolve them to a safe canonical form.
- Apply the same normalization in the already-staged `grep` check so the same file cannot be staged under two spellings (see Issue 4).

## Issue 4 — Duplicate staging via inconsistent path spelling

The already-staged check uses an exact string match:

```bash
if grep -qxF "$file" "$STAGING" 2>/dev/null; then
```

`foo.txt` and `./foo.txt` refer to the same file but are different strings. Both pass the check, both get written to `$STAGING`, and during `do_commit` the file is copied twice into the snapshot under two different keys, silently duplicating it.

**How to fix:** Normalize paths at the same point described in Issue 1. Once every path is canonicalised, `./foo.txt` becomes `foo.txt` before the `grep` check, preventing the duplicate.
