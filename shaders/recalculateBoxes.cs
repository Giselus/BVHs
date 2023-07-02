#version 430 core

//Step of calculating proper bounding boxes after hierarchy is generated

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

layout (std430, binding = 2) buffer Counters{
    uint data[];
} counters;

uniform int n;

void main(){
    uint currentNodeID = gl_GlobalInvocationID.x;
    node currentNode;
    if(currentNodeID < n){
        currentNode = nodes.data[currentNodeID];

        if(currentNode.children[0] != 0)
            return;
        currentNode.box[0] = vec4(1,1,1,0);
        currentNode.box[1] = vec4(0,0,0,0);
        for(uint i = currentNode.interval[0]; i <= currentNode.interval[1]; i++){
            currentNode.box[0] = min(currentNode.box[0], triangles.data[i].box[0]);
            currentNode.box[1] = max(currentNode.box[1], triangles.data[i].box[1]);
        }

        for(;;){
            nodes.data[currentNodeID] = currentNode;
            if(currentNodeID == 0)
                break;
//            nodes.data[currentNodeID].box[0] = vec4(0.5);
            currentNodeID = currentNode.parent;
            currentNode = nodes.data[currentNodeID];

            uint val = atomicAdd(counters.data[currentNodeID], 1);
            if(val == 0){
                break;
            }

            memoryBarrierBuffer();

            node L = nodes.data[currentNode.children[0]];
            node R = nodes.data[currentNode.children[1]];

            currentNode.box[0] = min(L.box[0], R.box[0]);
            currentNode.box[1] = max(L.box[1], R.box[1]);
        }
    }
}
