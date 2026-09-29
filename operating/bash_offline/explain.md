Markdown
# BVCS Architectural Overview & Core Logic

This document breaks down the core structural design and logic behind the Bash Version Control System (BVCS). 

---

## 1. Directory Structure & State Storage

BVCS relies on a flat-file database schema contained entirely within the hidden `.bvcs` folder to track staging environments and history tracking.

*   `.bvcs/staging`: A text file serving as an index cache. It stores newline-separated paths of files marked for tracking.
*   `.bvcs/HEAD`: A single pointer file holding the current active commit ID (e.g., `0003`).
*   `.bvcs/log`: A simple pipeline-delimited (`|`) sequential database storing the commit sequence metadata: `ID|Timestamp|Message`.
*   `.bvcs/objects/`: The storage engine containing static snapshot folders organized by commit ID.

---

## 2. The Core Git-Like Lifecycle

### File Staging (`add_files`)
When a file is staged, its path is appended to the `.bvcs/staging` registry. The system runs a quick duplication guard to protect against multi-line indexing redundancy:
```bash
if grep -qxF "$file" "$STAGING" 2>/dev/null; then ...
This ensures each relative file path appears exactly once in the staging area.

Snapshot Progression & Committing (do_commit)
The commit lifecycle mimics classic version control systems via a full directory roll-forward strategy:

ID Generation: It reads the total line count of .bvcs/log to compute the next incremental ID, zero-padded to four characters (e.g., 0001, 0002):

Bash
new_id=$(printf '%04d' $((log_lines + 1)))
Snapshot Inheritance: If a parent commit exists (extracted from HEAD), BVCS deep-copies the entire historical file structure into the new snapshot directory:

Bash
cp -r "$OBJECTS_DIR/$head_id/files/." "$new_snap_dir/"
Staging Merging & Cache Flush: It loops through the .bvcs/staging file, overwrites the inherited files with the modified versions from the workspace, updates HEAD, and wipes the staging registry (: > "$STAGING").

3. Workspace Inspection & Comparison
Status Engine (show_status)
The engine evaluates the differences between three trees: the current workspace, the staging index, and the current HEAD snapshot.

Modified Files Calculation: Loops through all tracked files found in the parent snapshot, checks if they are excluded from the current stage pool, and drops a flag if their structural hashes differ from the workspace copy via diff -q:

Bash
if [ ! -f "$rel" ] || ! diff -q "$snap_file" "$rel" >/dev/null 2>&1; then
Untracked Files Calculation: Leverages Unix find primitives to capture all filesystem nodes excluding .bvcs, filtering out anything already indexed or actively recorded in the active snapshot directory.

Differential Engine (show_diff)
Generates structural unified context patches. When evaluating a tracked file, it forces diff to explicitly name the comparison targets via label flags:

Bash
diff -u --label "$snap_path" --label "$file" "$snap_path" "$file"
If a file exists in the active snapshot history but has been deleted from the active workspace, it automatically generates a comparative deletion patch against the system null device: /dev/null.

Rollback Engine (restore_file)
Enforces manual checkouts by copying a targeted file out of the active immutable snapshot storage object mapped inside objects/$head_id/files/ and raw-writing it directly back into the working directory, effectively overriding uncommitted runtime mutations.