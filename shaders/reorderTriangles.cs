#version 430 core

layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

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
    uint father;
};

struct possibleSplit{
    uint interval[2];
    uint father;
};

layout (std430, binding = 0) buffer Nodes{
    node data[];
} nodes;

layout (std430, binding = 1) buffer Triangles{
    triangle data[];
} triangles;

layout (std430, binding = 2) buffer TrianglesTmp{
    triangle data[];
} trianglesTmp;

layout (std430, binding = 3) buffer CurrentQueue{
    uint data[];
} currentQueue;

layout (std430, binding = 4) buffer NextQueue{
    possibleSplit data[];
} nextQueue;

void main(){
    uint localID = gl_LocalInvocationID.x;
    uint splitID = gl_WorkGroupID.x;

    uint nodeID = currentQueue.data[splitID];
    node w = nodes.data[nodeID];

    uint l = w.interval[0];
    uint r = w.interval[1];

    for(uint offset = l; offset <= r; offset += gl_WorkGroupSize.x){
        if(offset + localID <= r){
            triangles.data[offset + localID] = trianglesTmp.data[offset + localID];
        }
    }
}
