#version 430 core

//Compacts active queue, shifting elements to the left to eliminate empty positions
//Gets prefix sums of values from preCompaction.cs to know positions of every object

#define SMALL_THRESHOLD (32)

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct node{
    vec4 box[2];
    uint children[2];
    uint interval[2];
    uint parent;
};

struct possibleSplit{
    uint interval[2];
    uint parent;
};

layout (std430, binding = 0) buffer CurrentQueue{
    possibleSplit data[];
} currentQueue;

layout (std430, binding = 1) buffer CompactedQueue{
    uint data[];
} compactedQueue;

layout (std430, binding = 2) buffer SmallQueue{
    uint data[];
} smallQueue;

layout (std430, binding = 3) buffer PrefixSum{
    int data[];
} prefixSum;

layout (std430, binding = 4) buffer SmallQueuePrefixSum{
    int data[];
} smallQueuePrefixSum;

layout (std430, binding = 5) buffer Nodes{
    node data[];
} nodes;

uniform int lastID;
uniform int smallSplitQueueSize;
uniform int n;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        possibleSplit tmp = currentQueue.data[id];

        //NULL value equivalent
        if(tmp.interval[1] < tmp.interval[0]){
            return;
        }

        uint nodeID = lastID + prefixSum.data[id] + smallQueuePrefixSum.data[id];

        nodes.data[nodeID].interval[0] = tmp.interval[0];
        nodes.data[nodeID].interval[1] = tmp.interval[1];
        nodes.data[nodeID].parent = tmp.parent;

        if(tmp.interval[1] - tmp.interval[0] + 1 <= SMALL_THRESHOLD){
            smallQueue.data[smallSplitQueueSize + smallQueuePrefixSum.data[id] - 1] = nodeID;
        }else{
            compactedQueue.data[prefixSum.data[id] - 1] = nodeID;
        }
    }
}
