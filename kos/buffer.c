#include <buffer.h>
#include <stdlib.h>
#include <stdio.h>

buffer_vec *buffer;

int buffer_pos;
int buffer_size;
int i;
int n;
buffer_vec *init_buffer(int buf_size){
	buffer_vec *buffer;	
	buffer=(buffer_vec*)malloc(sizeof(buffer_vec)*buf_size);
	
	sem_init(&sem_cg, 0, buf_size);
	sem_init(&sem_sg, 0,0);


	n = buf_size;

return buffer;
}


