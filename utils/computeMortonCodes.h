#ifndef COMPUTEMORTONCODES_H
#define COMPUTEMORTONCODES_H

#include "../opengl/gl.h"
#include "../utils/math.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"

#include <qopenglfunctions_4_3_core.h>

unsigned int computeMortonCodes(int n, int bits, int inputID){
    Shader* computeMortonCodesShader = ShaderManager::getShader("computeMortonCodesShader");
    computeMortonCodesShader->use();
    computeMortonCodesShader->setInt("n", n);
    computeMortonCodesShader->setInt("bits",bits/3);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0 , inputID);
    GL::funcs.glDispatchCompute(ceiling(n, 1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    return inputID;
}


#endif // COMPUTEMORTONCODES_H
