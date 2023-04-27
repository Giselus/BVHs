#version 430 core

#define MAX_INT 2147483647

//can't use define below
layout (local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

uniform int n;
uniform int bits;

shared int pref_in[gl_WorkGroupSize.x * 2];
shared int pref_out[gl_WorkGroupSize.x * 2];

void prefix_sum(){
    uint id = gl_GlobalInvocationID.x;
    uint localID = gl_LocalInvocationID.x;

    pref_out[localID * 2] = pref_in[localID * 2];
    pref_out[localID * 2 + 1] = pref_in[localID * 2 + 1];
    uint LOG = uint(log2(gl_WorkGroupSize.x));

    uint source;
    uint destination;
    uint mask;
    barrier();
    memoryBarrierShared();

    for(int i = 0; i <= LOG; i++){
        mask = (1 << i) - 1;
        source = ((localID >> i) << (i + 1)) | mask;
        destination = source + (localID & mask) + 1;
        pref_out[destination] += pref_out[source];
        barrier();
        memoryBarrierShared();
    }
}

shared int shared_data[gl_WorkGroupSize.x * 2];
shared int swap_table[gl_WorkGroupSize.x * 2];

void main(){
    uint id = gl_GlobalInvocationID.x;
    uint localID = gl_LocalInvocationID.x;

    if(id * 2 < n)
        shared_data[localID * 2] = T.data[id * 2];
    else
        shared_data[localID * 2] = MAX_INT;
    if(id * 2 + 1 < n)
        shared_data[localID * 2 + 1] = T.data[id * 2 + 1];
    else
        shared_data[localID * 2 + 1] = MAX_INT;

    uint mask;
    uint zeroCount;
    uint destination;
    barrier();
    memoryBarrierShared();
    for(int i = 0; i < bits; i++){
        mask = (1 << i);
        if(mask & shared_data[localID * 2])
            pref_in[localID * 2] = 1;
        else
            pref_in[localID * 2] = 0;

        if(mask & shared_data[localID * 2 + 1])
            pref_in[localID * 2 + 1] = 1;
        else
            pref_in[localID * 2 + 1] = 0;

        barrier();
        memoryBarrierShared();

        prefix_sum();
        barrier();
        memoryBarrierShared();

        zeroCount = gl_WorkGroupSize.x * 2 - pref_out[gl_WorkGroupSize.x * 2 - 1];

        if(pref_in[localID * 2] == 1)
            destination = pref_out[localID * 2] - 1 + zeroCount;
        else
            destination = localID * 2 - pref_out[localID * 2];
        swap_table[destination] = shared_data[localID * 2];

        if(pref_in[localID * 2 + 1] == 1)
            destination = pref_out[localID * 2 + 1] - 1 + zeroCount;
        else
            destination = localID * 2 + 1 - pref_out[localID * 2 + 1];
        swap_table[destination] = shared_data[localID * 2 + 1];

        barrier();
        memoryBarrierShared();

        shared_data[localID * 2] = swap_table[localID * 2];
        shared_data[localID * 2 + 1] = swap_table[localID * 2 + 1];
        barrier();
        memoryBarrierShared();
    }
    if(id * 2 < n)
        T.data[id * 2] = shared_data[localID * 2];
    if(id * 2 + 1 < n)
        T.data[id * 2 + 1] = shared_data[localID * 2 + 1];
}
