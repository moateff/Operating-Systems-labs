#include <pthread.h>

struct station {
	pthread_mutex_t train_lock;
	pthread_cond_t train_is_ready;
	pthread_cond_t train_arrival;
	int waiting_passengers;
	int empty_seats;
	int on_board_passengers;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);