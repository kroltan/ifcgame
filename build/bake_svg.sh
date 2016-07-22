#!/bin/bash

find assets -name '*.svg' | while read infile; do
	outfile="$OUT_DIR$(dirname "$infile")/$(basename "$infile" .svg).png"
	mkdir -p "$(dirname "$outfile")"
	inkscape -z "$infile" -e "$outfile"
done
