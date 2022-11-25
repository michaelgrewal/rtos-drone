#ifndef PROPELLER_H_
#define PROPELLER_H_

#define NUM_PROPS						4
#define RATE							500
#define PROP_WAIT						1
#define WIND_INTERVAL_SEC				10
#define ADJUST_SPEED_INTERVAL_MICROSEC	250000

const char *shmem_props[] = {"prop1", "prop2", "prop3", "prop4"};

struct thread_args {
	void *ptr;
	int coid;
};

#endif /* PROPELLER_H_ */
