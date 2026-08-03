#!/usr/bin/env bash
# Render every parametric part to STL. Re-run after editing params.scad.
# Requires OpenSCAD (headless STL export needs no display).
set -e
cd "$(dirname "$0")"
mkdir -p stl
for f in singulation_disc disc_housing hopper manifold_chute; do
  echo ">> $f"
  openscad -o "stl/$f.stl" "$f.scad"
done
echo "Done. STLs in cad/stl/. 'Simple: yes' in the log means a manifold (printable) part."
