/*
Primitive Functions for Scheme interpreter
Created by Tom Choi, Kaya Govek, Jonah Tuchow
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "interpreter.h"
#include "linkedlist.h"
#include "talloc.h"
#include "value.h"
#include "tokenizer.h"
#include "parser.h"

Value *checkMathArgs(Value *args, char *symbol) {
    Value *numArgs = car(checkNumArgs(args));
    if (numArgs->i != 2){
        printf("%s: arity mismatch;\nthe expected number of arguments ", symbol);
        printf("does not match the given number\nexpected: 2\ngiven: ");
        printf("%d\n", numArgs->i);
        texit(1);
    }
    
    double first;
    double second;
    
    // evaluate and check the arguments
    if (car(args)->type == INT_TYPE) {
        first = car(args)->i;
    } else if (car(args)->type == DOUBLE_TYPE) {
        first = car(args)->d;
    } else {
        printf("%s: contract violation\nexpected: number?\ngiven: ", symbol);
        printInterpTree(car(args));
        printf("\n");
    }
    if (car(cdr(args))->type == INT_TYPE) {
        second = car(cdr(args))->i;
    } else if (car(cdr(args))->type == DOUBLE_TYPE) {
        second = car(cdr(args))->d;
    } else {
        printf("%s: contract violation\nexpected: number?\ngiven: ", symbol);
        printInterpTree(car(cdr(args)));
        printf("\n");
    }
    
    Value *return_args = makeNull();
    Value *first_container = makeNull();
    first_container->type = DOUBLE_TYPE;
    first_container->d = first;
    Value *second_container = makeNull();
    second_container->type = DOUBLE_TYPE;
    second_container->d = second;
    return_args = cons(second_container, return_args);
    return_args = cons(first_container, return_args);
    return return_args;
}

Value *primitiveAdd(Value *args){
    Value *args_ptr = args;
    Value *addProduct = makeNull();
    int type = 0; // 0 - int type, 1 - double type
    int position = 0; // position of an argument
    double sum = 0;
    
    while(args_ptr->type != NULL_TYPE){
        position++;
        Value *v = car(args_ptr);
        
        if (v->type == INT_TYPE){
            sum = sum + v->i;
        } else if (v->type == DOUBLE_TYPE){
            type = 1;
            sum = sum + v->d;
        } else {
            printf("+: contract violation\nexpected: number?\ngiven: ");
            printInterpTree(v);
            printf("\nargument position: %d\n", position);
            texit(1);
        }
        args_ptr = cdr(args_ptr);
    }
    // If no argument, return 0
    if (position == 0){
        addProduct->type = INT_TYPE;
        addProduct->i = 0;
    }else{
        if (type == 0){
            addProduct->type = INT_TYPE;
            int int_sum = sum;
            addProduct->i = int_sum;
        }else{
            addProduct->type = DOUBLE_TYPE;
            addProduct->d = sum;
        }
    }
    return addProduct;
}

Value *primitiveSubtract(Value *args){
    Value *args_ptr = args;
    Value *subProduct = makeNull();
    int type = 0; // 0 - int type, 1 - double type
    int position = 0; // position of an argument
    double difference = 0;
    
    while(args_ptr->type != NULL_TYPE){
        position++;
        Value *v = car(args_ptr);
        
        if (position == 1 && cdr(args_ptr)->type != NULL_TYPE) {
            if (v->type == INT_TYPE){
                difference = difference + v->i;
            } else if (v->type == DOUBLE_TYPE){
                type = 1;
                difference = difference + v->d;
            } else {
                printf("-: contract violation\nexpected: number?\ngiven: ");
                printInterpTree(v);
                printf("\nargument position: %d\n", position);
                texit(1);
            }
        } else {
            if (v->type == INT_TYPE){
                difference = difference - v->i;
            } else if (v->type == DOUBLE_TYPE){
                type = 1;
                difference = difference - v->d;
            } else {
                printf("-: contract violation\nexpected: number?\ngiven: ");
                printInterpTree(v);
                printf("\nargument position: %d\n", position);
                texit(1);
            }
        }
        args_ptr = cdr(args_ptr);
    }
    if (position == 0){
        printf("-: arity mismatch;\nthe expected number of arguments ");
        printf("does not match the given number\nexpected: at least 1\n");
        printf("given: 0 arguments;\n");
        texit(1);
    }else{
        if (type == 0){
            subProduct->type = INT_TYPE;
            int int_difference = difference;
            subProduct->i = int_difference;
        }else{
            subProduct->type = DOUBLE_TYPE;
            subProduct->d = difference;
        }
    }
    return subProduct;
}

Value *primitiveMult(Value *args){
    Value *args_ptr = args;
    Value *multProduct = makeNull();
    int type = 0; // 0 - int type, 1 - double type
    int position = 0; // position of an argument
    double product = 1;
    
    while(args_ptr->type != NULL_TYPE){
        position++;
        Value *v = car(args_ptr);
        
        if (v->type == INT_TYPE){
            product = product * v->i;
        } else if (v->type == DOUBLE_TYPE){
            type = 1;
            product = product * v->d;
        } else {
            printf("*: contract violation\nexpected: number?\ngiven: ");
            printInterpTree(v);
            printf("\nargument position: %d\n", position);
            texit(1);
        }
        args_ptr = cdr(args_ptr);
    }
    // If no argument, return 1
    if (position == 0){
        multProduct->type = INT_TYPE;
        multProduct->i = 1;
    }else{
        if (type == 0){
            multProduct->type = INT_TYPE;
            int int_product = product;
            multProduct->i = int_product;
        }else{
            multProduct->type = DOUBLE_TYPE;
            multProduct->d = product;
        }
    }
    return multProduct;
}

Value *primitiveDivide(Value *args){
    Value *args_ptr = args;
    Value *divProduct = makeNull();
    int type = 0; // 0 - int type, 1 - double type
    int position = 0; // position of an argument
    double quotient = 1;
    
    while(args_ptr->type != NULL_TYPE){
        position++;
        Value *v = car(args_ptr);
        
        if (position == 1 && cdr(args_ptr)->type != NULL_TYPE) {
            if (v->type == INT_TYPE){
                quotient = v->i;
            } else if (v->type == DOUBLE_TYPE){
                type = 1;
                quotient = v->d;
            } else {
                printf("/: contract violation\nexpected: number?\ngiven: ");
                printInterpTree(v);
                printf("\nargument position: %d\n", position);
                texit(1);
            }
        } else {
            if (v->type == INT_TYPE){
                if (v->i == 0) {
                    printf("/: divide by 0 error\n");
                    texit(1);
                }
                if (fmod(quotient,v->i) != 0.0) {
                    type = 1; //always makes quotient a double
                }
                quotient = quotient / v->i;
            } else if (v->type == DOUBLE_TYPE){
                if (v->d == 0.0) {
                    printf("/: divide by 0 error\n");
                    texit(1);
                }
                type = 1;
                quotient = quotient / v->d;
            } else {
                printf("/: contract violation\nexpected: number?\ngiven: ");
                printInterpTree(v);
                printf("\nargument position: %d\n", position);
                texit(1);
            }
        }
        args_ptr = cdr(args_ptr);
    }
    if (position == 0){
        printf("-: arity mismatch;\nthe expected number of arguments ");
        printf("does not match the given number\nexpected: at least 1\n");
        printf("given: 0 arguments;\n");
        texit(1);
    }else{
        if (type == 0){
            divProduct->type = INT_TYPE;
            int int_quotient = quotient;
            divProduct->i = int_quotient;
        }else{
            divProduct->type = DOUBLE_TYPE;
            divProduct->d = quotient;
        }
    }
    return divProduct;
}

Value *primitiveModulo(Value *args){
    // check the number of arguments
    int i = 0;
    Value *args_ptr = args;
    
    while (args_ptr->type != NULL_TYPE){
        i = i + 1;
        args_ptr = cdr(args_ptr);
    }
    if (i != 2){
        printf("modulo: arity mismatch;\nthe expected number of arguments ");
        printf("does not match the given number\nexpected: 2\ngiven: ");
        printf("%d\n", i);
        texit(1);
    }
    
    Value *remainder = makeNull();
    int first;
    int second;
    
    if (car(cdr(args))->type == INT_TYPE) {
        second = car(cdr(args))->i;
        if (second == 0) {
            printf("modulo: divide by 0 error\n");
            texit(1);
        }
    } else {
        printf("modulo: contract violation\nexpected: int?\ngiven: ");
        printInterpTree(car(cdr(args)));
        printf("\n");
    }
    if (car(args)->type == INT_TYPE) {
        first = car(args)->i;
        remainder->type = INT_TYPE;
        remainder->i = first % second;
        if (remainder->i < 0 && first < 0 && second > 0) {
            remainder->i += second;
        } else if (remainder->i > 0 && first > 0 && second < 0) {
            remainder->i += second;
        }
    } else if (car(args)->type == DOUBLE_TYPE) {
        first = car(args)->d;
        remainder->type = DOUBLE_TYPE;
        remainder->d = first % second;
        if (remainder->d < 0 && first < 0 && second > 0) {
            remainder->d += second;
        } else if (remainder->d > 0 && first > 0 && second < 0) {
            remainder->d += second;
        }
    } else {
        printf("modulo: contract violation\nexpected: number?\ngiven: ");
        printInterpTree(car(args));
        printf("\n");
    }
    
    return remainder;
}

Value *primitiveNull(Value *args){
    Value *args_ptr = args;
    Value *boolValue = makeNull();
    boolValue->type = BOOL_TYPE;
    
    // check if the number of arguments is equal to 1
    int i = 0;
    int isNull = 0;
    
    while(args_ptr->type != NULL_TYPE){
        i = i + 1;
        args_ptr = cdr(args_ptr);
    }
    if (i != 1){
        printf("null?: arity mismatch;\nthe expected number of arguments ");
        printf("does not match the given number\nexpected: 1\ngiven: ");
        printf("%d arguments;\n", i);
        texit(1);
    }
    // see if args is an empty list
    else{
        if (args->type == CONS_TYPE){
            if (car(args)->type == NULL_TYPE){
                isNull = 1;
            }
        }
        if (isNull == 0){
            boolValue->s = "#f";
        }else{
            boolValue->s = "#t";
        }
    }
    return boolValue;
}

Value *primitiveCdr(Value *args){
    Value *args_ptr = args;
    
    // want to check args is in the form of a list : 1 - 2 - 3 - null_type
    if (args_ptr->type == CONS_TYPE){
        if (car(args_ptr)->type != CONS_TYPE){
            printf("cdr: contract violation\nexpected: pair?\ngiven: ");
            printInterpTree(args);
            printf("\n");
            texit(1); 
        }
    }
    int i = 0;
    while (args_ptr->type != NULL_TYPE){
        i = i + 1;
        args_ptr = cdr(args_ptr);
    }

    if (i != 1){
        printf("cdr: arity mismatch;\nthe expected number of arguments ");
        printf("does not match the given number\nexpected: 1\ngiven: ");
        printf("%d arguments;\n", i);
        texit(1);
    }
    
    Value *list = cdr(car(args));
    if (list->type == CONS_TYPE){
        if (car(list)->type == STR_TYPE &&
            !strcmp(car(list)->s, ".")){
            list = cdr(list);
        }  
    }
    return list;
}

Value *primitiveCar(Value *args){
    Value *args_ptr = args;
    if (car(args_ptr)->type != CONS_TYPE){
        printf("car: contract violation\nexpected: pair?\ngiven: ");
        printInterpTree(args);
        printf("\n");
        texit(1);
    }
    int i = 0;
    while (args_ptr->type != NULL_TYPE){
        i = i + 1;
        args_ptr = cdr(args_ptr);
    }

    if (i != 1){
        printf("car: arity mismatch;\nthe expected number of arguments ");
        printf("does not match the given number\nexpected: 1\ngiven: ");
        printf("%d arguments;\n", i);
        texit(1);
    }
    return car(car(args));
}


Value *primitiveCons(Value *args){
    // check the number of arguments
    int i = 0;
    Value *args_ptr = args;
    
    while (args_ptr->type != NULL_TYPE){
        i = i + 1;
        args_ptr = cdr(args_ptr);
    }
    if (i != 2){
        printf("cons: arity mismatch;\nthe expected number of arguments ");
        printf("does not match the given number\nexpected: 2\ngiven: ");
        printf("%d\n", i);
        texit(1);
    }
    
    Value *carPart;
    Value *cdrPart;
    carPart = car(args);
    cdrPart = car(cdr(args));
    
    Value *consReturn = cdrPart;
    
    // improper list: add a dot
    if (cdrPart->type != CONS_TYPE && cdrPart->type != NULL_TYPE){
        Value *dot = makeNull();
        dot->type = STR_TYPE;
        dot->s = ".";
        consReturn = cons(dot, consReturn);
    }
    
    consReturn = cons(carPart, consReturn);
    
    return consReturn;
}

Value *primitiveEqual(Value *args) {
    char *symbol = "=";
    Value *new_args = checkMathArgs(args, symbol);
    double first = car(new_args)->d;
    double second = car(cdr(new_args))->d;
    
    Value *equalReturn = makeNull();
    equalReturn->type = BOOL_TYPE;
    if (first == second) {
        equalReturn->s = "#t";
    } else {
        equalReturn->s = "#f";
    }
    return equalReturn;
}

Value *primitiveGreater(Value *args) {
    char *symbol = ">";
    Value *new_args = checkMathArgs(args, symbol);
    double first = car(new_args)->d;
    double second = car(cdr(new_args))->d;
    
    Value *greaterReturn = makeNull();
    greaterReturn->type = BOOL_TYPE;
    if (first > second) {
        greaterReturn->s = "#t";
    } else {
        greaterReturn->s = "#f";
    }
    return greaterReturn;
}

Value *primitiveGreaterEqual(Value *args) {
    char *symbol = ">=";
    Value *new_args = checkMathArgs(args, symbol);
    double first = car(new_args)->d;
    double second = car(cdr(new_args))->d;
    
    Value *greaterEqualReturn = makeNull();
    greaterEqualReturn->type = BOOL_TYPE;
    if (first >= second) {
        greaterEqualReturn->s = "#t";
    } else {
        greaterEqualReturn->s = "#f";
    }
    return greaterEqualReturn;
}

Value *primitiveLess(Value *args) {
    char *symbol = "<";
    Value *new_args = checkMathArgs(args, symbol);
    double first = car(new_args)->d;
    double second = car(cdr(new_args))->d;
    
    Value *lessReturn = makeNull();
    lessReturn->type = BOOL_TYPE;
    if (first < second) {
        lessReturn->s = "#t";
    } else {
        lessReturn->s = "#f";
    }
    return lessReturn;
}

Value *primitiveLessEqual(Value *args) {
    char *symbol = "<=";
    Value *new_args = checkMathArgs(args, symbol);
    double first = car(new_args)->d;
    double second = car(cdr(new_args))->d;
    
    Value *lessEqualReturn = makeNull();
    lessEqualReturn->type = BOOL_TYPE;
    if (first <= second) {
        lessEqualReturn->s = "#t";
    } else {
        lessEqualReturn->s = "#f";
    }
    return lessEqualReturn;
}

