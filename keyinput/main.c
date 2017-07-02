#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios;

void reset_terminal_mode()
{
	tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
	struct termios new_termios;

	tcgetattr(0, &orig_termios);
	new_termios = orig_termios;

	atexit(reset_terminal_mode);
	cfmakeraw(&new_termios);
	tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit_emulate()
{
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(0, &fds);

	return select(1, &fds, NULL, NULL, NULL);
}

int getch_emulate()
{
	char c;
	read(0, &c, sizeof (char));
	return c;
}


int main()
{
	char c;
	set_conio_terminal_mode();
	while (kbhit_emulate()) {
		c = getch_emulate();
		switch (c) {
		case 65:
			printf("UP\n");
			break;
		case 66:
			printf("DOWN\n");
			break;
		case 67:
			printf("RIGH\n");
			break;
		case 68:
			printf("LEFT\n");
			break;
		case 'q':
			return 0;
		}


		printf("%c\n", c);
	}
	return 0;
}
