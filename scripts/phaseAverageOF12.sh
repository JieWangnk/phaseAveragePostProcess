#!/bin/bash
# ============================================================================
# Phase-averaging workflow for OpenFOAM 12
# Replaces the three OF4 scripts (mkdirOnly, fullCycle, runOn) with a
# single script that uses phaseAveragePostProcess for offline phase averaging.
#
# Usage:
#   cd /path/to/parent/case   (where the time directories live)
#   bash /path/to/phaseAverageOF12.sh
#
# Prerequisites:
#   - OpenFOAM 12 environment sourced
#   - phaseAveragePostProcess compiled and on PATH
#   - No extra libraries needed (the utility does its own accumulation)
#   - Parent case has time directories spanning multiple cardiac cycles
#
# What this does:
#   For each phase point (0.00, 0.01, ..., endT):
#     1. Creates a phase directory
#     2. Symlinks system/, constant/ from parent
#     3. Symlinks the time dir from each cycle at that phase
#     4. Runs phaseAveragePostProcess to compute the arithmetic
#        ensemble mean (sum/N) of fields across cycles
# ============================================================================

set -e

# ======================== USER PARAMETERS ========================
endT=0.809          # last phase point within a cycle (T - dt_write)
ncycles_start=1     # first cycle to include (0-indexed; skip cycle 0 for transient)
ncycles=25          # total number of cycles available
T=0.81              # cardiac period (seconds)
dt_write=0.01       # snapshot write interval
# =================================================================

PARENT_DIR=$(pwd)

# No controlDict needed — phaseAveragePostProcess does its own
# arithmetic accumulation (sum/N) without fieldAverage.

echo "============================================="
echo "Phase-averaging workflow for OpenFOAM 12"
echo "Parent case: $PARENT_DIR"
echo "Period T = $T s, dt_write = $dt_write s"
echo "Cycles: $ncycles_start to $((ncycles-1))"
echo "============================================="

# Generate phase points
cycle1=($(seq 0.0 $dt_write $endT))
nphases=${#cycle1[@]}
echo "Number of phase points: $nphases"
echo ""

for i in "${cycle1[@]}"; do
    echo "--- Phase $i ---"

    # Create phase directory
    mkdir -p "$i"
    cd "$i"

    # Symlink system and constant from parent
    ln -sf ../system .  2>/dev/null || true
    ln -sf ../constant . 2>/dev/null || true

    # phaseAveragePostProcess reads fields directly — no controlDict needed

    # Symlink time directories from each cycle at this phase
    for j in $(seq $ncycles_start $((ncycles-1))); do
        t_actual=$(awk "BEGIN{printf \"%.6g\", $i + ($j * $T); exit}")
        if [ -d "../$t_actual" ]; then
            ln -sf ../$t_actual . 2>/dev/null || true
        else
            # Try with different precision
            t_actual=$(awk "BEGIN{printf \"%.4g\", $i + ($j * $T); exit}")
            if [ -d "../$t_actual" ]; then
                ln -sf ../$t_actual . 2>/dev/null || true
            fi
        fi
    done

    # Build comma-separated list of actual cycle times for -time flag
    # (avoids timeSelector scanning parent case through symlinks)
    time_list=""
    for j in $(seq $ncycles_start $((ncycles-1))); do
        t_actual=$(awk "BEGIN{printf \"%.6g\", $i + ($j * $T); exit}")
        if [ -d "$t_actual" ] || [ -L "$t_actual" ]; then
            if [ -z "$time_list" ]; then
                time_list="$t_actual"
            else
                time_list="$time_list,$t_actual"
            fi
        fi
    done

    n_linked=$(echo "$time_list" | tr ',' '\n' | wc -l)
    echo "  Linked $n_linked time directories"

    # Run post-processing with explicit time list
    if [ -n "$time_list" ]; then
        phaseAveragePostProcess -time "$time_list" > log.phaseAvg 2>&1
        echo "  Post-processing done"
    else
        echo "  WARNING: no time directories found, skipping"
    fi

    cd "$PARENT_DIR"
done

echo ""
echo "============================================="
echo "Phase averaging complete."
echo "Results in each phase directory (e.g. 0.32/):"
echo "  UMean, pMean (arithmetic mean of N cycle snapshots)"
echo "============================================="
