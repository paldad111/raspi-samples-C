#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>

#define IN1 (26) 
#define IN2 (27)
#define IN3 (28)
#define IN4 (29)

int phase[8][4] = {
	{0, 0, 0, 1}, /* Phase 1 */
	{0, 0, 1, 1}, /* Phase 2 */
	{0, 0, 1 ,0}, /* Phase 3 */
	{0, 1, 1, 0}, /* Phase 4 */
	{0, 1, 0, 0}, /* Phase 5 */
	{1, 1, 0, 0}, /* Phase 6 */
	{1, 0, 0, 0}, /* Phase 7 */
	{1, 0, 0, 1}  /* Phase 8 */
};


#define MAX_PHASE (8)
#define next_phase(p) ((p + 1) & (MAX_PHASE - 1))
#define prev_phase(p) ((p - 1) & (MAX_PHASE - 1))

void set_phase(int p)
{
	static int old = 0;

	if ((p != old) && (p != next_phase(old)) && (p != prev_phase(old))) {
		printf("cannot move %d -> %d\n", old, p);
		return;
	}

	/* update old phase */
	old = p;

	digitalWrite(IN1, phase[p][0] ? LOW : HIGH);
	digitalWrite(IN2, phase[p][1] ? LOW : HIGH);
	digitalWrite(IN3, phase[p][2] ? LOW : HIGH);
	digitalWrite(IN4, phase[p][3] ? LOW : HIGH);
	
	printf("%d %d %d %d\n", phase[p][3], phase[p][2], phase[p][1], phase[p][0]);
}

int main()
{
	int current_phase = 0;

	/* setup environment */
	if (wiringPiSetup() < 0) {
		printf("wiringPiSetup() failed\n");
		return 0;
	}

	pinMode(IN1, OUTPUT);
	digitalWrite(IN1, HIGH);

	sleep(5);
	digitalWrite(IN1, LOW);

	return 0;
}
