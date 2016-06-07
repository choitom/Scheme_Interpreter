#include "value.h"

#ifndef _PRIMITIVES
#define _PRIMITIVES

Value *primitiveAdd(Value *args);
Value *primitiveSubtract(Value *args);
Value *primitiveMult(Value *args);
Value *primitiveDivide(Value *args);
Value *primitiveModulo(Value *args);
Value *primitiveNull(Value *args);
Value *primitiveCdr(Value *args);
Value *primitiveCar(Value *args);
Value *primitiveCons(Value *args);
Value *primitiveEqual(Value *args);
Value *primitiveGreater(Value *args);
Value *primitiveGreaterEqual(Value *args);
Value *primitiveLess(Value *args);
Value *primitiveLessEqual(Value *args);

#endif