#version 430 core

#define LOG (5)
#define GROUP_SIZE (1 << LOG)

//can't use define below
layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

layout (std430, binding = 1) buffer Result{
    int data[];
} R;

layout (std430, binding = 2) buffer PartialResults{
    int data[];
} P;

uniform int n;

//these are actually boolean variables but openGL doesnt support them
uniform int copyPartialSums;

void main(){
    uint id = gl_GlobalInvocationID.x;
    uint localID = gl_LocalInvocationID.x;
    return;
    if(id < n){
        R.data[id] = T.data[id];
    }else{
        R.data[id] = 0;
    }
    barrier();
    int bit = 1;
    for (int i = 0; i < LOG; i++){
        if(((localID+1) % bit) == 0 && ((localID+1) % (bit << 1)) != 0){
            R.data[id + bit] += R.data[id];
        }
        bit <<= 1;
        barrier();
    }

    if(localID == GROUP_SIZE - 1){
        R.data[id] = 0;
    }
    barrier();
    for(int i = LOG - 1; i >= 0; i--){
        if(((localID+1) % bit) == 0){
           int tmp = R.data[id];
           R.data[id] += R.data[id - (bit >> 1)];
           R.data[id - (bit >> 1)] = tmp;
        }
        bit >>= 1;
        barrier();
    }

    if((copyPartialSums == 1) && (localID == GROUP_SIZE - 1)){
        P.data[gl_WorkGroupID.x] = R.data[id] + T.data[id];
    }
}
