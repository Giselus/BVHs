#version 430 core

//Computes center for every triangle, center for triangle is center of its bounding box

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct triangle{
    vec4 position[3];
    vec4 color;
    vec4 center;
    vec4 box[2];
    int code;
};

layout (std430, binding = 0) buffer Input{
    triangle data[];
} T;

uniform int n;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        vec4 box[2];
        box[0] = vec4(1,1,1,0);
        box[1] = vec4(0,0,0,0);

        for(int i = 0; i < 3; i++){
            box[0] = min(box[0], T.data[id].position[i]);
            box[1] = max(box[1], T.data[id].position[i]);
        }

        T.data[id].box[0] = box[0];
        T.data[id].box[1] = box[1];
        T.data[id].center = (box[1] + box[0]) / 2.0;
    }
}
