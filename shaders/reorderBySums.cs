#version 430 core

#define LOG (10)
#define GROUP_SIZE (1 << LOG)

//can't use define below
layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

layout (std430, binding = 1) buffer Result{
    int data[];
} R;

layout (std430, binding = 2) buffer PrefixSums{
    int data[];
} P;

uniform int n;
uniform int zeroCount;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        uint destination = 0;
        if(id == 0){
            if(P.data[id] == 1)
                destination = zeroCount;
            else
                destination = 0;
        }else{
            if(P.data[id] == P.data[id-1] + 1)
                destination = zeroCount + P.data[id] - 1;
            else
                destination = id - P.data[id];
        }
        R.data[destination] = T.data[id];
    }
}
