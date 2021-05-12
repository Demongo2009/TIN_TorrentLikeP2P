//
// Created by demongo on 11.05.2021.
//

#ifndef TIN_TORRENTLIKEP2P_THREAD_H
#define TIN_TORRENTLIKEP2P_THREAD_H

#include <pthread.h>

typedef struct thread_info {    /* Used as argument to thread_start() */
	pthread_t thread_id;        /* ID returned by pthread_create() */
	int       thread_num;       /* Application-defined thread # */
	char     *string;
	void *context;
	void *parentObj;
}thread_info;

#endif //TIN_TORRENTLIKEP2P_THREAD_H
