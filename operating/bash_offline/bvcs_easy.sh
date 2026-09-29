#!/usr/bin/bash

# ============================================
# BVCS — Super Simple Version Control
# ============================================
# This is an easy-to-read version of bvcs.sh.
# It works just like the original.
# ============================================

# --- FOLDER WHERE BVCS KEEPS ITS FILES ---
bvcs_folder=".bvcs"
objects_folder="$bvcs_folder/objects"
staging_file="$bvcs_folder/staging"
log_file="$bvcs_folder/log"
head_file="$bvcs_folder/HEAD"

# --- SHOW HELP ---
show_help() {
    cat <<'EOF'
Usage: bvcs <command> [arguments]

Commands:
    init                    Start a new BVCS repository
    add <file>...           Tell BVCS to track a file
    status                  See what's going on (staged, modified, untracked)
    commit -m <message>     Save a snapshot of your work
    log                     Look at all your past commits
    diff [file]             Compare current file with last saved version
    restore <file>          Get back an old version of a file
    help                    Show this help message
EOF
}

# --- CHECK IF WE'RE IN A BVCS REPO ---
check_repo() {
    if [ ! -d "$bvcs_folder" ]; then
        echo "Error: This is not a BVCS repository. Run 'init' first."
        exit 1
    fi
}

# --- INIT: START A NEW REPOSITORY ---
init_repo() {
    if [ -d "$bvcs_folder" ]; then
        echo "Error: A BVCS repository already exists here."
        exit 1
    fi
    mkdir -p "$objects_folder"
    > "$staging_file"
    > "$log_file"
    > "$head_file"
    echo "Initialized empty BVCS repository."
}

# --- ADD: STAGE FILES FOR COMMIT ---
add_files() {
    if [ $# -eq 0 ]; then
        echo "Error: Please specify at least one file to add."
        exit 1
    fi
    for file in "$@"; do
        if [ ! -f "$file" ]; then
            echo "Error: '$file' not found."
            continue
        fi
        if grep -qxF "$file" "$staging_file" 2>/dev/null; then
            echo "Already staged: $file"
            continue
        fi
        echo "$file" >> "$staging_file"
        echo "Staged: $file"
    done
}

# --- STATUS: SHOW WHAT'S HAPPENING ---
show_status() {
    local head_id
    head_id=$(cat "$head_file" 2>/dev/null)
    local temp_file="/tmp/bvcs_status.$$"
    local anything_printed=false

    # Staged files — files ready to be committed
    if [ -s "$staging_file" ]; then
        echo "Staged for commit:"
        cat "$staging_file"
        echo
        anything_printed=true
    fi

    # Modified files — changed since last commit but not staged
    if [ -n "$head_id" ] && [ -d "$objects_folder/$head_id/files" ]; then
        local snapshot_folder="$objects_folder/$head_id/files"
        find "$snapshot_folder" -type f 2>/dev/null > "$temp_file"
        local modified_list=""
        while IFS= read -r snapshot_path; do
            local relative_path="${snapshot_path#$snapshot_folder/}"
            if grep -qxF "$relative_path" "$staging_file" 2>/dev/null; then
                continue
            fi
            if [ ! -f "$relative_path" ] || ! diff -q "$snapshot_path" "$relative_path" >/dev/null 2>&1; then
                modified_list="$modified_list$relative_path"$'\n'
            fi
        done < "$temp_file"
        if [ -n "$modified_list" ]; then
            echo "Modified (not staged):"
            echo "$modified_list" | grep -v '^$' | sort
            echo
            anything_printed=true
        fi
    fi

    # Untracked files — new files BVCS doesn't know about
    find . -type f ! -path './.bvcs/*' 2>/dev/null | sed 's|^\./||' > "$temp_file"
    local untracked_list=""
    while IFS= read -r working_file; do
        if grep -qxF "$working_file" "$staging_file" 2>/dev/null; then
            continue
        fi
        if [ -n "$head_id" ] && [ -f "$objects_folder/$head_id/files/$working_file" ]; then
            continue
        fi
        untracked_list="$untracked_list$working_file"$'\n'
    done < "$temp_file"
    rm -f "$temp_file"
    if [ -n "$untracked_list" ]; then
        echo "Untracked files:"
        echo "$untracked_list" | grep -v '^$' | sort
        echo
        anything_printed=true
    fi

    if [ "$anything_printed" = false ]; then
        echo "Nothing to commit, working tree clean."
    fi
}

# --- COMMIT: SAVE A SNAPSHOT ---
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
        echo "Error: Commit message cannot be empty."
        exit 1
    fi
    if [ ! -s "$staging_file" ]; then
        echo "Error: Nothing to commit. Add some files first."
        exit 1
    fi

    local head_id
    head_id=$(cat "$head_file" 2>/dev/null)

    local log_line_count=0
    if [ -f "$log_file" ]; then
        log_line_count=$(wc -l < "$log_file")
    fi
    local new_commit_id
    new_commit_id=$(printf '%04d' $((log_line_count + 1)))

    local new_snapshot_folder="$objects_folder/$new_commit_id/files"
    mkdir -p "$new_snapshot_folder"

    # Copy files from previous commit (so we keep everything)
    if [ -n "$head_id" ] && [ -d "$objects_folder/$head_id/files" ]; then
        cp -r "$objects_folder/$head_id/files/." "$new_snapshot_folder/"
    fi

    # Copy staged files into the new snapshot
    local file_count=0
    while IFS= read -r staged_file; do
        destination="$new_snapshot_folder/$staged_file"
        mkdir -p "$(dirname "$destination")"
        if cp "$staged_file" "$destination"; then
            file_count=$((file_count + 1))
        else
            echo "Warning: could not copy '$staged_file' — skipping." >&2
        fi
    done < "$staging_file"

    # Save timestamp and message
    local timestamp
    timestamp=$(date +'%Y-%m-%d %H:%M:%S')
    echo "$timestamp" > "$objects_folder/$new_commit_id/timestamp"
    echo "$message" > "$objects_folder/$new_commit_id/message"

    # Update log and HEAD
    echo "$new_commit_id|$timestamp|$message" >> "$log_file"
    echo "$new_commit_id" > "$head_file"

    # Clear staging area
    > "$staging_file"

    echo "[$new_commit_id] $message"
    echo "$file_count file(s) committed."
}

# --- LOG: SHOW COMMIT HISTORY ---
show_log() {
    if [ ! -s "$log_file" ]; then
        echo "No commits yet."
        return
    fi
    tac "$log_file" | while IFS='|' read -r commit_id timestamp message; do
        echo "commit $commit_id"
        echo "Date:   $timestamp"
        echo "Message: $message"
        echo
    done
}

# --- DIFF: COMPARE CURRENT FILE WITH LAST COMMIT ---
show_diff() {
    local head_id
    head_id=$(cat "$head_file" 2>/dev/null)
    if [ -z "$head_id" ]; then
        echo "Error: No commits yet."
        exit 1
    fi
    local snapshot_folder="$objects_folder/$head_id/files"

    if [ $# -eq 0 ]; then
        # Show diff for all tracked files
        local temp_file="/tmp/bvcs_diff.$$"
        find "$snapshot_folder" -type f 2>/dev/null | sed "s|^$snapshot_folder/||" | sort > "$temp_file"
        while IFS= read -r relative_path; do
            local snapshot_path="$snapshot_folder/$relative_path"
            if [ -f "$relative_path" ]; then
                if diff -q "$snapshot_path" "$relative_path" >/dev/null 2>&1; then
                    echo "$relative_path: no changes."
                else
                    diff -u --label "$snapshot_path" --label "$relative_path" "$snapshot_path" "$relative_path"
                fi
            else
                diff -u --label "$snapshot_path" --label "$relative_path" "$snapshot_path" /dev/null
            fi
        done < "$temp_file"
        rm -f "$temp_file"
    else
        local file="$1"
        local snapshot_path="$snapshot_folder/$file"
        if [ ! -f "$snapshot_path" ]; then
            echo "Error: '$file' is not tracked."
            exit 1
        fi
        if [ -f "$file" ]; then
            if diff -q "$snapshot_path" "$file" >/dev/null 2>&1; then
                echo "$file: no changes."
            else
                diff -u --label "$snapshot_path" --label "$file" "$snapshot_path" "$file"
            fi
        else
            diff -u --label "$snapshot_path" --label "$file" "$snapshot_path" /dev/null
        fi
    fi
}

# --- RESTORE: GET BACK AN OLD VERSION OF A FILE ---
restore_file() {
    if [ $# -eq 0 ]; then
        echo "Error: No file specified."
        exit 1
    fi
    local file="$1"
    local head_id
    head_id=$(cat "$head_file" 2>/dev/null)
    if [ -z "$head_id" ]; then
        echo "Error: No commits yet."
        exit 1
    fi
    local snapshot_path="$objects_folder/$head_id/files/$file"
    if [ ! -f "$snapshot_path" ]; then
        echo "Error: '$file' not found in commit $head_id."
        exit 1
    fi
    read -p "Restore '$file' from commit $head_id? [y/N]: " answer
    if [ "$answer" != "y" ] && [ "$answer" != "Y" ]; then
        echo "Aborted."
        exit 0
    fi
    mkdir -p "$(dirname "$file")"
    cp "$snapshot_path" "$file"
    echo "Restored: $file"
}

# --- MAIN: DECIDE WHAT TO DO BASED ON THE FIRST ARGUMENT ---
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
        show_help
        ;;
    *)
        echo "Error: Unknown command '${1:-}'."
        echo "Run 'bvcs help' to see available commands."
        exit 1
        ;;
esac
