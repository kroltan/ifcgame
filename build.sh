#!/bin/bash

[ -z "$1" ] && exit 2
[ -z "$2" ] && exit 2

export PROJECT_DIR="$1"
export OUT_DIR="$2"

for script in $1/build/*sh; do
	if [ -f "$script" ]; then
		bash "$script"
	fi
done 