#include <kos_client.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

#include <delay.h>
#include <server.h>
#include <hash.h>
#include <buffer.h>

buffer_vec *buffer;
pthread_mutex_t mutex_cliente;
sem_t sem_cg;
sem_t sem_sg;

int cliptr=0;


int kos_init(int num_server_threads, int buf_size, int num_shards) {
	buffer=init_buffer(buf_size);
	kos_init_server(num_server_threads, buf_size, num_shards);
	pthread_mutex_init(&mutex_cliente, NULL);	
	return 0;
}

char* kos_get(int clientid, int shardId, char* key) {

	char *rsp;
	
	buffer_vec ped=(buffer_unit*)malloc(sizeof(buffer_unit));				//aloca memoria para o pedido
	sem_init(&(ped->sem_avisa),0,0);										//inicializa o semaforo do pedido
	
	strcpy(ped->tipo_pedido, "get");										// preenche os campos do pedido
	ped->clientid=clientid;
	ped->shardid=shardId;
	ped->key=key;
	
	
	sem_wait(&sem_cg);
	pthread_mutex_lock(&mutex_cliente);
	
		buffer[cliptr]=ped;
		cliptr = (cliptr + 1) % n;
	
	pthread_mutex_unlock(&mutex_cliente);
	sem_post(&sem_sg);
	sem_wait(&(ped->sem_avisa));											//fica a espera que a resposta esteja pronta

	if(ped->response==NULL)
		rsp=NULL;
	else	
		rsp=strdup(ped->response);
	
	free(ped);
	
	return rsp;
}
 
char* kos_put(int clientid, int shardId, char* key, char* value) {

	char *rsp;

	buffer_vec ped=(buffer_unit*)malloc(sizeof(buffer_unit));
	sem_init(&(ped->sem_avisa),0,0);
	
	strcpy(ped->tipo_pedido, "put");
	ped->clientid=clientid;
	ped->shardid=shardId;
	ped->key=key;
	ped->value=value;

	sem_wait(&sem_cg);
	pthread_mutex_lock(&mutex_cliente);
	
		buffer[cliptr]=ped;		
		cliptr = (cliptr + 1) % n;	
	
	pthread_mutex_unlock(&mutex_cliente);
	sem_post(&sem_sg);
	
	sem_wait(&(ped->sem_avisa));

	if(ped->response==NULL)
		rsp=NULL;
	else	
		rsp=strdup(ped->response);
	free(ped);
	
	return rsp; 
}

char* kos_remove(int clientid, int shardId, char* key) {

	char *rsp;
	
	buffer_vec ped=(buffer_unit*)malloc(sizeof(buffer_unit));
	sem_init(&(ped->sem_avisa),0,0);
	
	strcpy(ped->tipo_pedido, "remove");
	ped->clientid=clientid;
	ped->shardid=shardId;
	ped->key=key;
	
	sem_wait(&sem_cg);
	pthread_mutex_lock(&mutex_cliente);
	
		buffer[cliptr]=ped;
		cliptr = (cliptr + 1) % n;
	
	pthread_mutex_unlock(&mutex_cliente);
	sem_post(&sem_sg);

	sem_wait(&(ped->sem_avisa));
	
	if(ped->response==NULL)
		rsp=NULL;
	else	
		rsp=strdup(ped->response);
	free(ped);
	
	return rsp;
}

KV_t* kos_getAllKeys(int clientid, int shardId, int* dim) {

	KV_t *rsp;
	int j;
	
	buffer_vec ped=(buffer_unit*)malloc(sizeof(buffer_unit));
	sem_init(&(ped->sem_avisa),0,0);
	
	strcpy(ped->tipo_pedido, "allkeys");
	ped->clientid=clientid;
	ped->shardid=shardId;
	ped->dimen=dim;

	sem_wait(&sem_cg);
	pthread_mutex_lock(&mutex_cliente);
	
		buffer[cliptr]=ped;
		cliptr = (cliptr + 1) % n;
	
	pthread_mutex_unlock(&mutex_cliente);
	sem_post(&sem_sg);

	sem_wait(&(ped->sem_avisa));

	rsp=(KV_t*)malloc(sizeof(KV_t)*(*dim));
	rsp=ped->response_keys;
	
	for(j=0; j<(*dim); j++){												//recria a tablela response_keys fora da estrurura ped para que
				strcpy(rsp[j].key, ped->response_keys[j].key);				//possamos libertar a memoria da estrutura
				strcpy(rsp[j].value, ped->response_keys[j].value);
			}
		
	free(ped);
	
	return rsp;	
}
