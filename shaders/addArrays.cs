#version 430 core

#define LOG (10)
#define GROUP_SIZE (1 << LOG)

//can't use define below
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

layout (std430, binding = 1) buffer ToAdd{
    int data[];
} P;

uniform int n;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        T.data[id] += P.data[id];
    }
}
