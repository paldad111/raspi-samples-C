#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define IN1 (26) /* R1 */
#define IN2 (27) /* R2 */
#define IN3 (28) /* L1 */
#define IN4 (29) /* L2 */

#define PWMA (4) /* RIGHT */
#define PWMB (5) /* LEFT  */

#define LEFT  (1)
#define RIGHT (2)
#define BOTH  (LEFT | RIGHT)

/**********************************************/
/*           control  functions               */
/**********************************************/
static void parse_control(char *buf, int *degree, int *strength)
{
	sscanf(buf, "%d|%d\n", degree, strength);

	printf("%d %d\n", *degree, *strength);
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

	softPwmCreate(PWMA, 0, 100);
	softPwmCreate(PWMB, 0, 100);

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
	softPwmWrite(PWMA, 0);
	softPwmWrite(PWMB, 0);
}

void sigint_handler(int signo)
{
	system_done();
	exit(0);
}

/**********************************************/
/*            control functions               */
/**********************************************/
static void forward(int speed);
static void __forward(int direction, int lspeed, int rspeed);
static void backward(int speed);
static void __backward(int direction, int lspeed, int rspeed);

static void forward(int speed)
{
	__forward(BOTH, speed, speed);
}

static void __forward(int direction, int lspeed, int rspeed)
{
	if (direction & LEFT) {
		digitalWrite(IN3, HIGH);
		digitalWrite(IN4, LOW);
		softPwmWrite(PWMB, lspeed); 
	} else {
		digitalWrite(IN3, LOW);
		digitalWrite(IN4, LOW);
		softPwmWrite(PWMB, 0); 
	}

	if (direction & RIGHT) {
		digitalWrite(IN1, HIGH);
		digitalWrite(IN2, LOW);
		softPwmWrite(PWMA, rspeed); 
	} else {
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, LOW);
		softPwmWrite(PWMA, 0); 
	}
}

static void backward(int speed)
{
	__backward(BOTH, speed, speed);
}

static void __backward(int direction, int lspeed, int rspeed)
{
	if (direction & LEFT) {
		digitalWrite(IN4, HIGH);
		digitalWrite(IN3, LOW);
		softPwmWrite(PWMB, lspeed); 

	} else {
		digitalWrite(IN3, LOW);
		digitalWrite(IN4, LOW);
		softPwmWrite(PWMB, 0); 
	}

	if (direction & RIGHT) {
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, HIGH);
		softPwmWrite(PWMA, rspeed); 
	} else {
		digitalWrite(IN1, LOW);
		digitalWrite(IN2, LOW);
		softPwmWrite(PWMA, 0); 
	}
}


/**********************************************/
/*               main functions               */
/**********************************************/
int main()
{
	int s_socket, rtn;
	struct sockaddr_in s_addr;


	/* setup environment */
	if (system_init() < 0) {
		printf("wiringPiSetup() failed\n");
		return 0;
	}

	s_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (s_socket < 0) {
		printf("socket create fail\n");
		goto sock_err;
	}

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(9999);

	rtn = bind(s_socket, (struct sockaddr*)&s_addr, sizeof(s_addr));
	if (rtn < 0) {
		printf("bind failed\n");
		goto bind_err;
	}

	rtn = listen(s_socket, 5);
	if (rtn < 0) {
		printf("listen failed\n");
		goto listen_err;
	}


	while (1) {
		char buf[100];
		int c_socket, len;
		socklen_t c_socklen;
		struct sockaddr_in c_addr;

		printf("Socket Listening..\n");

		c_socklen = sizeof(struct sockaddr_in);
		c_socket = accept(s_socket, (struct sockaddr*)&c_addr, &c_socklen);
		if (c_socket < 0) {
			printf("accept failed\n");
		}

		printf("Connection accept\n");

		while (1) {
			len = read(c_socket, buf, sizeof(buf));
			if (len < 0) {
				printf("SYSTEM: ERR: reading from socket\n");
				goto read_err;
			}
			if (len == 0) {
				printf("SYSTEM: reading done\n");
				goto read_done;
			}
			buf[len] = '\0';

			{
				int degree, strength;
				int go, lspeed, rspeed;


				parse_control(buf, &degree, &strength);

				go = 0;
				/* 4 area, backward, move RIGHT (LEFT > RIGHT) */
				if (degree > 240) {
					lspeed = strength;
					rspeed = strength * (360 - degree) / 90;
					goto move;
				}

				/* 3 area, backward, move left  (LEFT < RIGHT) */
				if (degree > 180) {
					lspeed = strength * (degree - 180) / 90;
					rspeed = strength;
					goto move;
				}

				go = 1;
				/* 2 area, forward, move left  (LEFT < RIGHT)*/
				if (degree > 90) {
					lspeed = strength * (180 - degree) / 90;
					rspeed = strength;
					goto move;
				}

				/* 1 area, forward, move right (LEFT > RIGHT) */
				{
					lspeed = strength;
					rspeed = strength * (degree) / 90;
				}
move:
				if (go)
					__forward(BOTH, lspeed, rspeed);
				else
					__backward(BOTH, lspeed, rspeed);
			}
		}
read_err:
read_done:
		close(c_socket);
	}

listen_err:
bind_err:
sock_err:
	close(s_socket);
	system_done();

	return 0;
}
