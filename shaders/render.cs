#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct ray{
    vec4 origin;
    vec4 dir;
    uint intersectedTriangle;
    float closestDistance;
};

layout (rgba32f, binding = 0) uniform image2D imgOutput;

layout (std430, binding = 1) buffer Rays{
    ray data[];
} rays;

uniform int pixWidth;
uniform int pixHeight;

void main(){
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;
    if(x < pixWidth && y < pixHeight){
        vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
        ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
        if(rays.data[y * pixWidth + x].closestDistance < 1000)
            value = vec4(0.5,0.5,0.5,1.0);
        else
            value = vec4(0.8,0.8,0.8,1.0);
        imageStore(imgOutput, texelCoord, value);
    }
}
