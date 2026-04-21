#!/bin/bash
# Compare GL vs Cycles brightness across a grid of BG% and Exposure% values.
# Usage: scripts/compare-grid.sh [scene_file] [rotate]
#
# Outputs a table of overall brightness ratios for each combination.

SCENE="${1:-ICLExperimental/Raytracing/scenes/DamagedHelmet.glb}"
ROTATE="${2:-180,0,180}"
BIN="build/bin/cycles-scene-viewer"
SIZE="120x90"
BG_VALUES="25 50 75 100"
EXP_VALUES="50 100 150 200"
OUTDIR="outputs/compare-grid"

mkdir -p "$OUTDIR"

echo "=== Brightness Comparison Grid ==="
echo "Scene: $SCENE  Rotate: $ROTATE  Size: $SIZE"
echo ""

# Header
printf "%8s" "BG\\Exp"
for exp in $EXP_VALUES; do
  printf " %8s" "E=${exp}%"
done
echo ""
echo "-----------------------------------------------"

for bg in $BG_VALUES; do
  printf "BG=%3d%%" "$bg"
  for exp in $EXP_VALUES; do
    PREFIX="${OUTDIR}/bg${bg}_exp${exp}"

    # Run compare mode with -bg and -exp overrides
    OUTPUT=$("$BIN" \
      -scene "$SCENE" \
      -rotate "$ROTATE" \
      -size "$SIZE" \
      -background white \
      -bg "$bg" \
      -exp "$exp" \
      -compare "$PREFIX" \
      2>&1)

    # Extract overall ratio
    RATIO=$(echo "$OUTPUT" | grep "^Overall:" | sed 's/.*ratio=\([0-9.]*\)/\1/')

    if [ -z "$RATIO" ]; then
      printf " %8s" "FAIL"
    else
      printf " %8s" "$RATIO"
    fi
  done
  echo ""
done

echo ""
echo "Ratio = Cycles/GL (1.00 = perfect match, >1 = GL too dark, <1 = GL too bright)"
echo "Images saved in $OUTDIR/"
