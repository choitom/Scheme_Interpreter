/*
Spring Term 2016
CS 251 Programming Language Design & Implementation

Implementation of Scheme interpreter
Created by Tom Choi, Kaya Govek, Jonah Tuchow

With a given parse tree, it interprets the following expressions:
    and, begin, cond, define, if, let, let*, letrec, quote, set!
    +, null?, cdr, car, cons, *, -, /, modulo, <, <=, >, >=, =

Note: correctly evaluates Knuth's test
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpreter.h"
#include "linkedlist.h"
#include "talloc.h"
#include "value.h"
#include "tokenizer.h"
#include "parser.h"
#include "primitives.h"

Frame *globalFrame;
int procedureDisplay;

// throws an evaluation error
void evaluationError(){
    printf("Evaluation Error\n");
    texit(1);
}

// throws errors for invalid inputs following 'quote'
void quoteError(Value *args){
    printf("quote: wrong number of parts in: (quote ");
    printInterpTree(args);
    printf(")\n");
    texit(1);
}

//throws an application error
void applicationError(Value *tree){
    printf("application: not a procedure;\n");
    printf("expected a procedure that can be applied to arguments\n");
    printf("given: ");
    printInterpTree(tree);
    printf("\narguments...\n");
    texit(1);            
}

int printInterpTreeHelper(Value *tree, int firstItem) {
    int i = 0;
    switch(tree->type){
        case(CONS_TYPE):
            if (firstItem) {
                firstItem = 0;
            } else {
                printf("(");
            }
            printInterpTreeHelper(car(tree), firstItem);
            if (car(tree)->type == CONS_TYPE) {
                printf(")");
            }
            if (car(tree)->type == NULL_TYPE) {
                printf("()"); // empty list
            }
            if (cdr(tree)->type != NULL_TYPE) {
                printf(" ");
            }
            printInterpTreeHelper(cdr(tree), 1); 
            break;
        case(STR_TYPE):
            printf("%s", tree->s);
            break;
        case(INT_TYPE):
            printf("%i", tree->i);
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
        case(CLOSURE_TYPE):
            printf("#<procedure>");
            i = 1;
            break;
        default:
            break;
    }
    return i;
}

// Prints the tree to the screen in a readable fashion. Looks just like
// Racket code; uses parentheses to indicate subtrees.
void printInterpTree(Value *tree){
    procedureDisplay = printInterpTreeHelper(tree, 1);
}

// globally bind a string to a primitive function
void bind(char *name, Value *(*function)(struct Value *)){
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;

    Value *symbol = talloc(sizeof(Value));
    symbol->type = SYMBOL_TYPE;
    symbol->s = name;
    
    Value *list = makeNull();
    list = cons(value, list);
    list = cons(symbol, list);
    
    globalFrame->bindings = cons(list, globalFrame->bindings);
}

// returns value of input symbol or throws error if symbol is not in any frame
Value *lookUpSymbol(Value *symbol, Frame *frame, int modify){
    // error if frame is undefined
    if (frame == NULL){
        printf("Error: undefined frame\n");
        texit(1);
    }

    Value *bindings = frame->bindings;
    while (bindings->type != NULL_TYPE) {
        // if symbol found
        if (!strcmp(car(car(bindings))->s, symbol->s)) {
            // if symbol bound to another symbol, look up that symbol
            if (car(cdr(car(bindings)))->type == SYMBOL_TYPE) {
                return lookUpSymbol(car(cdr(car(bindings))), frame->parent, modify);
            }
            else if (modify == 1){
                return cdr(car(bindings));
            }else{
                return car(cdr(car(bindings)));
            }
        }  else {
            bindings = cdr(bindings);
        }
    }
    if (frame->parent == NULL) {
        printf("%s: undefined;\ncannot reference undefined identifier\n",
               symbol->s);
        texit(1);
    }
    return lookUpSymbol(symbol, frame->parent, modify);
}

// interprets input parse tree
// initializes the gloal frame that stores
// the bindings of variables and expressions of define statements
void interpret(Value *tree){
    globalFrame = talloc(sizeof(Frame));
    globalFrame->bindings = makeNull();
    
    // bind primitives to the global frame
    bind("+", primitiveAdd);
    bind("-", primitiveSubtract);
    bind("*", primitiveMult);
    bind("/", primitiveDivide);
    bind("modulo", primitiveModulo);
    bind("null?", primitiveNull);
    bind("cdr", primitiveCdr);
    bind("car", primitiveCar);
    bind("cons", primitiveCons);
    bind("=", primitiveEqual);
    bind(">", primitiveGreater);
    bind(">=", primitiveGreaterEqual);
    bind("<", primitiveLess);
    bind("<=", primitiveLessEqual);
    
    while(tree->type != NULL_TYPE){
        Frame *frame = talloc(sizeof(Frame));
        frame->bindings = makeNull();
        frame->parent = globalFrame;
        Value *value = eval(car(tree), frame);
        printInterpTree(value);

        if (value->type == CLOSURE_TYPE){
            if (procedureDisplay == 1 && value->type == CONS_TYPE){
                if (strcmp(car(car(tree))->s, "lambda")){
                    printf(":");
                    printInterpTree(car(tree));  
                }
            }
        }else if(value->type == PRIMITIVE_TYPE){
            printf("#<procedure>:%s",car(tree)->s);
        }
        if (value->type != VOID_TYPE && value->type != NULL_TYPE){
            printf("\n");
        }
        tree = cdr(tree);
    }
}

// check if a list of bindings is a nested list and if each binding consists
// of a variable and a value
int checkNestedInput(Value *binding_list, Frame *frame, int letType){
    // letType : 0 = let, 1 = let*
    int let = letType;
    
    if (car(binding_list)->type != CONS_TYPE){
        return 0;
    }
    Value *lst = binding_list;
    
    int i = 0;
    while(lst->type != NULL_TYPE){
        i = 0;
        Value *binding_ptr = car(lst);
        Value *nested_list = binding_ptr;
        while(nested_list->type != NULL_TYPE){
            Value *nested_ptr = car(nested_list);
            i  = i + 1;
            
            // error checking for cases like (let ((x 5 y)))
            if (i > 2){
                return 0;
            }
            
            // error checking for cases like (let ((5 10)) 5),
            // (let (("a" 10)) "a")
            if (i == 1){
                switch (nested_ptr->type){
                    printf("let: ");
                    case INT_TYPE:{
                        printf("%d not an identifier\n",nested_ptr->i);
                        texit(1);
                        break;
                    }
                    case DOUBLE_TYPE:{
                        printf("%f not an identifier\n",nested_ptr->d);
                        texit(1);
                        break;
                    }
                    case STR_TYPE:{
                        printf("%s not an identifier\n",nested_ptr->s);
                        texit(1);
                        break;
                    }
                    case BOOL_TYPE:{
                        printf("%s not an identifier\n",nested_ptr->s);
                        texit(1);
                        break;
                    }
                    default:{
                        break;
                    }
                }
            }
            //error checking for cases like (let (a b) a), where b is undefined
            else if (i == 2){
                if (let == 0){
                    switch (nested_ptr->type){
                        case SYMBOL_TYPE:{
                            Value *symbol = lookUpSymbol(nested_ptr, frame, 0);
                            break;
                        }default:{
                            break;
                        }
                    }
                }
            }
            nested_list = cdr(nested_list);
        }
        lst = cdr(lst);
    }
    if (i != 2){
        return 0;
    }else{
        return 1;
    }
}

Value *checkNumArgs(Value *args) {
    int i = 0;
    Value *arg_check = args;
    Value *previous;
    Value *output = makeNull();
    
    while(arg_check->type != NULL_TYPE){
        i = i +1;
        previous = car(arg_check);
        arg_check = cdr(arg_check);
    }
    
    //format output
    Value *numArgs = makeNull();
    numArgs->type = INT_TYPE;
    numArgs->i = i;
    output = cons(previous, output);
    output = reverse(output);
    output = cons(numArgs, output);
    return output;
}

Value *checkLetArgs(Value *args, Frame *frame, int star) {
    if (args->type == CONS_TYPE) {
        if (car(args)->type != CONS_TYPE) {
            printf("let: first arg should be list of bindings\n");
            texit(1);
        }
    }
    
    int i = 0;
    Value *arg_check = args;
    Value *previous;
    Value *output = makeNull();
    Value *body = makeNull();
    
    while(arg_check->type != NULL_TYPE){
        i = i +1;
        previous = car(arg_check);
        arg_check = cdr(arg_check);
        
        if (arg_check->type == CONS_TYPE){
            if (car(arg_check)->type == CONS_TYPE){
                if (car(car(arg_check))->type == SYMBOL_TYPE){
                    if (!strcmp(car(car(arg_check))->s, "set!")
                       || !strcmp(car(car(arg_check))->s, "begin")){
                        body = cons(car(arg_check), body);
                    }
                }
            }
        }
    }
    
    body = cons(previous, body);
    body = reverse(body);
//    printTree(body);
//    printf("\n");
    
    if (i < 2){
        printf("let: needs two arguments (1) a list of bindings (2) body\n");
        texit(1);
    }
    
    int nested = checkNestedInput(car(args), frame, star);
    if (!nested){
        printf("let: bindings formated incorrectly\n");
        texit(1);
    }
    return body;
}

//evaluate set! statement
Value *evalSet(Value *args, Frame *frame){
    Value *numArgs = car(checkNumArgs(args));

    if (numArgs->i != 2){
        printf("set! doesn't have exactly two arguments");
        texit(1);
    }
    Value *symbol = car(args);
    Value *target_value = eval(car(cdr(args)), frame);
    
    // retrieve the value associated with the symbol
    Value *binding = lookUpSymbol(symbol, frame, 1);
    
    // see if the target value is defined or not if it is a symbol type
    if (target_value->type == SYMBOL_TYPE){
        Value *value_check = lookUpSymbol(target_value, frame, 0);  
    }
    
    // change the value
    binding->c.car = target_value;
    
    return makeNull();
}

// evalBegin helper
Value *evalBeginHelper(Value *args, Frame *frame, int argNum){
    if (argNum == 1){
        return eval(car(args), frame);
    }else{
        Value *dummy_eval = eval(car(args), frame);
        return evalBeginHelper(cdr(args), frame, argNum-1);
    }
}

//evaluate begin statement
Value *evalBegin(Value *args, Frame *frame){
    Value *numArgs = car(checkNumArgs(args));
    Value *last_subexpression;
    if (numArgs->i == 0){
        last_subexpression = makeNull();
    }else{
        last_subexpression = evalBeginHelper(args, frame, numArgs->i);
    }
    return last_subexpression;
}

//evaluates body of let expression
Value *evalLetBody(Value *lastArg, Frame *frame){
    Value *body = lastArg;
    while (cdr(body)->type != NULL_TYPE) {
        eval(car(body), frame);
        body = cdr(body);
    }
    return body;
}

//evaluate if statement
Value *evalIf(Value *args, Frame *frame){
    Value *numArgs = car(checkNumArgs(args));
    
    if (numArgs->i != 3){
        printf("if: doesn't have exactly 3 arguments\n");
        texit(1);
    }
    
    // check if the first args is boolean or not
    Value *first_arg = eval(car(args), frame);
    if (first_arg->type != BOOL_TYPE){
        printf("if: test arg is not BOOL_TYPE\n");
        texit(1);
    }
    
    Value *returnEvalIf;
    
    // if true, evaluate the second argument
    if (!strcmp(first_arg->s, "#t")){
        returnEvalIf = eval(car(cdr(args)), frame);
    }
    // if false, evaluate the third argument
    else{
        returnEvalIf = eval(car(cdr(cdr(args))), frame);
    }
    return returnEvalIf;
}

//evaluate and statement
Value *evalAnd(Value *args, Frame *frame){
    Value *returnVal = makeNull();
    returnVal->type = BOOL_TYPE;
    
    Value *arg_check = args;
    while (arg_check->type != NULL_TYPE){
        Value *currentArg = eval(car(arg_check), frame);
        if (currentArg->type != BOOL_TYPE) {
            printf("and: bool? expected for arguments\n");
            texit(1);
        }
        else if (!strcmp(currentArg->s, "#f")){
            returnVal->s = "#f";
            return returnVal;
        }
        arg_check = cdr(arg_check);
    }
    
    returnVal->s = "#t";
    return returnVal;
}

//evaluate or statement
Value *evalOr(Value *args, Frame *frame){
    Value *returnVal = makeNull();
    returnVal->type = BOOL_TYPE;
    
    Value *arg_check = args;
    while (arg_check->type != NULL_TYPE){
        Value *currentArg = eval(car(arg_check), frame);
        if (currentArg->type != BOOL_TYPE) {
            printf("or: bool? expected for arguments\n");
            texit(1);
        }
        else if (!strcmp(currentArg->s, "#t")){
            returnVal->s = "#t";
            return returnVal;
        }
        arg_check = cdr(arg_check);
    }
    
    returnVal->s = "#f";
    return returnVal;
}

//evaluate cond statement
Value *evalCond(Value *args, Frame *frame){
    Value *arg_check = args;
    while (arg_check->type != NULL_TYPE){
        if (car(arg_check)->type != CONS_TYPE) {
            printf("cond: arguments formatted incorrectly\n");
            texit(1);
        }
        if (car(car(arg_check))->type == SYMBOL_TYPE) {
            if (!strcmp(car(car(arg_check))->s, "else")) {
                return eval(car(cdr(car(arg_check))), frame);
            }
        } else {
            Value *conditional = eval(car(car(arg_check)), frame);
            if (conditional->type != BOOL_TYPE){
                printf("cond: bool? expected for conditional arg\n");
                texit(1);
            }
            else if (!strcmp(conditional->s, "#t")){
                return eval(car(cdr(car(arg_check))), frame);
            }
        }
        
        arg_check = cdr(arg_check);
    }
    return makeNull();
}

//evaluate let statement
Value *evalLet(Value *args, Frame *frame){
    Value *lastArg = checkLetArgs(args, frame, 0);
    // evaluate the bindings
    Value *binding_list = car(args);
    Value *new_binding_list = makeNull();
    while (binding_list->type != NULL_TYPE) {
        Value *cur_binding = car(binding_list);
        Value *new_binding = makeNull();
        new_binding = cons(eval(car(cdr(cur_binding)), frame), new_binding);
        new_binding = cons(car(cur_binding), new_binding);
        new_binding_list = cons(new_binding, new_binding_list);
        
        binding_list = cdr(binding_list);
    }
    
    Frame *child_frame = talloc(sizeof(Frame));
    child_frame->bindings = new_binding_list;
    child_frame->parent = frame;
    
    Value *body = evalLetBody(lastArg, child_frame);
    Value *returnEvalLet = eval(car(body), child_frame);
    
    return returnEvalLet;
}

//evaluate let* statement
Value *evalLetStar(Value *args, Frame *frame){
    Value *lastArg = checkLetArgs(args, frame, 1);
    
    // evaluate the bindings
    Value *binding_list = car(args);

    Value *new_binding_list = makeNull();
    
    // each time a new binding is evaluated, updates it to temp_binding
    Value *original_binding = frame->bindings;
    Value *temp_binding = frame->bindings;
    
    while (binding_list->type != NULL_TYPE) {
        Value *cur_binding = car(binding_list);
        Value *new_binding = makeNull();

        // evaluate a new binding
        new_binding = cons(eval(car(cdr(cur_binding)), frame), new_binding);
        new_binding = cons(car(cur_binding), new_binding);
        new_binding_list = cons(new_binding, new_binding_list);
        
        temp_binding = cons(new_binding, temp_binding);
        frame->bindings = temp_binding;
        binding_list = cdr(binding_list);
    }
    
    // change the bindings in the current frame back to what its original binding
    frame->bindings = original_binding;
    
    Frame *child_frame = talloc(sizeof(Frame));
    child_frame->bindings = new_binding_list;
    child_frame->parent = frame;
    
    Value *body = evalLetBody(lastArg, child_frame);
    Value *returnEvalLet = eval(car(body), child_frame);
    
    return returnEvalLet;
}

//evaluate letrec statement
Value *evalLetRec(Value *args, Frame *frame) {
    Value *lastArg = checkLetArgs(args, frame, 1);
    
    // evaluate the bindings
    Value *binding_list = car(args);
    Value *dummy_binding_list = makeNull();
    
    //set bindings to dummy value
    while (binding_list->type != NULL_TYPE) {
        Value *cur_binding = car(binding_list);
        Value *dummy_binding = makeNull();
        Value *dummy_value = makeNull();
        dummy_value->type = STR_TYPE;
        dummy_value->s = "UNDEFINED";
        dummy_binding = cons(dummy_value, dummy_binding);
        dummy_binding = cons(car(cur_binding), dummy_binding);
        dummy_binding_list = cons(dummy_binding, dummy_binding_list);
        binding_list = cdr(binding_list);
    }
    Frame *child_frame = talloc(sizeof(Frame));
    child_frame->bindings = dummy_binding_list;
    child_frame->parent = frame;
    
    binding_list = car(args);
    Value *new_binding_list = makeNull();
    
    while (binding_list->type != NULL_TYPE) {
        Value *cur_binding = car(binding_list);
        Value *new_binding_value = eval(car(cdr(cur_binding)), child_frame);
        Value *new_binding = makeNull();
        new_binding = cons(new_binding_value, new_binding);
        new_binding = cons(car(cur_binding), new_binding);
        new_binding_list = cons(new_binding, new_binding_list);
        binding_list = cdr(binding_list);
    }
    
    child_frame->bindings = new_binding_list;
    
    Value *body = evalLetBody(lastArg, child_frame);
    Value *returnEvalLet = eval(car(body), child_frame);
    
    return returnEvalLet;
}

//evaluate quote statement
Value *evalQuote(Value *args){
    Value *numArgs = car(checkNumArgs(args));
    if (numArgs->i != 1){
        quoteError(args);
    }
    
    return car(args);
}

//evaluate define statement
Value *evalDefine(Value *args, Frame *frame){   
    //check for valid input
    Value *args_check = args;
    int i = 0;
    while (args_check->type != NULL_TYPE){
        i = i + 1;
        
        // when a variable is not a symbol type e.g. (define 10 20)
        if (i == 1 && car(args_check)->type != SYMBOL_TYPE){
            printf("bad syntax in: ");
            printInterpTree(car(args_check));
            printf("\n");
            texit(1);
        }
        args_check = cdr(args_check);
    }
    // error checking e.g. (define a)
    if (i < 2){
        printf("define: bad syntax\n(missing expression after identifier): (define ");
        printInterpTree(args);
        printf(")\n");
        texit(1);
    }
    // error checking e.g. (define a 10 20 30)
    else if (i > 2){
        printf("define: bad syntax\n(multiple expressions after identifier): (define ");
        printInterpTree(args);
        printf(")\n");
        texit(1);
    }
    
    // evaluate expression
    Value *expression =  eval(car(cdr(args)),frame);
    Value *binding = makeNull();
    binding = cons(expression, binding);
    binding = cons(car(args), binding);
    
    // let globalFrame be the parent of the current frame
    // stores var - expr bindings to globalFrame
    
    globalFrame->bindings = cons(binding, globalFrame->bindings);
    
    Value *void_ptr = makeNull();
    void_ptr->type = VOID_TYPE;
    void_ptr->p = cdr(args);
    return void_ptr;
}

/*
evaluate lambda expression

NOTE

(define function
    (lambda (x)
        (+ 10 x) (* 20 x)))
        
Only evaluates (* 20 x) for input x.
It recognizes the last expression as the body of lambda expression.
*/
Value *evalLambda(Value *args, Frame *frame){
    // check for valid input
    Value *args_check = args;
    Value *previous;
    int i = 0;
    
    Value *body = makeNull();
    
    while (args_check->type != NULL_TYPE){
        if (car(args_check)->type == CONS_TYPE) {
            if (car(car(args_check))->type != SYMBOL_TYPE) {
                printf("lambda: argument is not an identifier\n");
                texit(1);
            }
        }
        i = i + 1;
        previous = args_check;
        args_check = cdr(args_check);
        
        if (args_check->type == CONS_TYPE){
            if (car(args_check)->type == CONS_TYPE){
                if (car(car(args_check))->type == SYMBOL_TYPE){
                    if (!strcmp(car(car(args_check))->s, "set!")
                       || !strcmp(car(car(args_check))->s, "begin")){
                        body = cons(car(args_check), body);
                    }
                }
            }
        }
    }
    
    Value *parameters = car(args);
    body = cons(previous, body);
    body = reverse(body);
    // case: (lambda), (lambda (x))
    if (i < 2){
        printf("lambda: bad syntax in (lambda ");
        printInterpTree(args);
        printf(")\n");
        texit(1);
    }
    
    // creates a closure
    Value *closure = makeNull();
    closure->type = CLOSURE_TYPE;
    closure->cl.frame = frame;
    closure->cl.functionCode = body;
    closure->cl.paramNames = parameters;

    return closure;
}

// applies a function to arguments (runs body of function)
Value *apply(Value *function, Value *args) {
    // check that function is function
    if (function->type != CLOSURE_TYPE && function->type != PRIMITIVE_TYPE) {
        evaluationError();
    }
    
    // if function is primitive type, execute the function by passing args
    if (function->type == PRIMITIVE_TYPE){
        return function->pf(args);
    }
    // function type is closure type
    else{
        // create frame
        Value *binding_list = makeNull();
        Value *param_list = function->cl.paramNames;    
        Value *args_list = args;
        
        while (param_list->type != NULL_TYPE) {
            if (args_list->type == NULL_TYPE) {
                printf("too few arguments to function call\n");
                texit(1);
            }
            Value *binding = makeNull();
            binding = cons(car(args_list), binding);
            binding = cons(car(param_list), binding);
            binding_list = cons(binding, binding_list);
        
            param_list = cdr(param_list);
            args_list = cdr(args_list);
        }
        if (args_list->type != NULL_TYPE) {
            printf("too many arguments to function call\n");
            texit(1);
        }

        Frame *new_frame = talloc(sizeof(Frame));
        new_frame->bindings = binding_list;
        new_frame->parent = function->cl.frame;
        
        Value *fun_code = function->cl.functionCode;
        Value *body_ptr = fun_code;
        Value *last_body;
        
        while(body_ptr->type != NULL_TYPE){
            if (car(body_ptr)->type == CONS_TYPE){

                if (car(car(body_ptr))->type == SYMBOL_TYPE){
                    if(!strcmp(car(car(body_ptr))->s, "set!")){
                        evalSet(cdr(car(body_ptr)), new_frame);
                    }else if (!strcmp(car(car(body_ptr))->s, "begin")){
                        evalBegin(cdr(car(body_ptr)), new_frame);
                    }
                }
            }
            last_body = car(body_ptr);
            body_ptr = cdr(body_ptr);
        }
        //evaluate body of function in new frame
        return eval(car(last_body), new_frame);
    }
}

//returns a list of evaluated arguments
Value *evalEach(Value *args, Frame *frame) {
    // no args is fine
    if (args->type == NULL_TYPE) {
        return args;
    }
    // check that args is a list
    if (args->type != CONS_TYPE) {
        evaluationError();
    }
    
    Value *evaledArgs = makeNull();
    Value *args_list = args;
    
    while (args_list->type != NULL_TYPE) {
        evaledArgs = cons(eval(car(args_list), frame),evaledArgs);
        args_list = cdr(args_list);
    }

    return reverse(evaledArgs);
}

//evalates tree from the top down
Value *eval(Value *tree, Frame *frame){
    switch (tree->type){
        case INT_TYPE:{
            return tree;
            break;
        }
        case DOUBLE_TYPE:{
            return tree;
            break;
        }
        case BOOL_TYPE:{
            return tree;
            break;
        }
        case STR_TYPE:{
            return tree;
            break;
        }
        case SYMBOL_TYPE:{
            Value *result = lookUpSymbol(tree, frame, 0);
            if (result->type == CONS_TYPE){
                if(car(result)->type == SYMBOL_TYPE){
                    if (!strcmp(car(result)->s, "quote")){
                        result = cdr(result);
                    }  
                }
            }
            return result;
            break;
        }
        case CONS_TYPE:{
            Value *first = car(tree);
            Value *args = cdr(tree);
                    
            Value *result;
        
            if (first->type == SYMBOL_TYPE){
                // if (symbol == "...")
                if (!strcmp(first->s, "if")){
                    result = evalIf(args, frame);
                } else if (!strcmp(first->s, "let")){
                    result = evalLet(args, frame);
                } else if (!strcmp(first->s, "let*")){
                    result = evalLetStar(args, frame);
                } else if (!strcmp(first->s, "letrec")) {
                    result = evalLetRec(args, frame);
                } else if (!strcmp(first->s, "quote")){
                    result = evalQuote(args);
                } else if (!strcmp(first->s, "define")){
                    result = evalDefine(args, frame);
                } else if (!strcmp(first->s, "lambda")){
                    result = evalLambda(args, frame);
                } else if (!strcmp(first->s, "cond")){
                    result = evalCond(args, frame);
                } else if (!strcmp(first->s, "and")){
                    result = evalAnd(args, frame);
                } else if (!strcmp(first->s, "or")){
                    result = evalOr(args, frame);
                } else if (!strcmp(first->s, "set!")){
                    result = evalSet(args, frame);
                } else if (!strcmp(first->s, "begin")){
                    result = evalBegin(args, frame);
                } else {
                    Value *evaledOperator = eval(first, frame);
                    Value *evaledArgs = evalEach(args, frame);
                    return apply(evaledOperator, evaledArgs);
                }
            }
            
            // when directly calls lambda
            // e.g. ((lambda (x) x) 10)
            // or a procedure is used that returns another procedure
            else if (first->type == CONS_TYPE){
                Value *returned = eval(first, frame);
                if (returned->type == CLOSURE_TYPE) {
                    Value *evaledOperator = eval(returned, frame);
                    Value *evaledArgs = evalEach(args, frame);
                    return apply(evaledOperator, evaledArgs);
                } else {
                    evaluationError();
                }
            }
            
            else if (first->type == INT_TYPE || first->type == STR_TYPE ||
                    first->type == BOOL_TYPE || first->type == DOUBLE_TYPE){ 
                applicationError(tree);
            }
            
            // not a recognized special forms
            else{
                evaluationError();
            }
            return result;
            break;
        }
        case NULL_TYPE:{
            return tree;
            break;
        }
        default:{
            return tree;
            break;
        }
    }
}