#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

struct ray{
    vec4 origin;
    vec4 dir;
    uint intersectedTriangle;
    float closestDistance;
};

layout (std430, binding = 0) buffer Rays{
    ray data[];
} rays;

uniform float width;
uniform float height;

uniform int pixWidth;
uniform int pixHeight;

uniform vec3 cameraPos;

uniform vec3 cameraFront;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

void main(){
    uint x = gl_GlobalInvocationID.x;
    uint y = gl_GlobalInvocationID.y;

    if(x < pixWidth && y < pixHeight){
        vec3 pos = cameraPos + cameraFront + cameraRight * (float(x)/float(pixWidth) - 0.5f) * 2.0f * width + cameraUp * (float(y)/float(pixHeight) - 0.5f) * 2.0f * height;
        ray r;
        r.closestDistance = 1000;
        r.dir = vec4(normalize(pos - cameraPos), 0);
        r.origin = vec4(cameraPos, 0);
//        r.xPix = x;
//        r.yPix = y;
        rays.data[y * pixWidth + x] = r;
    }
}
