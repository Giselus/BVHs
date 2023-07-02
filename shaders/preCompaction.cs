#version 430 core

#define SMALL_THRESHOLD (32)

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct possibleSplit{
    uint interval[2];
    uint parent;
};

layout (std430, binding = 0) buffer CurrentQueue{
    possibleSplit data[];
} currentQueue;

layout (std430, binding = 1) buffer PrefixSum{
    int data[];
} prefixSum;

layout (std430, binding = 2) buffer SmallQueuePrefixSum{
    int data[];
} smallQueuePrefixSum;

uniform int n;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        possibleSplit tmp = currentQueue.data[id];

        //NULL value equivalent
        if(tmp.interval[1] < tmp.interval[0]){
            smallQueuePrefixSum.data[id] = 0;
            prefixSum.data[id] = 0;
            return;
        }

        if(tmp.interval[1] - tmp.interval[0] + 1 <= SMALL_THRESHOLD){
            smallQueuePrefixSum.data[id] = 1;
            prefixSum.data[id] = 0;
        }else{
            smallQueuePrefixSum.data[id] = 0;
            prefixSum.data[id] = 1;
        }
    }
}
