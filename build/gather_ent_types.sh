#!/bin/bash


entries=$(find ./include/entities/ -type f -printf "#include <entities/%f>\n" -exec grep EntityType {} +)

includes=$(grep "#" <<<"$entries")
types=$(grep -v "#"<<<"$entries" | sed 's/.*EntityType\s\(.*\);/\t\&\1,/')

printf "\
#ifndef ALL_ENT_TYPES_H
#define ALL_ENT_TYPES_H

%s

#include <stdlib.h>

const EntityType *ALL_ENTITY_TYPES[] = {
%s
};
const size_t ALL_ENTITY_TYPES_COUNT = sizeof(ALL_ENTITY_TYPES) / sizeof (*ALL_ENTITY_TYPES);

#endif //ALL_ENT_TYPES_H
" "$includes" "$types" > ./include/entity_types.gen.h
