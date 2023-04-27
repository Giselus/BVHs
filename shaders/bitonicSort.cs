#version 430 core

#define MAX_INT 2147483647

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

layout (std430, binding = 1) buffer Result{
    int data[];
} R;

uniform int n;
uniform int chunkSize;

shared int first_chunk[gl_WorkGroupSize.x];
shared int second_chunk[gl_WorkGroupSize.x];

shared int reduce_array[gl_WorkGroupSize.x];

//sums array, final result is in reduce_array[0]
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
    int tmp;
    for(int i = 0; i < steps; i++){
        if(first_it + localID < first_block_end)
            first_chunk[localID] = T.data[first_it + localID];
        else
            first_chunk[localID] = MAX_INT;

        if(second_it + gl_WorkGroupSize.x - localID - 1 < second_block_end)
            second_chunk[localID] = T.data[second_it + gl_WorkGroupSize.x - localID - 1];
        else
            second_chunk[localID] = MAX_INT;
        barrier();
        memoryBarrierShared();

        if(first_chunk[localID] <= second_chunk[localID]){
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
                if(first_chunk[first_el] > first_chunk[second_el]){
                    tmp = first_chunk[first_el];
                    first_chunk[first_el] = first_chunk[second_el];
                    first_chunk[second_el] = tmp;
                }
                h /= 2;
            }
            barrier();
            memoryBarrierShared();
        }

        //save data
        if(offset + localID < n)
            R.data[offset + localID] = first_chunk[localID];
        offset += gl_WorkGroupSize.x;
        //increment iterators
        reduce();
        first_it += reduce_array[0];
        second_it += gl_WorkGroupSize.x - reduce_array[0];
        barrier();
        memoryBarrierShared();
    }
}
