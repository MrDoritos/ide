#include "console.h"
#include <ncurses.h>

int main() {
	raw();
	keypad(stdscr, 1);
	int key = 0;
	int p = 0;
	while ((key = console::readKey()) != VK_ESCAPE) {
		char buf[127];
		snprintf(&buf[0], 127, "You pressed %i:%c\r\n", key, key);
		console::write(0, p++, &buf[0]);
	}
}
