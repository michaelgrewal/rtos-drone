#ifndef PROPELLER_H_
#define PROPELLER_H_

#define NUM_PROPS						4
#define RATE							500
#define PROP_WAIT						1
#define WIND_INTERVAL_SEC				10
#define ADJUST_SPEED_INTERVAL_MICROSEC	250000

const char *shmem_props[] = {"prop1", "prop2", "prop3", "prop4"};

typedef enum {
	FRONT_LEFT  = 0,
	FRONT_RIGHT = 1,
	BACK_LEFT   = 2,
	BACK_RIGHT  = 3
} propeller_index_t;

typedef struct {
	void *ptr;
	int coid;
	union {
		int8_t prop_code;
		struct {
			pthread_mutex_t *mutex;
			propeller_index_t prop_index;
		};
		
	};
} thread_args_t;

#endif /* PROPELLER_H_ */
