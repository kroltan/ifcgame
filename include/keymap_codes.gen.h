#ifndef KEYMAP_CODES_GEN_H
#define KEYMAP_CODES_GEN_H

#include "hashmap.h"

typedef struct {
	const char *name;
	int keycode;
} NamedKeyCode;

NamedKeyCode KEYCODE_NAMES[] = {
	{"a", 1},
	{"b", 2},
	{"c", 3},
	{"d", 4},
	{"e", 5},
	{"f", 6},
	{"g", 7},
	{"h", 8},
	{"i", 9},
	{"j", 10},
	{"k", 11},
	{"l", 12},
	{"m", 13},
	{"n", 14},
	{"o", 15},
	{"p", 16},
	{"q", 17},
	{"r", 18},
	{"s", 19},
	{"t", 20},
	{"u", 21},
	{"v", 22},
	{"w", 23},
	{"x", 24},
	{"y", 25},
	{"z", 26},
	{"0", 27},
	{"1", 28},
	{"2", 29},
	{"3", 30},
	{"4", 31},
	{"5", 32},
	{"6", 33},
	{"7", 34},
	{"8", 35},
	{"9", 36},
	{"pad_0", 37},
	{"pad_1", 38},
	{"pad_2", 39},
	{"pad_3", 40},
	{"pad_4", 41},
	{"pad_5", 42},
	{"pad_6", 43},
	{"pad_7", 44},
	{"pad_8", 45},
	{"pad_9", 46},
	{"f1", 47},
	{"f2", 48},
	{"f3", 49},
	{"f4", 50},
	{"f5", 51},
	{"f6", 52},
	{"f7", 53},
	{"f8", 54},
	{"f9", 55},
	{"f10", 56},
	{"f11", 57},
	{"f12", 58},
	{"escape", 59},
	{"tilde", 60},
	{"minus", 61},
	{"equals", 62},
	{"backspace", 63},
	{"tab", 64},
	{"openbrace", 65},
	{"closebrace", 66},
	{"enter", 67},
	{"semicolon", 68},
	{"quote", 69},
	{"backslash", 70},
	{"backslash2", 71},
	{"comma", 72},
	{"fullstop", 73},
	{"slash", 74},
	{"space", 75},
	{"insert", 76},
	{"delete", 77},
	{"home", 78},
	{"end", 79},
	{"pgup", 80},
	{"pgdn", 81},
	{"left", 82},
	{"right", 83},
	{"up", 84},
	{"down", 85},
	{"pad_slash", 86},
	{"pad_asterisk", 87},
	{"pad_minus", 88},
	{"pad_plus", 89},
	{"pad_delete", 90},
	{"pad_enter", 91},
	{"printscreen", 92},
	{"pause", 93},
	{"abnt_c1", 94},
	{"yen", 95},
	{"kana", 96},
	{"convert", 97},
	{"noconvert", 98},
	{"at", 99},
	{"circumflex", 100},
	{"colon2", 101},
	{"kanji", 102},
	{"pad_equals", 103},
	{"backquote", 104},
	{"semicolon2", 105},
	{"command", 106},
	{"back", 107},
	{"volume_up", 108},
	{"volume_down", 109},
	{"search", 110},
	{"dpad_center", 111},
	{"button_x", 112},
	{"button_y", 113},
	{"dpad_up", 114},
	{"dpad_down", 115},
	{"dpad_left", 116},
	{"dpad_right", 117},
	{"select", 118},
	{"start", 119},
	{"button_l1", 120},
	{"button_r1", 121},
	{"button_l2", 122},
	{"button_r2", 123},
	{"button_a", 124},
	{"button_b", 125},
	{"thumbl", 126},
	{"thumbr", 127},
	{"unknown", 128},
	{"modifiers", 215},
	{"lshift", 215},
	{"rshift", 216},
	{"lctrl", 217},
	{"rctrl", 218},
	{"alt", 219},
	{"altgr", 220},
	{"lwin", 221},
	{"rwin", 222},
	{"menu", 223},
	{"scrolllock", 224},
	{"numlock", 225},
	{"capslock", 226},
};
const size_t KEYCODE_NAMES_COUNT = sizeof(KEYCODE_NAMES) / sizeof (*KEYCODE_NAMES);

#endif //KEYMAP_CODES_GEN_H

