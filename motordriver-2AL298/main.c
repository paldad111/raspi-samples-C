#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <signal.h>
#include <termios.h>

#define IN1 (26) 
#define IN2 (27)
#define IN3 (28)
#define IN4 (29)

#define LEFT  (1)
#define RIGHT (2)
#define BOTH  (LEFT | RIGHT)

/**********************************************/
/*            termios functions               */
/**********************************************/
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


/**********************************************/
/*            system  functions               */
/**********************************************/
typedef void (*sighandler_t)(int);
sighandler_t orig_sigint_handler;
void sigint_handler(int signo);

static int system_init()
{
	int ret;
	if ((ret = wiringPiSetup() < 0))
		return ret;

	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);

	/* 2, CTRL+C */
	orig_sigint_handler = signal(SIGINT, sigint_handler);

	return 0;
}

static void system_done()
{
	digitalWrite(IN1, LOW);
	digitalWrite(IN2, LOW);
	digitalWrite(IN3, LOW);
	digitalWrite(IN4, LOW);
}

void sigint_handler(int signo)
{
	system_done();
	exit(0);
}

/**********************************************/
/*            control functions               */
/**********************************************/
static void forward();
static void __forward(int direction, int speed);
static void backword();
static void __backward(int direction, int speed);

static void forward()
{
	__forward(BOTH, 0);
}

static void __forward(int direction, int speed)
{
	if (direction & LEFT) {
		digitalWrite(IN3, HIGH);
		digitalWrite(IN4, LOW);
	} else {
		digitalWrite(IN3, LOW);
		digitalWrite(IN4, LOW);
	}

	if (direction & RIGHT) {
		digitalWrite(IN1, HIGH);
		digitalWrite(IN2, LOW);
	} else {
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, LOW);
	}
}

static void backward()
{
	__backward(BOTH, 0);
}

static void __backward(int direction, int speed)
{
	if (direction & LEFT) {
		digitalWrite(IN4, HIGH);
		digitalWrite(IN3, LOW);
	} else {
		digitalWrite(IN3, LOW);
		digitalWrite(IN4, LOW);
	}

	if (direction & RIGHT) {
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, HIGH);
	} else {
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, LOW);
	}
}


/**********************************************/
/*               main functions               */
/**********************************************/
int main()
{
	/* setup environment */
	if (system_init() < 0) {
		printf("wiringPiSetup() failed\n");
		return 0;
	}

	forward();
	sleep(1);
	backward();
	sleep(1);
	__forward(RIGHT, 0);
	sleep(1);
	__forward(LEFT, 0);
	sleep(1);
	__backward(RIGHT, 0);
	sleep(1);
	__backward(LEFT, 0);
	sleep(1);

#if 0
	while (kbhit_emulate()) {
		c = getch_emulate();

		switch (c) {
		case 65:
			forward();
			break;
		case 66:
			backward();
			break;
		case 67:
			__forward(RIGHT, 0);
			break;
		case 68:
			__forward(LEFT, 0);
			break;
		case 'q':
			goto done;
		}

	}
done:
#endif
	system_done();

	return 0;
}
