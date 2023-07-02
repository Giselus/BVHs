#ifndef HYBRID_H
#define HYBRID_H

#include "../opengl/gl.h"
#include "../utils/LBVH.h"
#include "../utils/math.h"
#include "../utils/prefixsum.h"
#include "../utils/SAH.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"
#include "../utils/triangle.h"

#include <qopenglfunctions_4_3_core.h>


void hybrid(int n, unsigned int nodeBuffer, unsigned int triangleBuffer, unsigned int queueBuffer){
    LBVH(n, 6, nodeBuffer, triangleBuffer);

    Shader *getActiveSplitsShader = ShaderManager::getShader("getActiveSplitsShader");
    getActiveSplitsShader->use();
    getActiveSplitsShader->setInt("n", n);

    unsigned int allNodesPrefixSumBuffer = createEmptySSBO<unsigned int>(2 * n - 1);
    unsigned int activeNodesPrefixSumBuffer = createEmptySSBO<unsigned int>(2 * n - 1);


    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, allNodesPrefixSumBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, activeNodesPrefixSumBuffer);
    GL::funcs.glDispatchCompute(ceiling(2 * n - 1, 1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    unsigned int allNodesPrefixSumResult = prefixSum(2 * n - 1, allNodesPrefixSumBuffer, true);
    unsigned int activeNodesPrefixSumResult = prefixSum(2 * n - 1, activeNodesPrefixSumBuffer, true);

    GL::funcs.glDeleteBuffers(1, &allNodesPrefixSumBuffer);
    GL::funcs.glDeleteBuffers(1, &activeNodesPrefixSumBuffer);

    int lastID = getElementFromBuffer<unsigned int>(2 * n - 2, allNodesPrefixSumResult) - 1;
    int queueSize = getElementFromBuffer<unsigned int>(2 * n - 2, activeNodesPrefixSumResult);
    Shader *relabelActiveSplitsShader = ShaderManager::getShader("relabelActiveSplitsShader");
    relabelActiveSplitsShader->use();
    relabelActiveSplitsShader->setInt("n", n);
    unsigned int nodeSwapBuffer = createEmptySSBO<node>(2 * n);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, nodeSwapBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, allNodesPrefixSumResult);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, activeNodesPrefixSumResult);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, queueBuffer);
    GL::funcs.glDispatchCompute(ceiling(2 * n - 1, 1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    copyBuffer<node>(2 * n, nodeSwapBuffer, nodeBuffer);
    GL::funcs.glDeleteBuffers(1, &nodeSwapBuffer);

    SAH(n, lastID, queueSize, nodeBuffer, triangleBuffer, queueBuffer);
}

#endif // HYBRID_H
