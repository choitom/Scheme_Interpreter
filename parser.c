// Jonah Tuchow
// CS 251 - Parser
// May 11, 2016

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"
#include "interpreter.h"

// helper for print function that avoids printing first open paren at first
// new depth
void printTreeHelper(Value *tree, int firstItem) {
    switch(tree->type){
        case(CONS_TYPE):
            if (firstItem) {
                firstItem = 0;
            } else {
                printf("(");
            }
            printTreeHelper(car(tree), firstItem);
            if (car(tree)->type == CONS_TYPE) {
                printf(")");
            }
            if (cdr(tree)->type != NULL_TYPE) {
                printf(" ");
            }
            printTreeHelper(cdr(tree), 1);
            break;
        case(STR_TYPE):
            printf("%s", tree->s);
            break;
        case(INT_TYPE):
            printf("%d", tree->i);
            break;
        case(DOUBLE_TYPE):
            printf("%lf", tree->d);
            break;
        case(BOOL_TYPE):
            printf("%s", tree->s);
            break;
        case(SYMBOL_TYPE):
            printf("%s", tree->s);
            break;
        default:
            break;
    }
}

// Prints the tree to the screen in a readable fashion. Looks just like
// Racket code; uses parentheses to indicate subtrees.
void printTree(Value *tree) {
    printTreeHelper(tree, 1);
}

// Displays a syntax error and exits parser (could be expanded in future)
void syntaxError(int error) {
    printf("Syntax Error: ");
    switch (error) {
        case(1):
            printf("premature close parenthesis");
            break;
        case(2):
            printf("unclosed parenthesis");
            break;
    }
    printf("\n");
    texit(1);
}
    
// Add a token to the parse tree. If it's a close paren, pop off all tokens
// up to the last open paren, and add the poped tokens as a new subtree.
Value *addToParseTree(Value *tree, int *depth, Value *token) {
    if (token->type == CLOSE_TYPE) {
        Value *stack = makeNull();
        *depth = *depth - 1;
        if (*depth < 0) {
            syntaxError(1);
        }
        while (token->type != OPEN_TYPE) {
            if (token->type != CLOSE_TYPE) {
                stack = cons(token, stack);
                tree = cdr(tree);
            }
            if (tree->type == CONS_TYPE) {
                token = car(tree);
            } else {
                token = tree;
            }
        }
        if (tree->type == CONS_TYPE) {
            tree = cdr(tree); //remove open paren from tree
        }
        tree = cons(stack, tree);
    } else {
        if (token->type == OPEN_TYPE) {
            *depth = *depth + 1;
        }
        tree = cons(token, tree);
    }
    return tree;
}

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *list) {
    Value *tree = makeNull();
    int depth = 0;
    Value *current = list;
    assert(current != NULL && "Error (parse): null pointer");
    
    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree,&depth,token);
        current = cdr(current);
    }
    
    if (depth > 0) {
        syntaxError(2);
    }
    
    Value *newTree = makeNull();
    while (tree->type == CONS_TYPE) { //reverse multiline program
        newTree = cons(car(tree), newTree);
        tree = cdr(tree);
    }
    
    return newTree;
}