#version 430 core

#define STACK_LIMIT 128

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
    uint parent;
};

struct ray{
    vec4 origin;
    vec4 dir;
    uint intersectedTriangle;
    float closestDistance;
};

layout (std430, binding = 0) buffer Nodes{
    node data[];
} nodes;

layout (std430, binding = 1) buffer Triangles{
    triangle data[];
} triangles;

layout (std430, binding = 2) buffer Rays{
    ray data[];
} rays;

shared uint reduce_array_left[gl_WorkGroupSize.x];
shared uint reduce_array_right[gl_WorkGroupSize.x];

void reduce(){
    uint localID = gl_LocalInvocationID.x;
    uint LOG = uint(log2(gl_WorkGroupSize.x));
    uint mask;
    for(int i = 1; i <= LOG; i++){
        mask = (1 << i) - 1;
        if((localID & mask) == (1 << (i-1))){
            reduce_array_left[localID - (1 << (i-1))] += reduce_array_left[localID];
            reduce_array_right[localID - (1 << (i-1))] += reduce_array_right[localID];
        }
        barrier();
        memoryBarrierShared();
    }
}

bool intersectBox(inout ray r, vec4 inv, vec4 minBox, vec4 maxBox){
    float tmin = 0.0;
    float tmax = r.closestDistance;

    for(int i = 0; i < 3; i++){
        float t1 = (minBox[i] - r.origin[i]) * inv[i];
        float t2 = (maxBox[i] - r.origin[i]) * inv[i];

        tmin = min(max(t1, tmin), max(t2, tmin));
        tmax = max(min(t1, tmax), min(t2, tmax));
    }

    return tmin <= tmax;
}

bool pointInOrOn(vec3 P1, vec3 P2, vec3 A, vec3 B)
{
    vec3 CP1 = cross(B - A, P1 - A);
    vec3 CP2 = cross(B - A, P2 - A);
    return dot(CP1, CP2) >= 0;
//    return step(0.0, dot(CP1, CP2));
}

bool pointInTriangle(vec3 px, vec3 p0, vec3 p1, vec3 p2)
{
    return
        pointInOrOn(px, p0, p1, p2) &&
        pointInOrOn(px, p1, p2, p0) &&
        pointInOrOn(px, p2, p0, p1);
}

vec3 intersectPlane(ray r, vec3 p1, vec3 p2, vec3 p3){
    vec3 D = r.dir.xyz;
    vec3 N = cross(p2-p1, p3-p1);
    vec3 X = r.origin.xyz + D * dot((p1 - r.origin.xyz), N) / dot(D,N);

    return X;
}

float intersectTriangle(ray r, triangle t){
    vec3 X = intersectPlane(r, t.position[0].xyz, t.position[1].xyz, t.position[2].xyz);
    if(pointInTriangle(X, t.position[0].xyz, t.position[1].xyz, t.position[2].xyz)){
        return distance(r.origin.xyz, X);
    }else{
        return 1e4;
    }
}

shared triangle triangle_batch[gl_WorkGroupSize.x];

shared ray rays_batch[gl_WorkGroupSize.x];

shared node stack[STACK_LIMIT];

uniform int n;

void main(){
    uint localID = gl_LocalInvocationID.x;
    uint ID = gl_GlobalInvocationID.x;
    uint groupID = gl_WorkGroupID.x;

    if(ID >= n){
        return;
    }
    ray r = rays.data[ID];

//    r.closestDistance = 1e4;
//    for(uint offset = 0; offset <= 69450; offset += gl_WorkGroupSize.x){
//        if(offset + localID <= 69450)
//            triangle_batch[localID] = triangles.data[offset + localID];
//        barrier();
//        memoryBarrierShared();
//        for(uint k = 0; k < gl_WorkGroupSize.x && offset + k <= 69450; k++){
//            r.closestDistance = min(r.closestDistance, intersectTriangle(r, triangle_batch[k]));
//        }
//        barrier();
//        memoryBarrierShared();
//    }

//    rays.data[ID] = r;
//    return;

    vec4 inv = vec4(1.0) / r.dir;
    //enough due to scalling scene to [0,1]
    r.closestDistance = 1e4;

    node currentNode = nodes.data[0];
    uint stackSize = 1;
//    if(localID == 0){
//        stack.data[0] = 0;
//    }

    for(; ;){
        //always 2 or 0 children

        if(currentNode.children[0] != 0){
            node L = nodes.data[currentNode.children[0]];
            reduce_array_left[localID] = int(intersectBox(r, inv, L.box[0], L.box[1]));

            node R = nodes.data[currentNode.children[1]];
            reduce_array_right[localID] = int(intersectBox(r, inv, R.box[0], R.box[1]));

            barrier();
            memoryBarrierShared();
            reduce();
            barrier();
            memoryBarrierShared();

            uint l = reduce_array_left[0];
            uint r = reduce_array_right[0];

            if(l > 0 && r > 0){
                stackSize++;
                if(l >= r){
                    if(localID == 0)
                        stack[stackSize - 1] = R;
                    currentNode = L;
                }else{
                    if(localID == 0)
                        stack[stackSize - 1] = L;
                    currentNode = R;
                }
            }else{
                if(l > 0){
                    currentNode = L;
                }else{
                    currentNode = R;
                }
            }
        }else{
            //we're in a leaf node
            uint l_it = currentNode.interval[0];
            uint r_it = currentNode.interval[1];
            for(uint offset = l_it; offset <= r_it; offset += gl_WorkGroupSize.x){
                if(offset + localID <= r_it)
                    triangle_batch[localID] = triangles.data[offset + localID];
                barrier();
                memoryBarrierShared();
                for(uint k = 0; k < gl_WorkGroupSize.x && offset + k <= r_it; k++){
                    r.closestDistance = min(r.closestDistance, intersectTriangle(r, triangle_batch[k]));
                }
                barrier();
                memoryBarrierShared();
            }

            if(stackSize == 0){
                break;
            }else{
                //make sure that stack is up to date
                barrier();
                memoryBarrierShared();
                currentNode = stack[stackSize-1];
                stackSize--;

                barrier();
                memoryBarrierShared();
            }
        }
    }
    rays.data[ID] = r;
}
