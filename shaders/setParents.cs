#version 430 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct node{
    vec4 box[2];
    uint children[2];
    uint interval[2];
    uint parent;
};

layout (std430, binding = 0) buffer Nodes{
    node data[];
} nodes;

layout (std430, binding = 1) buffer Counters{
    uint data[];
} counters;

uniform int n;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n && id != 0){
        uint parent = nodes.data[id].parent;
        uint val = atomicAdd(counters.data[parent], 1);
        nodes.data[parent].children[val] = id;
    }
}
