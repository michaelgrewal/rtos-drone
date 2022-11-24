#ifndef PROPELLER_H_
#define PROPELLER_H_

#define NUM_PROPS	4
#define RATE		500

const char *shmem_props[] = {"prop1", "prop2", "prop3", "prop4"};

struct thread_args {
	void *ptr;
	int coid;
};

#endif /* PROPELLER_H_ */
