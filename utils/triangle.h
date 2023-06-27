#ifndef TRIANGLE_H
#define TRIANGLE_H


struct triangle{
    float position[12];
    float color[4];
    float center[4];
    float box[8];
    int code; int __padding[3];
}; //128 bytes

struct node{
    float box[8];
    unsigned int children[2];
    unsigned int interval[2];
    unsigned int father;
    unsigned int __padding[3];
}; //64 bytes

struct possibleSplit{
    unsigned int interval[2];
    unsigned int father;
}; //12 bytes

#endif // TRIANGLE_H
