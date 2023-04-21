#ifndef MATH_H
#define MATH_H

int ceiling(int x, int y){
    if(x % y == 0)
        return x/y;
    else
        return (x/y)+1;
}

int nextMultiple(int x, int y){
    return ceiling(x,y) * y;
}

#endif // MATH_H
