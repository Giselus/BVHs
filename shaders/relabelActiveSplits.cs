#version 430 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct triangle{
    vec4 position[3];
    vec4 color;
    vec4 center;
    vec4 box[2];
    int code;
};

struct node{
    vec4 box[2];
    uint children[2];
    uint interval[2];
    uint parent;
};

layout (std430, binding = 0) buffer Nodes{
    node data[];
} nodes;

layout (std430, binding = 1) buffer NodesSwap{
    node data[];
} nodesSwap;

layout (std430, binding = 2) buffer Triangles{
    triangle data[];
} triangles;

layout (std430, binding = 3) buffer AllNodesPrefixSum{
    uint data[];
} allNodesPrefixSum;

layout (std430, binding = 4) buffer ActiveNodesPrefixSum{
    uint data[];
} activeNodesPrefixSum;

layout (std430, binding = 5) buffer Queue{
    uint data[];
} queue;

uniform int n;

void main(){
    uint nodeID = gl_GlobalInvocationID.x;
    if(nodeID == 0){
        nodesSwap.data[0] = nodes.data[0];
        return;
    }
    if(nodeID >= n-1){
        return;
    }

    if(allNodesPrefixSum.data[nodeID] != allNodesPrefixSum.data[nodeID-1]){
        uint newID = allNodesPrefixSum.data[nodeID] - 1;

        nodesSwap.data[newID] = nodes.data[nodeID];
        nodesSwap.data[newID].parent = allNodesPrefixSum.data[nodes.data[nodeID].parent] - 1;
        if(activeNodesPrefixSum.data[nodeID] != activeNodesPrefixSum.data[nodeID-1]){
            queue.data[activeNodesPrefixSum.data[nodeID] - 1] = newID;
        }
    }
}
