#ifndef CONSOLE_H
#define CONSOLE_H

#include <allegro5/allegro.h>

extern const unsigned int CONSOLE_EVENT_ID;

void init_console();
bool console_on_event(ALLEGRO_EVENT *event);
void console_draw();

#endif // CONSOLE_H
