#version 430 core

//can't use define below
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

uniform int n;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        T.data[id] = 0;
    }
}
