#version 430 core

//can't use define below
layout (local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

layout (std430, binding = 1) buffer Result{
    int data[];
} R;

layout (std430, binding = 2) buffer PartialResults{
    int data[];
} P;

uniform int n;

//these are actually boolean variables but openGL doesnt support them
uniform int copyPartialSums;

shared int shared_data[gl_WorkGroupSize.x * 2];

void main(){
    uint id = gl_GlobalInvocationID.x;
    uint localID = gl_LocalInvocationID.x;

    if(id * 2 < n)
        shared_data[localID * 2] = T.data[id * 2];
    else
        shared_data[localID * 2] = 0;
    if(id * 2 + 1 < n)
        shared_data[localID * 2 + 1] = T.data[id * 2 + 1];
    else
        shared_data[localID * 2 + 1] = 0;

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
        shared_data[destination] += shared_data[source];
        barrier();
        memoryBarrierShared();
    }
    if(id * 2 < n)
        R.data[id * 2] = shared_data[localID * 2];
    if(id * 2 + 1 < n)
        R.data[id * 2 + 1] = shared_data[localID * 2 + 1];
    barrier();
    memoryBarrierShared();
    if(copyPartialSums == 1 && localID == gl_WorkGroupSize.x - 1)
        P.data[gl_WorkGroupID.x] = R.data[id * 2 + 1];

//    uint id = gl_GlobalInvocationID.x;
//    uint rd_id;
//    uint wr_id;
//    uint mask;

//    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;
//    uint step = 0;

//    shared_data[id * 2] = T.data[id * 2];
//    shared_data[id * 2 + 1] = T.data[id * 2 + 1];
//    barrier();
//    memoryBarrierShared();

//    for (step = 0; step < steps; step++)
//    {

//        mask = (1 << step) - 1;
//        rd_id = ((id >> step) << (step + 1)) + mask;
//        wr_id = rd_id + 1 + (id & mask);

//        shared_data[wr_id] += shared_data[rd_id];

//        barrier();
//        memoryBarrierShared();
//    }

//    R.data[id * 2] = shared_data[id * 2];
//    R.data[id * 2 + 1] = shared_data[id * 2 + 1];
//    barrier();
//    memoryBarrierShared();
//    if(copyPartialSums == 1)
//        P.data[gl_WorkGroupID.x] = R.data[gl_WorkGroupSize.x * 2 - 1] + T.data[gl_WorkGroupSize.x * 2 - 1];

}
