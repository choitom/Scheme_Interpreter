/*
* Tom Choi, Kiya Govek, Jonah Tuchow
* Implementation of talloc which stores
* every pointer to memory blocks allocated in the heap,
* and tfree that frees every memory block in the heap
*/

#include <stdlib.h>
#include <stdio.h>
#include "value.h"

// Global declaration of the head
Value *head;
int freed = 0;

// Helper method for creating a pointer to NULL_TYPE Value
Value *makeNull_talloc(){
    Value *nulltype = malloc(sizeof(Value));
    nulltype->type = NULL_TYPE;
    return nulltype;
}

// Helper method for creating a pointer to CONS_TYPE Value
Value *cons_talloc(Value *car, Value *cdr){ 
    Value *constype = malloc(sizeof(Value));
    constype->type = CONS_TYPE;
    constype->c.car = car;
    constype->c.cdr = cdr;
    return constype;
}

// Creates a pointer to a memory block of an input size
// and stores that pointer into a global linked list
void *talloc(size_t size) {
    void *val = malloc(size);
    if (head == NULL){
        head = makeNull_talloc();
    }
    head = cons_talloc(val, head);
    return val;
}

// Helper method for tfree
// Frees car and then recursively frees cdr part
void tfree_helper(Value *ptr){
    if (ptr->type == CONS_TYPE){
        free(ptr->c.car);
        tfree_helper(ptr->c.cdr);
        free(ptr);
    }else{
        free(ptr);
    }
}

// Frees every memory block in the heap
void tfree() {
    if (head->type == CONS_TYPE){
        free(head->c.car);
        tfree_helper(head->c.cdr);
        free(head);
    }else{
        free(head);
    }
    freed = 1;
}

// Frees the memory allocated in the heap and then exits
void texit(int status) {
    if (freed == 0){
        tfree();
    }
    exit(status);
}
