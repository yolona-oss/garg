#ifndef _GARG_TIME_H_
#define _GARG_TIME_H_

void difftimespec(struct timespec *res, struct timespec *a, struct timespec *b);
struct timespec now();
struct timespec time_remain(struct timespec *start, int interval_ms);
void wait_left(struct timespec wait);

#endif
