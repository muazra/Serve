/*
 * Name - Muaz Rahman
 * Email - muazra@gwmail.gwu.edu
 * GWU - CSCi 3411
 * Creation of a Ring Buffer
*/

#include <stdlib.h>
#include <stdio.h>
#define BUFFER_LENGTH 16

//Structure for ring buffer
struct ring{
	unsigned int head;
	unsigned int tail;
        unsigned int size;
	void **elems;
};

/*
*Allocates and returns a pointer to a ring buffer. 
*If unable to allocate the ring buffer,
*rb create returns NULL.
*/
struct ring* rb_create(void){
	struct ring *rb = (struct ring*) malloc(sizeof(struct ring));
	rb->head = 0;
	rb->tail = 0;
	rb->size = BUFFER_LENGTH+1;
	
	//allocating array of elements for ring buffer based on given size
	rb->elems = malloc(rb->size*sizeof(rb->elems));

	if (rb == NULL)
		return NULL;
	
	return rb;
}

/*
*De-allocates the contents of the ring 
*buffer and free's the memory space. 
*/
void rb_delete(struct ring *rb){
	if(rb != NULL){
		int i=0;
		while(rb->elems[i] != NULL){
			void *temp = rb->elems[i];
			free(temp);
			i++;
		}
		free(rb);
	}
}

/*
*Checks to see if ring buffer is empty. 
*If buffer not allocated, returns 0. 
*/
int rb_isempty(struct ring *rb){
	if(rb == NULL)
		return 0;

	//empty if head == tail
	if(rb->head == rb->tail)
		return -1;

	return 0;
}

/*
*Checks to see if ring buffer is full. 
*If buffer not allocated, returns 0. 
*/
int rb_isfull(struct ring *rb){
	if(rb == NULL)
		return 0;

	//full if head == tail
	if (((rb->tail+1) % rb->size) == rb->head)
		return -1;

	return 0;
}

/*
*Inserts a value at the (tail-1) of the ring buffer.
*Advances the tail, and returns -1 (logical true).
*/
int rb_enqueue(struct ring *rb, void *value){
	if(rb == NULL)
		return 0;

	//dont add or override if buffer is full - return 0. 
	if(rb_isfull(rb) == -1)
		return 0;

	/*add element to list based on the modulo with size
	*then increment tail
	*/
	rb->elems[rb->tail] = value;
	rb->tail = (rb->tail+1) % rb->size;

	return -1;
}

/*
*Removes the value at the head of the ring buffer, 
*advances the head, and returns the value.
*/
void* rb_dequeue(struct ring *rb){
	if((rb_isempty(rb) == -1) || (rb == NULL))
		return 0;

	//get return value
	void *value = rb->elems[rb->head];

	//set new head value based on module with size
	rb->head = (rb->head+1) % rb->size;

	return value;
}
