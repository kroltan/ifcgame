#include "config.h"

#include <stdlib.h>
#include <stdio.h>


long long config_get_ll(ALLEGRO_CONFIG *config, const char *section, const char *key, long long fallback) {
	const char *value = al_get_config_value(config, section, key);
	if (value) {
		return strtoll(al_get_config_value(config, section, key), NULL, 10);
	}
	return fallback;
}

double config_get_double(ALLEGRO_CONFIG *config, const char *section, const char *key, double fallback) {
	const char *value = al_get_config_value(config, section, key);
	if (value) {
		return strtod(al_get_config_value(config, section, key), NULL);
	}
	return fallback;
}

const char *config_get_string(ALLEGRO_CONFIG *config, const char *section, const char *key, const char *fallback) {
	const char *value = al_get_config_value(config, section, key);
	if (value) {
		return al_get_config_value(config, section, key);
	}
	return fallback;
}

void config_each_entry(ALLEGRO_CONFIG *config, const char *section,
                       bool (*iterator)(ALLEGRO_CONFIG *config, const char *section, const char *key)) {
    ALLEGRO_CONFIG_ENTRY *entry;
    for (const char *key = al_get_first_config_entry(config, section, &entry); key; key = al_get_next_config_entry(&entry)) {
        iterator(config, section, key);
    }
}

void config_each_section(ALLEGRO_CONFIG *config, bool (*iterator)(ALLEGRO_CONFIG *config, const char *section)) {
    ALLEGRO_CONFIG_SECTION *section;
    for (const char *name = al_get_first_config_section(config, &section); name; name = al_get_next_config_section(&section)) {
        iterator(config, name);
    }
}
