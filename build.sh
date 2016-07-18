#!/bin/bash

[ -z "$1" ] && exit 2

for script in $1/build/*sh; do
	if [ -f "$script" ]; then
		bash "$script" "$1"
	fi
done 