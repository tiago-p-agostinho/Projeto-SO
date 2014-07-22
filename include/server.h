#ifndef SERVER_H
#define SERVER_H 1

#include <kos_client.h>

#define HT_SIZE 10
#define KV_SIZE 20

typedef struct node {
	KV_t pair;
	struct node *next;
}node;
  
typedef struct hashtable {
	node *table[HT_SIZE];	
}hashtable;

extern int n_server_threads;
extern int n_shards;


void *servidor_thread(void *ptr);

int kos_init_server(int num_server_threads, int buf_size, int num_shards);

char* gets_server(int shardId, char* key);

char* puts_server(int shardId, char* key, char* value);

char* removes_server(int shardId, char* key);

KV_t* get_all_keys(int clientid, int shardId, int* dim);
#endif
