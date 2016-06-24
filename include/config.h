#ifndef _game_config_h
#define _game_config_h

#include <allegro5/allegro.h>

#include <stdbool.h>

/**
 * \brief Gets a double value from configuration.
 * @param section Configuration section, empty string for root.
 * @param key Key name within the given section.
 * @param fallback Default value to use if the value doesn't exist.
 * @return A double value, or fallback if it doesn't exist.
 */
long long config_get_ll(ALLEGRO_CONFIG *config, const char *section, const char *key, long long fallback);

/**
 * \brief Gets an integer from configuration.
 * @param section Configuration section, empty string for root.
 * @param key Key name within the given section.
 * @param fallback Default value to use if the value doesn't exist.
 * @return A integer value, or fallback if it doesn't exist.
 */
double config_get_double(ALLEGRO_CONFIG *config, const char *section, const char *key, double fallback);

/**
 * \brief Gets a string from configuration.
 * This string must be copied if it is going to be stored,
 * it becomes invalid when the configuration is reloaded.
 * @param section Configuration section, empty string for root.
 * @param key Key name within the given section.
 * @param fallback Default value to use if the value doesn't exist.
 * @return A string or fallback if it doesn't exist.
 */
const char *config_get_string(ALLEGRO_CONFIG *config, const char *section, const char *key, const char *fallback);

/**
 * \brief Iterates every entry of a section.
 * @param section Section to iterate.
 * @param iterator Function to process entries, return false to stop iteration early
 */
void config_each_entry(ALLEGRO_CONFIG *config, const char *section,
                       bool (*iterator)(ALLEGRO_CONFIG *config, const char *section, const char *key));

/**
 * \brief Iterates every entry of a section.
 * @param iterator Function to process sections, return false to stop iteration early
 */
void config_each_section(ALLEGRO_CONFIG *config, bool (*iterator)(ALLEGRO_CONFIG *config, const char *section));

#endif
