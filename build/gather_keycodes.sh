#!/bin/bash

input_content="
#include <allegro5/keycodes.h>
"

# register all Allegro keycodes for config usage
keycodes=$(cpp <<<"$input_content" |		# get contents of allegro keycode defines
	grep "ALLEGRO_KEY_.*=" |				# find only lines that define keycodes
	cut -d' ' -f4,6 |						# get the constant name and numeric parts
	cut -d'_' -f3- |						# strip the ALLEGRO_KEY_ prefix
	sed 's/\(.*\) \(.*\),/\t{"\1", \2},/' |	# format it as a struct initializer
	tr "[:upper:]" "[:lower:]"				# convert names to lowercase
)

printf "\
#ifndef KEYMAP_CODES_GEN_H
#define KEYMAP_CODES_GEN_H

#include \"hashmap.h\"

typedef struct {
	const char *name;
	int keycode;
} NamedKeyCode;

NamedKeyCode KEYCODE_NAMES[] = {
%s
};
const size_t KEYCODE_NAMES_COUNT = sizeof(KEYCODE_NAMES) / sizeof (*KEYCODE_NAMES);

#endif //KEYMAP_CODES_GEN_H
%s
" "$keycodes" > ./include/keymap_codes.gen.h