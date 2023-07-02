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

layout (std430, binding = 1) buffer Triangles{
    triangle data[];
} triangles;

layout (std430, binding = 2) buffer AllNodesPrefixSum{
    uint data[];
} allNodesPrefixSum;

layout (std430, binding = 3) buffer ActiveNodesPrefixSum{
    uint data[];
} activeNodesPrefixSum;

uniform int n;

void main(){
    uint nodeID = gl_GlobalInvocationID.x;
    if(nodeID < 2 * n - 1){
        nodes.data[nodeID].children[0] = 0;
        nodes.data[nodeID].children[1] = 0;
    }
    if(nodeID >= n-1){
        allNodesPrefixSum.data[nodeID] = 0;
        activeNodesPrefixSum.data[nodeID] = 0;
        return;
    }
    uint l = nodes.data[nodeID].interval[0];
    uint r = nodes.data[nodeID].interval[1];

    bool validNode = true;

    if(l != 0 && triangles.data[l].code == triangles.data[l-1].code){
        validNode = false;
    }

    if(r != n-1 && triangles.data[r].code == triangles.data[r+1].code){
        validNode = false;
    }

//    if(triangles.data[l].code != triangles.data[r].code){
//        validNode = false;
//    }

    if(validNode){
        allNodesPrefixSum.data[nodeID] = 1;
        if(triangles.data[l].code == triangles.data[r].code){
            activeNodesPrefixSum.data[nodeID] = 1;
        }else{
            activeNodesPrefixSum.data[nodeID] = 0;
        }
    }else{
        allNodesPrefixSum.data[nodeID] = 0;
        activeNodesPrefixSum.data[nodeID] = 0;
    }
}
