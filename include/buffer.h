#ifndef BUFFER_H
#define BUFFER_H 1
#include <semaphore.h>
#include <kos_client.h>

#define HT_SIZE 10
#define KV_SIZE 20

typedef struct{		/* struct of clients requests */
	char tipo_pedido[KV_SIZE];
	int clientid;
	int shardid;
	char *key;
	char *value;

	char *response;
	KV_t *response_keys;
	int *dimen;

	sem_t sem_avisa;

}buffer_unit;

typedef buffer_unit *buffer_vec; 


int n;
extern sem_t sem_cg,sem_sg;

extern buffer_vec *buffer;

buffer_vec *init_buffer(int buf_size);

#endif 
