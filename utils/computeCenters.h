#ifndef COMPUTECENTERS_H
#define COMPUTECENTERS_H

#include "../opengl/gl.h"
#include "../utils/math.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"

#include <qopenglfunctions_4_3_core.h>

unsigned int computeCenters(int n, int inputID){
    Shader* computeCentersShader = ShaderManager::getShader("computeCenterShader");
    computeCentersShader->use();
    computeCentersShader->setInt("n", n);

    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputID);
    GL::funcs.glDispatchCompute(ceiling(n,1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    return inputID;
}

#endif // COMPUTECENTERS_H
