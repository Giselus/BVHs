#version 430 core

#define MAX_INT 2147483647

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

struct triangle{
    vec4 position[3];
    vec4 color;
    vec4 center;
    vec4 box[2];
    int code;
};

struct pair{
    uint x;
    int y;
    int z;
};

layout (std430, binding = 0) buffer Input{
    triangle data[];
} T;

layout (std430, binding = 1) buffer Result{
    triangle data[];
} R;

uniform int n;
uniform int chunkSize;

shared pair first_chunk[gl_WorkGroupSize.x];
shared pair second_chunk[gl_WorkGroupSize.x];

shared int reduce_array[gl_WorkGroupSize.x];

shared triangle first_swap_buf[gl_WorkGroupSize.x];

//reduces array, final result is in reduce_array[0]
void reduce(){
    uint localID = gl_LocalInvocationID.x;
    uint LOG = uint(log2(gl_WorkGroupSize.x));
    uint mask;
    for(int i = 1; i <= LOG; i++){
        mask = (1 << i) - 1;
        if((localID & mask) == (1 << (i-1)))
            reduce_array[localID - (1 << (i-1))] += reduce_array[localID];
        barrier();
        memoryBarrierShared();
    }
}

void main(){
    uint id = gl_GlobalInvocationID.x;
    uint localID = gl_LocalInvocationID.x;
    uint LOG = uint(log2(gl_WorkGroupSize.x));
    uint HALF_GROUP_SIZE = gl_WorkGroupSize.x / 2;

    uint offset = gl_WorkGroupID.x * chunkSize * 2;
    uint first_it = offset;
    uint first_block_end = min(n,first_it + chunkSize);
    //we assume that both block are not empty
    uint second_it = first_block_end;
    uint second_block_end = min(n, second_it + chunkSize);

    uint steps = (second_block_end - first_it + gl_WorkGroupSize.x - 1) / gl_WorkGroupSize.x;

    uint group;
    uint in_group_id;
    uint mask;

    uint first_el;
    uint second_el;
    pair tmp;
    for(int i = 0; i < steps; i++){
        first_chunk[localID].y = 1;
        if(first_it + localID < first_block_end){
            first_chunk[localID].x = first_it + localID;
            first_chunk[localID].z = T.data[first_it + localID].code;
        }else{
            first_chunk[localID].x = -1;
            first_chunk[localID].z = MAX_INT;
        }
        second_chunk[localID].y = 2;
        if(second_it + gl_WorkGroupSize.x - localID - 1 < second_block_end){
            second_chunk[localID].x = second_it + gl_WorkGroupSize.x - localID - 1;
            second_chunk[localID].z = T.data[second_it + gl_WorkGroupSize.x - localID - 1].code;
        }else{
            second_chunk[localID].x = -1;
            second_chunk[localID].z = MAX_INT;
        }
        barrier();
        memoryBarrierShared();
        if(first_chunk[localID].z <= second_chunk[localID].z){
            reduce_array[localID] = 1;
        }else{
            reduce_array[localID] = 0;
            first_chunk[localID] = second_chunk[localID];
        }
        barrier();
        memoryBarrierShared();
        //sort first_chunk using bitonic sorter
        uint h = HALF_GROUP_SIZE;
//        uint h = 1;
        for(int j = int(LOG) - 1; j >= 0; j--){
            if(localID < HALF_GROUP_SIZE){
                mask = h - 1;
                group = localID >> j;
                in_group_id = localID & mask;
                first_el = h * group * 2 + in_group_id;
                second_el = first_el + h;
                if(first_chunk[first_el].z > first_chunk[second_el].z){
                    tmp = first_chunk[first_el];
                    first_chunk[first_el] = first_chunk[second_el];
                    first_chunk[second_el] = tmp;
                }
                h /= 2;
            }
            barrier();
            memoryBarrierShared();
        }
        barrier();
        memoryBarrierShared();
        if(first_chunk[localID].x != -1){
            if(first_chunk[localID].y == 1){
                first_swap_buf[localID] = T.data[first_chunk[localID].x];
            }
        }
        barrier();
        memoryBarrierShared();
        if(first_chunk[localID].x != -1){
            if(first_chunk[localID].y == 2){
                first_swap_buf[localID] = T.data[first_chunk[localID].x];
            }
        }
        barrier();
        memoryBarrierShared();
        if(offset + localID < n){
            R.data[offset + localID] = first_swap_buf[localID];
        }
        offset += gl_WorkGroupSize.x;
        //increment iterators
        reduce();
        first_it += reduce_array[0];
        second_it += gl_WorkGroupSize.x - reduce_array[0];
        barrier();
        memoryBarrierShared();
    }
}
