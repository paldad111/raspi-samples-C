#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <softPwm.h>

#define IN1 (26) 
#define IN2 (27)
#define IN3 (28)
#define IN4 (29)

/* Move forward */
int fphase[8][4] = {
	{0, 0, 0, 1}, /* Phase 1 */
	{0, 0, 1, 1}, /* Phase 2 */
	{0, 0, 1 ,0}, /* Phase 3 */
	{0, 1, 1, 0}, /* Phase 4 */
	{0, 1, 0, 0}, /* Phase 5 */
	{1, 1, 0, 0}, /* Phase 6 */
	{1, 0, 0, 0}, /* Phase 7 */
	{1, 0, 0, 1}  /* Phase 8 */
};
/* Move backward */
int rphase[8][4] = {
	{1, 0, 0, 0}, /* Phase 7 */
	{1, 1, 0, 0}, /* Phase 6 */
	{0, 1, 0, 0}, /* Phase 5 */
	{0, 1, 1, 0}, /* Phase 4 */
	{0, 0, 1 ,0}, /* Phase 3 */
	{0, 0, 1, 1}, /* Phase 2 */
	{0, 0, 0, 1}, /* Phase 1 */
	{1, 0, 0, 1}  /* Phase 8 */
};
#define MAX_PHASE (8)
#define next_phase(p) ((p + 1) & (MAX_PHASE - 1))
#define prev_phase(p) ((p - 1) & (MAX_PHASE - 1))

void set_phase(int p, int (*phase)[4])
{
	static int old = 0;

	if ((p != old) && (p != next_phase(old)) && (p != prev_phase(old))) {
		printf("cannot move %d -> %d\n", old, p);
		return;
	}

	/* update old phase */
	old = p;

	digitalWrite(IN1, phase[p][0] ? HIGH : LOW);
	digitalWrite(IN2, phase[p][1] ? HIGH : LOW);
	digitalWrite(IN3, phase[p][2] ? HIGH : LOW);
	digitalWrite(IN4, phase[p][3] ? HIGH : LOW);
}

void flush_remain_input(void)
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
}

unsigned long long degree_to_count(int degree)
{
	unsigned long long fixed32;

	fixed32  = (((unsigned long long)degree * 1000) << 32) * 8;
	fixed32 /= 5625;

	return (fixed32 >> 32);
}

int main()
{
	int i, c;
	unsigned long long loop_count;

	/* setup environment */
	if (wiringPiSetup() < 0) {
		printf("wiringPiSetup() failed\n");
		return 0;
	}
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);

	while (1) {
		int (*phase)[4];
		int nr_scan, degree;

		nr_scan = scanf("%d", &degree);
		if (nr_scan)
			goto parse_done;

		scanf("%*[^0-9]%d", &degree);
parse_done:
		flush_remain_input();

		if (degree > 0) {
			phase = fphase;
		} else {
			degree = -degree;
			phase = rphase;
		}

		loop_count = degree_to_count(degree);

		printf("degree %d -> count %lld\n", degree, loop_count);

		for (i = 0; i < loop_count; i++) {
			/* 0 1 2 3 4 5 6 7 */
			for (c = 0; c < MAX_PHASE; c++) {
				set_phase(c, phase);
				usleep(1000);
			}
		}
	}
	return 0;
}
