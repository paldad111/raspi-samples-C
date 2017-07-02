#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>

#define INA (28) /* Wind back */
#define INB (29) /* Wind Front */

static void forward(int p)
{
	softPwmWrite(INA, p);
	softPwmWrite(INB, 0);
}

static void backward(int p)
{
	softPwmWrite(INA, 0);
	softPwmWrite(INB, p);
}

int main()
{
	int pow, direction, mag;

	if (wiringPiSetup() < 0) {
		printf("wiringPiSetup() failed\n");
		return 0;
	}

	pinMode(INA, OUTPUT);
	pinMode(INB, OUTPUT);

	/* Inital value to low */
	digitalWrite(INA, LOW);
	digitalWrite(INB, LOW);

	softPwmCreate(INA, 0, 200);
	softPwmCreate(INB, 0, 200);

	
	/* initialize value */
	direction = 1;
	mag = 20;
	pow = 0;

	/* loop */
	while (1) {

		/* change direction */
		if (pow >= 200 || pow <= -200)
			direction = -direction;

		/* get a next pow */
		pow = pow + direction * mag;
		printf("%d\n", pow);

		if (pow > 0)
			forward(pow);
		else
			backward(-pow);

		sleep(1);
	}

	return 0;
	
}
