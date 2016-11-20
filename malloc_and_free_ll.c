#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define HEAP_SIZE 2048*sizeof(uint8_t) //Preprocessor macro
#define METADATA_SIZE sizeof(node) //How much to increment base_pointer by when storing size

typedef struct _list_node node;

void *base_pointer;
void *allocated_head_pointer;
void *free_head_pointer;
void *allocated_tail_pointer;
void *free_tail_pointer;


struct _block{
	uint8_t numbers[100];
};

struct _list_node {
	size_t size;
	node *next;
	node *prev;
};

void add(node *node_to_add, bool free) {
	//If head exists, the list is not empty.
	//In this case, the list is sorted by memory address
	//As a result, we can iterate and insert appropriately very easily
	//our goal is to maintain this order
	
	//If head does not exist, the list is empty.
	//Make the head and tail node_to_add
	
	node *temp_pointer = NULL;
//	printf("add flag\n");
	node_to_add->next = NULL;
	node_to_add->prev = NULL;
	if(free) {
	//	printf("add free flag\n");
		if(free_head_pointer) {	
			temp_pointer = free_head_pointer;
		}
		else {
			free_head_pointer = free_tail_pointer = node_to_add;
		}
	}
	else {
	//	printf("add alloc flag\n");
		if(allocated_head_pointer) {	
			temp_pointer = allocated_head_pointer;
		}
		else {
			allocated_head_pointer = allocated_tail_pointer = node_to_add;
		}
	}
	while(temp_pointer) {
	//	printf("%d\n",temp_pointer->size);		
		if(temp_pointer->next){
			//	printf("not at end flag tempointer: %d\n", temp_pointer);
				if(temp_pointer->next > node_to_add) {
				//	printf("right spot flag\n");
					//Given scenario A->C, trying to insert B
					//set C's prev to B
					temp_pointer->next->prev = node_to_add;	
					//Set B's next to C
					node_to_add->next = temp_pointer->next;
					//Set A's next to B
				//	printf("insert halfway flag\n");
					temp_pointer->next = node_to_add;
					//Set B's prev to A
					node_to_add->prev = temp_pointer;
					//Finally, set temp_pointer to null
					//To flag that we have successfully inserted
					//And that we need not append the node to the end
				//	printf("insert done flag\n");
					temp_pointer = NULL;
					}
				else {
					temp_pointer = temp_pointer->next;
				//	printf("tempointer->next: %d\n",temp_pointer->next);
				}
		}
		else if(temp_pointer == free_tail_pointer) {
				//	printf("free tail flag\n");
					//If we hit the end of the list, simply append
					//First, set tail's next to node_to_add
					((node *)free_tail_pointer)->next = node_to_add;
					//Second, set node_to_add's prev to tail
					node_to_add->prev = free_tail_pointer;
					//Finally, set the tail = to node_to_add,
					//Since the node we added is now the tail
					free_tail_pointer = node_to_add;
				//	printf("%d %d",((node *)free_tail_pointer)->next,free_tail_pointer);
					temp_pointer = NULL;
			}
		else if(temp_pointer == allocated_tail_pointer) {
				//	printf("allocated tail flag\n");	
					//If we hit the end of the list, simply append
					//First, set tail's next to node_to_add
					((node *)allocated_tail_pointer)->next = node_to_add;
					//Second, set node_to_add's prev to tail
					node_to_add->prev = allocated_tail_pointer;
					//Finally, set the tail = to node_to_add,
					//Since the node we added is now the tail
					allocated_tail_pointer = node_to_add;
					temp_pointer = NULL;
				//	printf("finish allocated tail flag\n");
			}
	}
}

void print(bool free) {
	node *temp_pointer = free ? free_head_pointer : allocated_head_pointer;
	while(temp_pointer){
		printf("address: %p size: %d\n", temp_pointer, temp_pointer->size);
		temp_pointer = temp_pointer->next;
	}
}

void remove_node(node *node_to_remove, bool free) {
	//	printf("remove flag\n");
		//Remove memory from allocated and into free, or vice versa
		//First, remove the node from the current list
		//From scenario A->B->C where B is node to remove
		//Set A's next to C
		if(node_to_remove->prev) {
			node_to_remove->prev->next = node_to_remove->next;
		}
		else{
			if(free) {
				free_head_pointer = node_to_remove->next;
			}
			else {
				allocated_head_pointer = node_to_remove->next;
			}
		}
		//Set C's prev to A
	//	printf("1st remove flag\n");
		if(node_to_remove->next){
			node_to_remove->next->prev = node_to_remove->prev;
		}
		else {
			if(free) {
				free_tail_pointer = node_to_remove->prev;	
			}
			else {
				allocated_tail_pointer = node_to_remove->prev;
			}
		}	
	//	printf("2nd remove flag\n");
		//Now, add the node to the other list
		//Since we've already removed from the current list,
		//We simply add exactly this node to the other list
		//(the add method will fixup its next and prev pointers)
	//	printf("ready to add flag %p\n",node_to_remove);
		add(node_to_remove, !free);
	//	printf("remove finish flag\n");
}

void *allocate(size_t size) {
//	printf("alloc flag\n");
	//First, check the free list for any available space
//	printf("first free space address: %p\n", free_head_pointer);
	void *poss_free_space_pointer = free_head_pointer;
	while(poss_free_space_pointer) {
//		printf("checking for prefreed memory at  %p\n", poss_free_space_pointer);
		//Check if the current free node has enough space
		if(((node *)poss_free_space_pointer)->size >= size){
//			printf("Deleting prefreed memory at %p\n", poss_free_space_pointer);
			//If the node has enough space, use it.
			remove_node(((node *)poss_free_space_pointer), true);
			poss_free_space_pointer += METADATA_SIZE;
			printf("You will be allocated memory at this location: %p of this size: %d\n", 
				poss_free_space_pointer, ((node *)(poss_free_space_pointer-METADATA_SIZE))->size);	
			return poss_free_space_pointer;
		}
		//If we have reached this line, no node thus far has had enough space
		//As such, we increment the pointer and continue iterating
		poss_free_space_pointer = ((node *)poss_free_space_pointer)->next;
	}
	//Create a temp node at current mem address
	//save metadata into node, increment past metadata
	//return new pointer to memory past metadata
	node *temp_node = base_pointer; 
	temp_node->size = size;
//	printf("size = %d",size);
	add(temp_node, false);
	base_pointer += METADATA_SIZE;
	void *temp_pointer = base_pointer;
	printf("You will be allocated memory at this location: %p of this size: %d\n", temp_pointer, ((node *)(temp_pointer-METADATA_SIZE))->size);
	base_pointer += size;
	return temp_pointer;
}

void free(void *address_to_free){
	//Decrement pointer to location of size
	address_to_free -= METADATA_SIZE;
	printf("Freeing memory loc %p of size: %d\n", address_to_free + METADATA_SIZE, ((node *)(address_to_free))->size);
	//Remove from allocated list, which also adds to free list.	
	remove_node(((node *)address_to_free), false);
}

int main(int argc, const char * argv[]) {
	base_pointer = malloc(HEAP_SIZE);

	uint8_t *numberz = allocate(sizeof(uint8_t)*100);
	uint8_t *numberz_two = allocate(sizeof(uint8_t)*45);
	uint8_t *not_big_enough = allocate(sizeof(uint8_t)*20);
		
	for(int i = 0; i < 100; i ++) {
		numberz[i] = i;
	}	
	for(int i = 0; i < 45; i ++) {
		numberz_two[i] = i;
	}
	for(int i = 0; i < 20; i ++) {
		not_big_enough[i] = 2*i;
	}
	
	printf("%d\n",memcmp(numberz, numberz_two, 45));
	
	free(not_big_enough);
	free(numberz);
	
	uint8_t *numberz_three = allocate(sizeof(uint8_t)*30);
	printf("%d\n",memcmp(numberz_three, numberz_two, 30));

	return 0;
}
