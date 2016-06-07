/*
* Tom Choi, Kiya Govek, Jonah Tuchow
* Implementation of linked lists in c,
* for use with talloc (this linked list uses malloc)
*/

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Create a new NULL_TYPE value node.
Value *makeNull(){
    Value *nulltype = talloc(sizeof(Value));
    nulltype->type = NULL_TYPE;
    return nulltype;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    assert(value);
    return (value->type == NULL_TYPE);
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr){ 
    Value *constype = talloc(sizeof(Value));
    constype->type = CONS_TYPE;
    constype->c.car = car;
    constype->c.cdr = cdr;
    return constype;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
    switch (list->type) {
        case (CONS_TYPE):
            display(list->c.car);
            display(list->c.cdr);
            break;
        case (INT_TYPE):
            printf("%i ", list->i);
            break;
        case (DOUBLE_TYPE):
            printf("%f ", list->d);
            break;
        case (STR_TYPE):
            printf("%s ", list->s);
            break;
        case (PTR_TYPE):
            printf("%p ", list->p);
            break;
        case (NULL_TYPE):
            break;
        case (OPEN_TYPE):
            printf("(\n");
            break;
        case (CLOSE_TYPE):
            printf(")\n");
            break;
        case (BOOL_TYPE):
            printf("%s ", list->s);
            break;
        case (SYMBOL_TYPE):
            printf("%s ", list->s);
            break;
        default:
            break;
    }
}

// Performs recursive step of reverse process
Value *reverseHelper(Value *list, Value *newlist) {
    assert(list);
    assert(isNull(list) || list->type == CONS_TYPE);
    if (isNull(list)) {
        return newlist;
    } else {
        newlist = cons(car(list), newlist);
        return reverseHelper(cdr(list), newlist);
    }
}

// Return a new list that is the reverse of the one that is passed in.
Value *reverse(Value *list){
    Value *newlist = makeNull();
    newlist = reverseHelper(list, newlist);
    return newlist;
}

// Utility to make it less typing to get car value.
Value *car(Value *list){
    assert(list);
    assert(list->type == CONS_TYPE);
    return (list->c.car);
}

// Utility to make it less typing to get cdr value.
Value *cdr(Value *list){
    assert(list);
    assert(list->type == CONS_TYPE);
    return (list->c.cdr);
}

// Measure length of list.
int length(Value *value){
    assert(value);
    if (value->type == CONS_TYPE) {
        return length(value->c.cdr)+1;
    } else {
        return 0;
    }
}
