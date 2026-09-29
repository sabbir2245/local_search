#!/usr/bin/bash

BVCS_DIR=".bvcs"
OBJECTS_DIR="$BVCS_DIR/objects"
STAGING="$BVCS_DIR/staging"
LOG="$BVCS_DIR/log"
HEAD_FILE="$BVCS_DIR/HEAD"

usage() {
    cat <<'EOF'
Usage: bvcs <command> [arguments]

Commands:
    init                    Initialize a new BVCS repository
    add <file>...           Stage files for the next commit
    status                  Show staged, modified, and untracked files
    commit -m <message>     Create a commit with all staged files
    log                     Display the full commit history
    diff [file]             Compare working files with the latest commit
    restore <file>          Restore a file from the latest commit
    help                    Show this help message
EOF
}

check_repo() {
    if [ ! -d "$BVCS_DIR" ]; then
        echo "Error: Not a BVCS repository. Run 'init' first."
        exit 1
    fi
}


init_repo() {
    if [ -d "$BVCS_DIR" ]; then
        echo "Error: BVCS repository already exists."
        exit 1
    fi
    mkdir -p "$OBJECTS_DIR"
     > "$STAGING" 
     > "$LOG"
     > "$HEAD_FILE"
    echo "Initialized empty BVCS repository."
}

add_files() {
    if [ $# -eq 0 ]; then
        echo "Error: No files specified."
        exit 1
    fi
    for file in "$@"; do
        if [ ! -f "$file" ]; then
            echo "Error: '$file' not found."
            continue
        fi
        if grep -qxF "$file" "$STAGING" 2>/dev/null; then
            echo "Already staged: $file"
            continue
        fi
        echo "$file" >> "$STAGING"
        echo "Staged: $file"
    done
}

# status
show_status() {
    local head_id
    head_id=$(cat "$HEAD_FILE" 2>/dev/null)
    local tmpf=/tmp/bvcs_status.$$
    local printed=false

    # Staged files
    if [ -s "$STAGING" ]; then
        echo "Staged for commit:"
        cat "$STAGING"
        echo
        printed=true
    fi

    # Modified files (in HEAD, not staged, differs from working copy)
    if [ -n "$head_id" ] && [ -d "$OBJECTS_DIR/$head_id/files" ]; then
        local snap_dir="$OBJECTS_DIR/$head_id/files"
        find "$snap_dir" -type f 2>/dev/null > "$tmpf"
        local modified=""
        while IFS= read -r snap_file; do
            local rel="${snap_file#$snap_dir/}"
            if grep -qxF "$rel" "$STAGING" 2>/dev/null; then
                continue
            fi
            if [ ! -f "$rel" ] || ! diff -q "$snap_file" "$rel" >/dev/null 2>&1; then
                modified="$modified$rel"$'\n'
            fi
        done < "$tmpf"
        if [ -n "$modified" ]; then
            echo "Modified (not staged):"
            echo "$modified" | grep -v '^$' | sort
            echo
            printed=true
        fi
    fi

    # Untracked files
    find . -type f ! -path './.bvcs/*' 2>/dev/null | sed 's|^\./||' > "$tmpf"
    local untracked=""
    while IFS= read -r wf; do
        if grep -qxF "$wf" "$STAGING" 2>/dev/null; then
            continue
        fi
        if [ -n "$head_id" ] && [ -f "$OBJECTS_DIR/$head_id/files/$wf" ]; then
            continue
        fi
        untracked="$untracked$wf"$'\n'
    done < "$tmpf"
    rm -f "$tmpf"
    if [ -n "$untracked" ]; then
        echo "Untracked files:"
        echo "$untracked" | grep -v '^$' | sort
        echo
        printed=true
    fi

    if [ "$printed" = false ]; then
        echo "Nothing to commit, working tree clean."
    fi
}


do_commit() {
    local message=""
    if [ $# -ge 2 ] && [ "$1" = "-m" ]; then
        shift
        message="$1"
    else
        echo "Error: Commit message required. Use -m \"message\"."
        exit 1
    fi
    if [ -z "$message" ]; then
        echo "Error: Commit message required. Use -m \"message\"."
        exit 1
    fi
    if [ ! -s "$STAGING" ]; then
        echo "Error: Nothing to commit."
        exit 1
    fi

    local head_id
    head_id=$(cat "$HEAD_FILE" 2>/dev/null)

    local log_lines=0
    if [ -f "$LOG" ]; then
        log_lines=$(wc -l < "$LOG")
    fi
    local new_id
    new_id=$(printf '%04d' $((log_lines + 1)))

    local new_snap_dir="$OBJECTS_DIR/$new_id/files"
    mkdir -p "$new_snap_dir"

    
    if [ -n "$head_id" ] && [ -d "$OBJECTS_DIR/$head_id/files" ]; then
        cp -r "$OBJECTS_DIR/$head_id/files/." "$new_snap_dir/"
    fi

    
    local file_count=0
    while IFS= read -r staged_file; do
        dest="$new_snap_dir/$staged_file"
        mkdir -p "$(dirname "$dest")"
        if cp "$staged_file" "$dest"; then
            file_count=$((file_count + 1))
        else
            echo "Warning: could not copy '$staged_file' — skipping." >&2
        fi
    done < "$STAGING"

    local timestamp
    timestamp=$(date +'%Y-%m-%d %H:%M:%S')
    echo "$timestamp" > "$OBJECTS_DIR/$new_id/timestamp"
    echo "$message" > "$OBJECTS_DIR/$new_id/message"

    echo "$new_id|$timestamp|$message" >> "$LOG"
    echo "$new_id" > "$HEAD_FILE"

    : > "$STAGING"

    echo "[$new_id] $message"
    echo "$file_count file(s) committed."
}


show_log() {
    if [ ! -s "$LOG" ]; then
        echo "No commits yet."
        return
    fi
    tac "$LOG" | while IFS='|' read -r id ts msg; do
        echo "commit $id"
        echo "Date:   $ts"
        echo "Message: $msg"
        echo
    done
}


show_diff() {
    local head_id
    head_id=$(cat "$HEAD_FILE" 2>/dev/null)
    if [ -z "$head_id" ]; then
        echo "Error: No commits yet."
        exit 1
    fi
    local snap_dir="$OBJECTS_DIR/$head_id/files"

    if [ $# -eq 0 ]; then
        # Diff all tracked files
        local tmpf=/tmp/bvcs_diff.$$
        find "$snap_dir" -type f 2>/dev/null | sed "s|^$snap_dir/||" | sort > "$tmpf"
        while IFS= read -r rel; do
            local snap_path="$snap_dir/$rel"
            if [ -f "$rel" ]; then
                if diff -q "$snap_path" "$rel" >/dev/null 2>&1; then
                    echo "$rel: no changes."
                else
                    diff -u --label "$snap_path" --label "$rel" "$snap_path" "$rel"
                fi
            else
                diff -u --label "$snap_path" --label "$rel" "$snap_path" /dev/null
            fi
        done < "$tmpf"
        rm -f "$tmpf"
    else
        local file="$1"
        local snap_path="$snap_dir/$file"
        if [ ! -f "$snap_path" ]; then
            echo "Error: '$file' is not tracked."
            exit 1
        fi
        if [ -f "$file" ]; then
            if diff -q "$snap_path" "$file" >/dev/null 2>&1; then
                echo "$file: no changes."
            else
                diff -u --label "$snap_path" --label "$file" "$snap_path" "$file"
            fi
        else
            diff -u --label "$snap_path" --label "$file" "$snap_path" /dev/null
        fi
    fi
}


restore_file() {
    if [ $# -eq 0 ]; then
        echo "Error: No file specified."
        exit 1
    fi
    local file="$1"
    local head_id
    head_id=$(cat "$HEAD_FILE" 2>/dev/null)
    if [ -z "$head_id" ]; then
        echo "Error: No commits yet."
        exit 1
    fi
    local snap_path="$OBJECTS_DIR/$head_id/files/$file"
    if [ ! -f "$snap_path" ]; then
        echo "Error: '$file' not found in commit $head_id."
        exit 1
    fi
    read -p "Restore '$file' from commit $head_id? [y/N]: " ans
    if [ "$ans" != "y" ] && [ "$ans" != "Y" ]; then
        echo "Aborted."
        exit 0
    fi
    mkdir -p "$(dirname "$file")"
    cp "$snap_path" "$file"
    echo "Restored: $file"
}

case "${1:-help}" in
    init)
        init_repo
        ;;
    add)
        check_repo
        shift
        add_files "$@"
        ;;
    status)
        check_repo
        show_status
        ;;
    commit)
        check_repo
        shift
        do_commit "$@"
        ;;
    log)
        check_repo
        show_log
        ;;
    diff)
        check_repo
        shift
        show_diff "$@"
        ;;
    restore)
        check_repo
        shift
        restore_file "$@"
        ;;
    help)
        usage
        ;;
    *)
        echo "Error: Unknown subcommand '${1:-}'."
        exit 1
        ;;
esac
