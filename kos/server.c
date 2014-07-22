#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <kos_client.h>
#include <server.h>
#include <buffer.h>
#include <delay.h>
#include <hash.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


pthread_t *servidores;
buffer_vec *buffer;

sem_t sem_sg;
sem_t *leitores;
sem_t *escritores;

pthread_mutex_t *mutex_kos; 
pthread_mutex_t mutex_servidor;

hashtable **kos_shards=NULL;

int n_server_threads;
int n_shards;
int srvptr = 0;

int *e_espera;
int *l_espera;
int *n_leitores;

bool *em_escrita;

char* puts_server_init(int shardId, char* key, char* value) {
  int id;
	char *value_old;	
	node *newpair = NULL;
	node *next_lista = NULL;
	
	id=hash(key);
	next_lista = kos_shards[shardId]->table[id];
	
	while(next_lista!=NULL){											//verifica se já existe um par com a key pretendida
			if(strcmp(key, next_lista->pair.key)==0){
				value_old=strdup(next_lista->pair.value);
				strcpy(next_lista->pair.value, value);	
				return value_old;
			}												//caso exista, retorna o par dessa key 
			next_lista = next_lista->next;
		}

	newpair=malloc(sizeof(node));											//caso não exista, cria um novo par 
	strcpy(newpair->pair.key, key);
	strcpy(newpair->pair.value, value);
	newpair->next=kos_shards[shardId]->table[id];
	kos_shards[shardId]->table[id]=newpair;

	return NULL;
}

int kos_init_server(int num_server_threads, int buf_size, int num_shards) {
	int* id=(int*) malloc(sizeof(int)*num_server_threads);	
	int i;
	int j;
	int fd;
	char str[KV_SIZE];
	char buffer_key[KV_SIZE];
	char buffer_value[KV_SIZE];
			
	n_shards=num_shards;
	n_server_threads=num_server_threads;
	
	leitores=malloc(sizeof(sem_t)*num_shards*HT_SIZE);	
	for(i=0; i<num_shards*HT_SIZE; i++)
		sem_init(&leitores[i], 0,0);

	escritores=malloc(sizeof(sem_t)*num_shards*HT_SIZE);	
	for(i=0; i<num_shards*HT_SIZE; i++)
		sem_init(&escritores[i], 0,0);

	e_espera=malloc(sizeof(int)*num_shards*HT_SIZE);
	for(i=0; i<num_shards*HT_SIZE; i++)
		e_espera[i]=0;
	
	l_espera=malloc(sizeof(int)*num_shards*HT_SIZE);	
	for(i=0; i<num_shards*HT_SIZE; i++)
		l_espera[i]=0;

	n_leitores=malloc(sizeof(int)*num_shards*HT_SIZE);	
	em_escrita=malloc(sizeof(bool)*num_shards*HT_SIZE);
	

	mutex_kos=malloc(sizeof(pthread_mutex_t)*num_shards*HT_SIZE);
	for(i=0; i<num_shards*HT_SIZE; i++)
		pthread_mutex_init(&mutex_kos[i], NULL);

	pthread_mutex_init(&mutex_servidor, NULL);
	
	kos_shards=malloc(sizeof(hashtable*)*num_shards);						 // aloca memoria para o kos 
	for(i=0;i<num_shards;i++){		
		kos_shards[i]= malloc(sizeof(hashtable));
		for(j=0; j<HT_SIZE; j++)
			kos_shards[i]->table[j] = NULL;
	}
	
	
	for(i=0; i<num_shards; i++){
		sprintf(str, "f%d.txt",i);
	  	fd = open(str, O_CREAT | O_TRUNC | O_RDONLY, S_IRUSR | S_IWUSR);
  
		while(read(fd, buffer_key, KV_SIZE)>0){
			read(fd, buffer_value, KV_SIZE);
			if(strcmp(buffer_key,"")){
				puts_server_init(i, buffer_key, buffer_value);
				printf("%s %s\n", buffer_key, buffer_value);
			}			
		}
		close(fd);	
	}

	servidores=(pthread_t*)malloc(sizeof(pthread_t)*num_server_threads);	// array de threads do servidor
	for(i=0;i<num_server_threads;i++){
		id[i]=i;		
		pthread_create(&(servidores[i]),NULL,servidor_thread,&(id[i]));
	}

	return 0;
}

void inicia_leitura (int pos) {
	pthread_mutex_lock(&mutex_kos[pos]);
		if(em_escrita[pos] || e_espera[pos] > 0){
			l_espera[pos]++;
			pthread_mutex_unlock(&mutex_kos[pos]);
			sem_wait(&leitores[pos]);
			pthread_mutex_lock(&mutex_kos[pos]); 
			if(l_espera[pos]>0){
				n_leitores[pos]++;
				l_espera[pos]--;
				sem_post(&leitores[pos]);
			}
		}
		else n_leitores[pos]++;
	pthread_mutex_unlock(&mutex_kos[pos]);
}
			
void termina_leitura (int pos) {
	pthread_mutex_lock(&mutex_kos[pos]);
	n_leitores[pos]--;
		if(n_leitores[pos] == 0 && e_espera[pos] > 0){
			sem_post(&escritores[pos]);
			em_escrita[pos] = true;
			e_espera[pos]--;
		}
	pthread_mutex_unlock(&mutex_kos[pos]);	
}

void inicia_escrita (int pos) {
	pthread_mutex_lock(&mutex_kos[pos]);
	if(em_escrita[pos] || n_leitores[pos] > 0){
			e_espera[pos]++;
			pthread_mutex_unlock(&mutex_kos[pos]);
			sem_wait(&escritores[pos]);
			pthread_mutex_lock(&mutex_kos[pos]);
		}
		em_escrita[pos] = true;
	pthread_mutex_unlock(&mutex_kos[pos]);
}

void termina_escrita (int pos) {
	pthread_mutex_lock(&mutex_kos[pos]);
	em_escrita[pos] = false;
	if(l_espera[pos] > 0){
		sem_post(&leitores[pos]);
		n_leitores[pos]++;
		l_espera[pos]--;
	}
	else if(e_espera[pos] > 0){
		sem_post(&escritores[pos]);
		em_escrita[pos] = true;
		e_espera[pos]--;
	}
	pthread_mutex_unlock(&mutex_kos[pos]);
}


char* gets_server(int shardId, char* key) {
  int id;
	node *pairs=NULL;
		
	id=hash(key);
	pairs= kos_shards[shardId]->table[id];
	
	inicia_leitura(shardId*HT_SIZE+id);	
	while(pairs!=NULL){
		if(strcmp(key, pairs->pair.key)==0){
			termina_leitura(shardId*HT_SIZE+id);
			return pairs->pair.value;
			}		
		pairs = pairs->next;
	}
	
	termina_leitura(shardId*HT_SIZE+id);
	return NULL;
}
 
char* puts_server(int shardId, char* key, char* value) {
  int id;
	char *value_old;	
	node *newpair = NULL;
	node *next_lista = NULL;
	int fd;
	char str[KV_SIZE];
		
	id=hash(key);
	next_lista = kos_shards[shardId]->table[id];
	
	inicia_escrita(shardId*HT_SIZE+id);
	
	while(next_lista!=NULL){											//verifica se já existe um par com a key pretendida
		if(strcmp(key, next_lista->pair.key)==0){
			value_old=strdup(next_lista->pair.value);
			strcpy(next_lista->pair.value, value);

			sprintf(str, "f%d.txt", shardId);
			fd = open(str, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR);

			while(read(fd, str, KV_SIZE) > 0){
				if(strcmp(str, key) == 0)
					write(fd,value,strlen(value)+1);
			}
			termina_escrita(shardId*HT_SIZE+id);
			return value_old;										//caso exista, retorna o par dessa key
		} 			
		next_lista = next_lista->next;
	}

	newpair=malloc(sizeof(node));											//caso não exista, cria um novo par 
	strcpy(newpair->pair.key, key);
	strcpy(newpair->pair.value, value);
	newpair->next=kos_shards[shardId]->table[id];
	kos_shards[shardId]->table[id]=newpair;

	sprintf(str, "f%d.txt", shardId);
	fd = open(str, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR);
   	
   	lseek(fd, 0, SEEK_END);
	write(fd, key, strlen(key)+1);
	write(fd, value, strlen(value)+1);

	termina_escrita(shardId*HT_SIZE+id);
	return NULL;
}
	
char *removes_server(int shardId, char* key) {
	int id = hash(key);
	char *value_found=(char*)malloc(sizeof(char)*KV_SIZE); 
	char str[KV_SIZE]; 
	int fd;

	node *previous_node = NULL; 
	node *next_node=kos_shards[shardId]->table[id];
	
	inicia_escrita(shardId*HT_SIZE+id);	
	while(next_node!=NULL) {												//verifica se o par que queremos remover, existe
	 if(strcmp(key, next_node->pair.key)==0){
			value_found=strdup(next_node->pair.value);
			
			if(next_node==kos_shards[shardId]->table[id])
				kos_shards[shardId]->table[id]=next_node->next;
			else
				previous_node->next=next_node->next;
			
			free(next_node);

			sprintf(str, "f%d.txt", shardId);
			fd = open(str, O_CREAT | O_RDWR , S_IRUSR | S_IWUSR);

			while(read(fd, str, KV_SIZE) > 0){
				if(strcmp(str, key) == 0){
					lseek(fd, -KV_SIZE, SEEK_CUR);
					write(fd,"                                        ",KV_SIZE*2);
				}
			}
			termina_escrita(shardId*HT_SIZE+id);
			return value_found;												//caso exista, retorna o valor associado a essa chave
		}
		previous_node=next_node;
		next_node=next_node->next;
	}

	termina_escrita(shardId*HT_SIZE+id);	
		
	return NULL;
}

KV_t* get_all_keys(int clientid, int shardId, int *dim) {
	KV_t *all_pairs, *all_pairs_aux;
	hashtable *shard=kos_shards[shardId];	
	int i, dimen=0;
	node *act=NULL;

	if((shardId<0)||(shardId>(n_shards-1))||(clientid>(n_server_threads-1))||(clientid<0)){
		*dim=-1;
		return NULL;
	}	
	
	for(i=0; i<HT_SIZE; i++){												//calcula o numero de pares presentes na shard desejada
		inicia_leitura(shardId*HT_SIZE+i);		
		act=shard->table[i];
		while(act!=NULL){
				dimen++;
				act=act->next;
			}
		termina_leitura(shardId*HT_SIZE+i);		
		}

		all_pairs_aux=(KV_t*)malloc(sizeof(KV_t)*dimen);					//aloca memoria suficiente para todos os pares
		all_pairs = all_pairs_aux;
			
	for(i=0; i<HT_SIZE; i++){												// preenche a lista com todos os pares existentes
		inicia_leitura(shardId*HT_SIZE+i);		
		act=shard->table[i];
		while(act!=NULL){
				strcpy(all_pairs_aux->key, act->pair.key);
				strcpy(all_pairs_aux->value, act->pair.value);
				act=act->next;
			}
						
	termina_leitura(shardId*HT_SIZE+i);		
	}

	*dim=dimen;
	return all_pairs;
}

void *servidor_thread(void *ptr){
	int clientid, shardid, *dimen;
	char *key, *value;
	int teste = 0;
	char *response;
	char pedido[KV_SIZE];
	buffer_vec ped=(buffer_unit*)malloc(sizeof(buffer_unit));
	
	while(1) {
		sem_wait(&sem_sg); //wait servidor
		pthread_mutex_lock(&mutex_servidor);
		
		ped=buffer[srvptr];
		strcpy(pedido, ped->tipo_pedido);
 		key = ped->key;	
		value=ped->value;
		clientid = ped->clientid;
		shardid = ped->shardid;
		dimen = ped->dimen;
	
		srvptr = (srvptr + 1) % n;
		pthread_mutex_unlock(&mutex_servidor);
		sem_post(&sem_cg);

		if(strcmp("get",pedido)==0){
			response=gets_server(shardid, key);
			teste = 1;
		}

		if(strcmp("put",pedido)==0){
			response=puts_server(shardid, key, value);
			teste = 1;
		}
		
		if(strcmp("remove",pedido)==0){
			response=removes_server(shardid, key);
			teste = 1;
		}

		if(teste == 1){
			if(response==NULL){
				ped->response=NULL;
				teste = 0;
			}
			else{
				ped->response=strdup(response);
				teste = 0;
			}
		}

		if(strcmp("allkeys",pedido)==0){
			ped->response_keys=get_all_keys(clientid, shardid, dimen);
		}
	
		sem_post(&(ped->sem_avisa)); //post cliente
		delay();	
	}		
	return NULL;
}

