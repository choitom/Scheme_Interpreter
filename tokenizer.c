/*
* Tom Choi, Kaya Govek, Jonah Tuchow
* Implementation of tokenizer
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"

/*
Vector is for storing information regarding what characters are being read
and the size of the string for dynamic memory allocation.

In case a close parenthesis is read and terminates tokenizing a double,
it stores the close parenthesis and later tokenizes the close parenthesis
seperately.

In case it encounters a decimal point while reading digits and
Vector needs to know what kind of number it is reading, it has type variable.
Once finished reading numbers, it then converts the digits read to its
corresponding type.
*/
struct Vector{
    int type; // 0 -> int, 1 -> double, 2 -> string, 3-> bool, 4-> symbol
    char endChar;
    char* str;
    int memorySize;
    int size;
};

typedef struct Vector Vector;

// Initialize a vector
Vector *init_vector(char* str_ptr,int size, int memorySize, int type){
    Vector *v = (Vector *)talloc(sizeof(Vector));
    v->size = size;
    v->memorySize = memorySize;
    v->str = str_ptr;
    v->endChar = ' ';
    v->type = type;
    return v;
}

// Double the memory size of a string in a vector
void doubleStringMemory(Vector *v){
    char *new_str = talloc(sizeof(char) * 2 * v->memorySize);
    int i;
    for (i = 0; i < v->memorySize; i++){
        new_str[i] = v->str[i];
    }
    v->str = new_str;
    v->memorySize = (v->memorySize) *2;
}

// Return 1 if a given char is a digit. Otherwise, return 0
int isDigit(char c){
    return (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' ||
            c == '6' || c == '7' || c == '8' || c == '9' || c == '0');
}

// Returns 1 if a given char is a letter. Otherwise, return 0
int isLetter(char c){
    return (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' ||
            c == 'f' || c == 'g' || c == 'h' || c == 'i' || c == 'j' ||
            c == 'k' || c == 'l' || c == 'm' || c == 'n' || c == 'o' ||
            c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' ||
            c == 'u' || c == 'v' || c == 'w' || c == 'x' || c == 'y' ||
            c == 'z' || c == 'A' || c == 'B' || c == 'C' || c == 'D' ||
            c == 'E' || c == 'F' || c == 'G' || c == 'H' || c == 'I' ||
            c == 'J' || c == 'K' || c == 'L' || c == 'M' || c == 'N' ||
            c == 'O' || c == 'P' || c == 'Q' || c == 'R' || c == 'S' ||
            c == 'T' || c == 'U' || c == 'V' || c == 'W' || c == 'X' ||
            c == 'Y' || c == 'Z');
}

// Returns 1 if initial. Otherwise, return 0
int isInitial(char c){
    return (isLetter(c) || c == '!' || c == '$' || c == '%' || c == '&' ||
            c == '*' || c == '/' || c == ':' || c == '<' || c == '=' ||
            c == '>' || c == '?' || c == '~' || c == '_' || c == '^');
}

// Returns 1 if subsequent. Otherwise, return 0
int isSubsequent(char c){
    return (isInitial(c) || isDigit(c) || c == '.' || c == '+' || c == '-');
}

// Removes comments
void removeComments(){
    char nextCharRead;
    nextCharRead = fgetc(stdin);
    while (nextCharRead != EOF && nextCharRead != '\n') {
        nextCharRead = fgetc(stdin);
    }
}

// Tokenizes a string
char *stringTokenize(){        
    char charRead;
    char nextCharRead;
    char *str = talloc(sizeof(char)*10);
    str[0] = '\"';
    str[1] = '\0';
    Vector *stringVector = init_vector(str, 2, 2, 2);
    
    do{
        charRead = fgetc(stdin);
            
        //If the next character read after a double quote is the end of the
        //input, then force quit.
        if(charRead == EOF){
            printf("String cannot be tokenized\n");
            texit(1);
        } else {
            if (charRead == '\\'){
                nextCharRead = fgetc(stdin);
                if (nextCharRead != EOF){
                    // If every memory block in str is occupied,
                    // double its memory size
                    if (((stringVector->size)+2) > (stringVector->memorySize)){
                       doubleStringMemory(stringVector);
                    }
                    
                    // Append two characters
                    stringVector->size = stringVector->size + 2;
                    stringVector->str[stringVector->size - 3] = charRead;
                    stringVector->str[stringVector->size - 2] = nextCharRead;
                    stringVector->str[stringVector->size - 1] = '\0';
                }
            } else {
                if (((stringVector->size)+2) > (stringVector->memorySize)){
                   doubleStringMemory(stringVector);
                }

                // Append a character
                stringVector->size = stringVector->size + 1;
                stringVector->str[stringVector->size - 2] = charRead;
                stringVector->str[stringVector->size - 1] = '\0';
            }
        }
    }while(charRead != '\"');
    return stringVector->str;
}

// Tokenizes any digits following a decimal point
Vector *doubleTokenize(int udecimal){
    char charRead;
    char *double_str = talloc(sizeof(char)*10);
    double_str[0] = '.';
    double_str[1] = '\0';
    Vector *doubleVector = init_vector(double_str, 2, 2, 1);
    
    // Takes care of  .<digit>+
    if (udecimal == 0){
        charRead = fgetc(stdin);
        if (charRead == EOF || !isDigit(charRead)){
            printf("Double cannot be tokenized\n");
            texit(1);
        }
    
        while (charRead != EOF && charRead != '\n' &&
               charRead != ' '){
            if (!isDigit(charRead) && charRead != ')' && charRead != '(' &&
                charRead != ';'){
                printf("Double cannot be tokenized\n");
                texit(1);
            } else if (charRead == ')'){
                doubleVector->endChar = ')';
                break;
            } else if (charRead == '('){
                doubleVector->endChar = '(';
                break;
            } else if (charRead == ';'){
                removeComments();
                break;
            }
//            else if (charRead == '.') {
//                break;
//            }
        
            //If every memory block in str is occupied, double its memory size.
            if (((doubleVector->size)+2) > doubleVector->memorySize){
                doubleStringMemory(doubleVector);
            }
        
            // Append a digit
            doubleVector->str[doubleVector->size - 1] = charRead;
            doubleVector->size = doubleVector->size + 1;
            doubleVector->str[doubleVector->size - 1] = '\0';
        
            charRead = fgetc(stdin);
        }
    }
    
    // Takes care of <digit>+ . <digit>*
    else{
        charRead = fgetc(stdin);
        
        if (charRead == EOF || charRead == ' '){
            doubleVector->str[doubleVector->size - 1] = '0';
            doubleVector->size = doubleVector->size + 1;
            doubleVector->str[doubleVector->size - 1] = '\0';
        }
        else{
            while (charRead != EOF && charRead != '\n' && charRead != ' '){
                if (!isDigit(charRead) && charRead != ')' && charRead != ';'){
                   printf("Double cannot be tokenized\n");
                    texit(1);
                }else if (charRead == ')'){
                    doubleVector->endChar = ')';
                    break;
                }else if (charRead == '('){
                    doubleVector->endChar = '(';
                    break;
                }else if (charRead == ';'){
                    removeComments();
                    break;
                }
        
                // If every memory block in str is occupied, double its
                // memory size.
                if (((doubleVector->size)+2) > doubleVector->memorySize){
                    doubleStringMemory(doubleVector);
                }
        
                // Append a digit
                doubleVector->str[doubleVector->size - 1] = charRead;
                doubleVector->size = doubleVector->size + 1;
                doubleVector->str[doubleVector->size - 1] = '\0';
        
                charRead = fgetc(stdin);
            }        
        }  
    }
    return doubleVector;
}

// Tokenizes either int or double upon reading a digit
Vector *numberTokenize(char firstDigit){
    char charRead;
    char *num_str = talloc(sizeof(char)*10);
    num_str[0] = firstDigit;
    num_str[1] = '\0';
    
    Vector *numVector = init_vector(num_str, 2, 2, 0);
    
    do{
        charRead = fgetc(stdin);
        
        // If a digit is read, append it
        if (isDigit(charRead)){
            if (((numVector->size)+2) > numVector->memorySize){
                doubleStringMemory(numVector);
            }
            numVector->str[numVector->size - 1] = charRead;
            numVector->size = numVector->size + 1;
            numVector->str[numVector->size - 1] = '\0';
        }
        
        // A decimal point is read
        else if (charRead == '.'){
            numVector->type = 1;
            Vector *doubleVector = doubleTokenize(1);
            int mem_size = (doubleVector->memorySize)+(numVector->memorySize);
            char *double_str = talloc(sizeof(char) * mem_size);
            strcpy(double_str, numVector->str);
            strcat(double_str, doubleVector->str);
             
            doubleVector->str = double_str;
            doubleVector->memorySize = (mem_size);
            numVector = doubleVector;
            
            break;
        }
        
        // Store the close parenthesis and exit
        else if (charRead == ')'){
            numVector->endChar = ')';
            break;
        } else if (charRead == '('){
            numVector->endChar = '(';
            break;
        } else if (charRead == ';'){
            removeComments();
            break;
        }
        
        else if (!isDigit(charRead) && charRead != EOF && charRead != '\n'
                 && charRead != ' ' && charRead != ';'){
            printf("Cannot tokenize a number\n");
            texit(1);
        }     
    }while (charRead != EOF && charRead != '\n' && charRead != ' ');
    
    return numVector;
}

// Invoke doubleTokenize or numberTokenize depending on a character read
Vector *tokenizeDigits(Value *ptr, char charRead, char sign){
    Vector *digit_vec;
    
    // Decimal point (double)
    if (charRead == '.'){
        digit_vec = doubleTokenize(0);
        double double_num = atof(digit_vec->str);
        if (sign == '-'){
            double_num = double_num * -1.0;
        }
        ptr->type = DOUBLE_TYPE;
        ptr->d = double_num;
    }
        
    // Digit (int or double)
    else if (isDigit(charRead)){
        digit_vec = numberTokenize(charRead);
        // Integer
        if (digit_vec->type == 0){
            int integer = atoi(digit_vec->str);
            if (sign == '-'){
                integer = integer * -1;
            }
            ptr->type = INT_TYPE;
            ptr->i = integer;
        }
        // Double
        else{
            double double_num = atof(digit_vec->str);
            if (sign == '-'){
                double_num = double_num * -1;
            }
            ptr->type = DOUBLE_TYPE;
            ptr->d = double_num;
        }
    }
    if (digit_vec->endChar == ';'){
        removeComments();
    }
    return digit_vec;
}

// Tokenizes bool
Vector *tokenizeBool(){
    char charRead;
    char nextCharRead;
    char *bool_str = talloc(sizeof(char)*3);
    bool_str[0] = '#';
    bool_str[1] = '\0';
    
    Vector *boolVector= init_vector(bool_str, 2, 2, 3);
    
    charRead = fgetc(stdin);
    nextCharRead = fgetc(stdin);
        
    if ((charRead == 't' || charRead == 'T' || charRead == 'f' ||
        charRead == 'F') && (nextCharRead == ' ' || nextCharRead == EOF ||
                             nextCharRead == '\n' || nextCharRead == ';' ||
                            nextCharRead == ')' || nextCharRead == '(')){
        boolVector->str[boolVector->size - 1] = charRead;
        boolVector->size = boolVector->size + 1;
        boolVector->str[boolVector->size - 1] = '\0';
    }else{
        printf("A bool type cannot be tokenized\n");
        texit(1);
    }
    // Store the close parenthesis
    if (nextCharRead == ')'){
        boolVector->endChar = ')';
    } else if (nextCharRead == '('){
        boolVector->endChar = '(';
    } else if (nextCharRead == ';'){
        removeComments();
    }
    return boolVector;
}

// Tokenizes symbols
Vector *tokenizeSymbol(char c1, char c2){
    char charRead;
    char *symbol_str = talloc(sizeof(char)*10);
    symbol_str[0] = c1;
    symbol_str[1] = c2;
    symbol_str[2] = '\0';
    
    Vector *symbolVector = init_vector(symbol_str, 3, 3, 4);
    
    charRead = fgetc(stdin);
    while(charRead != EOF && charRead != '\n' && charRead != ' '){  
        if (!isSubsequent(charRead) && charRead != ')'
            && charRead != '(' && charRead != ';'){
            printf("Symbol cannot be tokenized\n");
            texit(1);
        }else if (charRead == ')'){
            symbolVector->endChar = ')';
            break;
        }else if (charRead == '('){
            symbolVector->endChar ='(';
            break;
        }else if (charRead == ';'){
            symbolVector->endChar = ';';
            break;
        }
        if (symbolVector->size + 2 > symbolVector->memorySize){
            doubleStringMemory(symbolVector);
        }
        symbolVector->str[symbolVector->size - 1] = charRead;
        symbolVector->size = symbolVector->size + 1;
        symbolVector->str[symbolVector->size - 1] = '\0';
        charRead = fgetc(stdin);
    }
    return symbolVector;
}

// Tokenizes the input text file in accordance with Scheme grammar
Value *tokenize(){
    char charRead;
    char nextCharRead;
    Value *list = makeNull();
    charRead = fgetc(stdin);
    
    while (charRead != EOF){
        Value *ptr = talloc(sizeof(Value));
        
        // Symbol
        if (isInitial(charRead) || charRead == '+' || charRead == '-'){
            nextCharRead = fgetc(stdin);
            char *str_holder;
            Vector *vec = init_vector(str_holder,0,0,NULL_TYPE);
            
            // Tells whether +, - is a sign or an identifier
            if ((charRead == '+' || charRead == '-') &&
                (isDigit(nextCharRead) || nextCharRead == '.')){
                vec = tokenizeDigits(ptr, nextCharRead, charRead);
                list = cons(ptr, list);
                Value *ptr2 = talloc(sizeof(Value));
                if (vec->endChar == ')'){
                    ptr2->type = CLOSE_TYPE;
                    list = cons(ptr2, list);
                }else if(vec->endChar=='('){
                    ptr2->type = OPEN_TYPE;
                    list = cons(ptr2, list);
                }else if(vec->endChar==';'){
                    removeComments();
                }
            }
            // Tokenize symbols in accordance with the given symbol grammar
            else if (nextCharRead != ' ' && nextCharRead != EOF &&
                     nextCharRead != ')' && nextCharRead != '(' &&
                     nextCharRead != '\n' && nextCharRead != ';'){
                if (charRead == '+' || charRead == '-') {
                    printf("Invalid symbol\n");
                    texit(1);
                }
                vec = tokenizeSymbol(charRead, nextCharRead);
                ptr->type = SYMBOL_TYPE;
                ptr->s = vec->str;
                list = cons(ptr, list);
                
                Value *ptr2 = talloc(sizeof(Value));
                if (vec->endChar == ')'){
                    ptr2->type = CLOSE_TYPE;
                    list = cons(ptr2, list);
                }else if(vec->endChar=='('){
                    ptr2->type = OPEN_TYPE;
                    list = cons(ptr2, list);
                }else if(vec->endChar==';'){
                    removeComments();
                }
            }
             // When symbol length is 1 e.g. >, +, a
            else{
                ptr->type = SYMBOL_TYPE;
                char *str = talloc(sizeof(char)*5);
                str[0] = charRead;
                str[1] = '\0';
                ptr->s = str;
                list = cons(ptr, list);
                Value *ptr2 = talloc(sizeof(Value));
                if (nextCharRead == ')'){    
                    ptr2->type = CLOSE_TYPE;
                    list = cons(ptr2, list);
                }else if(nextCharRead =='('){
                    ptr2->type = OPEN_TYPE;
                    list = cons(ptr2, list);
                }else if(nextCharRead ==';'){
                    removeComments();
                }
            }
        }
        
        // Remove line comments
        else if (charRead == ';'){
            removeComments();
        }
        
        // Open parenthesis
        else if (charRead == '('){
            ptr->type = OPEN_TYPE;
            list = cons(ptr, list);
        }
        
        // Close parenthesis
        else if (charRead == ')'){
            ptr->type = CLOSE_TYPE;
            list = cons(ptr, list);
        }
        
        // Double quote (string)
        else if (charRead == '\"'){
            char *str = stringTokenize();
            ptr->type = STR_TYPE;
            ptr->s = str;
            list = cons(ptr, list);
        }
          
        // Single quote (string, int, double, or symbol)
        else if (charRead == '\''){
            nextCharRead = fgetc(stdin);
            
            // string
            if (nextCharRead == '\"'){
                char *str = stringTokenize();
                ptr->type = STR_TYPE;
                ptr->s = str;
                list = cons(ptr, list);
            }
            
            // symbol
            else if (nextCharRead == '('){
                ptr->type = SYMBOL_TYPE;
                ptr->s = "'";
                
                Value *ptr2 = talloc(sizeof(Value));
                ptr2->type = OPEN_TYPE;
                
                list = cons(ptr, list);
                list = cons(ptr2, list);
            }
            
            // int or double
            else if (isDigit(nextCharRead) || nextCharRead =='+' ||
                     nextCharRead == '-' || nextCharRead == '.'){
                
                // '.12, '12, '12.34
                if (nextCharRead == '.' || isDigit(nextCharRead)){
                    Vector *num_vec;
                    num_vec = tokenizeDigits(ptr, nextCharRead, '+');
                    
                    list = cons(ptr, list);
                    
                    Value *ptr2 = talloc(sizeof(Value));
                    if (num_vec->endChar == ')'){
                        ptr2->type = CLOSE_TYPE;
                        list = cons(ptr2, list);
                    } else if (num_vec->endChar == '('){
                        ptr2->type = OPEN_TYPE;
                        list = cons(ptr2, list);
                    }
                }
                // '+123, '-123, '-12.34, '+12.34, '-.12
                else{
                    char sign = nextCharRead;
                    nextCharRead = fgetc(stdin);
                    
                    if (isDigit(nextCharRead) || nextCharRead == '.'){
                        Vector *vec;
                        vec = tokenizeDigits(ptr, nextCharRead, sign);
                        list = cons(ptr, list);
                        Value *ptr2 = talloc(sizeof(Value));
                        if (vec->endChar == ')'){
                            ptr2->type = CLOSE_TYPE;
                            list = cons(ptr2, list);
                        } else if (vec->endChar == '('){
                            ptr2->type = OPEN_TYPE;
                            list = cons(ptr2, list);
                        }
                    }else{
                        printf("Single quote cannot be tokenized\n");
                        texit(1);
                    }
                }
            }else{
                printf("Single quote cannot be tokenized\n");
                texit(1);
            }
        }
        
        // Decimal point or a digit (int or double)
        else if (charRead == '.' || isDigit(charRead)){
            Vector *num_vec;
            num_vec = tokenizeDigits(ptr, charRead, '+');
            list = cons(ptr, list);
            
            if (num_vec->endChar == ')'){
                Value *ptr2 = talloc(sizeof(Value));
                ptr2->type = CLOSE_TYPE;
                list = cons(ptr2, list);
            } else if (num_vec->endChar == '('){
                Value *ptr2 = talloc(sizeof(Value));
                ptr2->type = OPEN_TYPE;
                list = cons(ptr2, list);
            } else if (num_vec->endChar == ';'){
                removeComments();
            }
        }
                
        // Hash tag (bool)
        else if (charRead == '#'){
            Vector *boolVector = tokenizeBool();
            ptr->type = BOOL_TYPE;
            ptr->s = boolVector->str;
            list = cons(ptr, list);
            
            if (boolVector->endChar == ')'){
                Value *ptr2 = talloc(sizeof(Value));
                ptr2->type = CLOSE_TYPE;
                list = cons(ptr2, list);
            } else if (boolVector->endChar == '('){
                Value *ptr2 = talloc(sizeof(Value));
                ptr2->type = OPEN_TYPE;
                list = cons(ptr2, list);
            }
        }
        charRead = fgetc(stdin);
    }
    Value *revList  = reverse(list);
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
    switch(list->type){
        case(NULL_TYPE):
            break;
        case(CONS_TYPE):
            displayTokens(list->c.car);
            displayTokens(list->c.cdr);
            break;
        case(OPEN_TYPE):
            printf("( : open\n");
            break;
        case(CLOSE_TYPE):
            printf(") : close\n");
            break;
        case(STR_TYPE):
            printf("%s : string\n", list->s);
            break;
        case(INT_TYPE):
            printf("%d : int\n", list->i);
            break;
        case(DOUBLE_TYPE):
            printf("%lf : double\n", list->d);
            break;
        case(BOOL_TYPE):
            printf("%s : bool\n", list->s);
            break;
        case(SYMBOL_TYPE):
            printf("%s : symbol\n", list->s);
            break;
        case(PTR_TYPE):
            break;
        default:
            break;
    }
}