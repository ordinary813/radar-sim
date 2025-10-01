#!/bin/bash
set -e

DIR="build/"

if [ -d "$DIR" ]; then
    rm -rf "$DIR"
fi

mkdir "$DIR"

echo "Building..."

cd "$DIR" && cmake .. && make

echo "Running simulation..."
./radar_sim

echo "Launching visualizer..."

if [[ "$1" == "--save" ]]; then
    python3 ../scripts/visualize.py --save
else
    python3 ../scripts/visualize.py
fi
