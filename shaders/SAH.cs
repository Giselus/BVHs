#version 430 core

//SAH algorithm with 256 sampled planes for each split.

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

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

struct possibleSplit{
    uint interval[2];
    uint parent;
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

uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

uint rand(uint x, uint y, uint seed){
    return x + wang_hash(seed)%(y-x+1);
}

shared triangle triangle_batch[gl_WorkGroupSize.x];

struct triple{
    float cost;
    uint dim;
    float splitPos;
};

shared triple reduce_array[gl_WorkGroupSize.x];

void reduce(){
    uint localID = gl_LocalInvocationID.x;
    uint LOG = uint(log2(gl_WorkGroupSize.x));
    uint mask;
    for(int i = 1; i <= LOG; i++){
        mask = (1 << i) - 1;
        if((localID & mask) == (1 << (i-1))){
            if(reduce_array[localID].cost < reduce_array[localID - (1 << (i-1))].cost){
                reduce_array[localID - (1 << (i-1))] = reduce_array[localID];
            }
        }
        barrier();
        memoryBarrierShared();
    }
}

shared uint prefix_sum_array[gl_WorkGroupSize.x];

void prefixSum(){
    uint localID = gl_LocalInvocationID.x;
    uint LOG = uint(log2(gl_WorkGroupSize.x));
    uint source;
    uint destination;
    uint mask;

    for(int i = 0; i < LOG; i++){
        if(localID < gl_WorkGroupSize.x / 2){
            mask = (1 << i) - 1;
            source = ((localID >> i) << (i + 1)) | mask;
            destination = source + (localID & mask) + 1;
            prefix_sum_array[destination] += prefix_sum_array[source];
        }
        barrier();
        memoryBarrierShared();
    }
}

uniform int ss;

void main(){
    uint localID = gl_LocalInvocationID.x;
    uint splitID = gl_WorkGroupID.x;

    uint nodeID = currentQueue.data[splitID];
    node w = nodes.data[nodeID];

    uint l = w.interval[0];
    uint r = w.interval[1];

    uint dim = rand(0, 2, localID);
    float splitPos = triangles.data[rand(l,r,localID)].center[dim];

    vec4 leftBox[2];
    vec4 rightBox[2];
    vec4 currentBox[2];

    leftBox[0] = vec4(1,1,1,0);
    leftBox[1] = vec4(0,0,0,0);

    rightBox[0] = vec4(1,1,1,0);
    rightBox[1] = vec4(0,0,0,0);

    currentBox[0] = vec4(1,1,1,0);
    currentBox[1] = vec4(0,0,0,0);

    uint leftElements = 0;
    uint rightElements = 0;

    for(uint offset = l; offset <= r; offset+=gl_WorkGroupSize.x){
        if(offset + localID <= r)
            triangle_batch[localID] = triangles.data[offset + localID];

        barrier();
        memoryBarrierShared();

        for(uint k = 0; k < gl_WorkGroupSize.x && offset + k <= r; k++){
            if(triangle_batch[k].center[dim] <= splitPos){
                leftBox[0] = min(leftBox[0], triangle_batch[k].box[0]);
                leftBox[1] = max(leftBox[1], triangle_batch[k].box[1]);
                leftElements++;
            }else{
                rightBox[0] = min(rightBox[0], triangle_batch[k].box[0]);
                rightBox[1] = max(rightBox[1], triangle_batch[k].box[1]);
                rightElements++;
            }
            currentBox[0] = min(currentBox[0], triangle_batch[k].box[0]);
            currentBox[1] = max(currentBox[1], triangle_batch[k].box[1]);
        }

        barrier();
        memoryBarrierShared();
    }

    float leftArea = 1;
    float rightArea = 1;
    float fullArea = 1;
    for(int i = 0; i < 3; i++){
        leftArea *= leftBox[1][i] - leftBox[0][i];
        rightArea *= rightBox[1][i] - rightBox[0][i];
        fullArea *= currentBox[1][i] - currentBox[0][i];
    }

    float cost = 0;
    //if one side of the split is empty, cost will be equal to n * full area, which shouldn't be minimal
    cost = leftArea * leftElements + rightArea * rightElements;

    reduce_array[localID] = triple(cost,dim,splitPos);

    barrier();
    memoryBarrierShared();
    reduce();
    barrier();
    memoryBarrierShared();

    dim = reduce_array[0].dim;
    splitPos = reduce_array[0].splitPos;
    cost = reduce_array[0].cost;

    uint onLeftLocal = 0;
    uint onRightLocal = 0;
    for(uint offset = l; offset <= r; offset+=gl_WorkGroupSize.x){
        if(offset + localID <= r){
            if(triangles.data[offset + localID].center[dim] <= splitPos)
                onLeftLocal++;
            else
                onRightLocal++;
        }
    }

    prefix_sum_array[localID] = onLeftLocal;

    barrier();
    memoryBarrierShared();
    prefixSum();
    barrier();
    memoryBarrierShared();

    uint leftOffset;
    uint rightOffset;
    leftOffset = prefix_sum_array[localID] - onLeftLocal;
    rightOffset = prefix_sum_array[gl_WorkGroupSize.x - 1];

    uint onLeftGlobal = prefix_sum_array[gl_WorkGroupSize.x - 1];

    barrier();
    memoryBarrierShared();

    prefix_sum_array[localID] = onRightLocal;

    barrier();
    memoryBarrierShared();
    prefixSum();
    barrier();
    memoryBarrierShared();

    rightOffset += prefix_sum_array[localID] - onRightLocal;
    uint onRightGlobal = prefix_sum_array[gl_WorkGroupSize.x - 1];

    //reorder
    for(uint offset = l; offset <= r; offset += gl_WorkGroupSize.x){
        if(offset + localID <= r){
            triangle tmp = triangles.data[offset + localID];
            if(tmp.center[dim] <= splitPos){
                trianglesTmp.data[l + leftOffset] = tmp;
                leftOffset++;
            }else{
                trianglesTmp.data[l + rightOffset] = tmp;
                rightOffset++;
            }
        }
    }

    barrier();
    memoryBarrierShared();
    memoryBarrierBuffer();

    for(uint offset = l; offset <= r; offset += gl_WorkGroupSize.x){
        if(offset + localID <= r){
            triangles.data[offset + localID] = trianglesTmp.data[offset + localID];
        }
    }

    //add new splits to queue

    if(localID == 0){
        if(onLeftGlobal == 0 || onRightGlobal == 0){
            nextQueue.data[splitID * 2].interval[0] = 1;
            nextQueue.data[splitID * 2].interval[1] = 0;

            nextQueue.data[splitID * 2 + 1].interval[0] = 1;
            nextQueue.data[splitID * 2 + 1].interval[1] = 0;
        }else{
            nextQueue.data[splitID * 2].interval[0] = l;
            nextQueue.data[splitID * 2].interval[1] = l + onLeftGlobal - 1;
            nextQueue.data[splitID * 2].parent = nodeID;

            nextQueue.data[splitID * 2 + 1].interval[0] = l + onLeftGlobal;
            nextQueue.data[splitID * 2 + 1].interval[1] = r;
            nextQueue.data[splitID * 2 + 1].parent = nodeID;
        }
    }
}
