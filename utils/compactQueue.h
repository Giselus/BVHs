#ifndef COMPACTQUEUE_H
#define COMPACTQUEUE_H

#include "../opengl/gl.h"
#include "../utils/math.h"
#include "../utils/prefixsum.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"
#include "../utils/triangle.h"

#include <qopenglfunctions_4_3_core.h>

#include <iostream>

void compactQueue(int n, int &lastID, int &queueSize, int &smallQueueSize, unsigned int nodeBuffer, unsigned int queueBuffer, unsigned int queueSwapBuffer, unsigned int smallQueueBuffer){

    //preCompaction
    //prefix sums
    //compaction

    Shader *preCompactionShader = ShaderManager::getShader("preCompactionShader");
    Shader *compactionShader = ShaderManager::getShader("compactionShader");

    unsigned int prefixSumBuffer = createEmptySSBO<unsigned int>(n, true);
    unsigned int smallQueuePrefixSumBuffer = createEmptySSBO<unsigned int>(n, true);

    preCompactionShader->use();
    preCompactionShader->setInt("n", queueSize * 2);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, queueSwapBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, prefixSumBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, smallQueuePrefixSumBuffer);
    GL::funcs.glDispatchCompute(ceiling(queueSize * 2 ,1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    unsigned int prefixSumOutput =
            prefixSum(queueSize * 2, prefixSumBuffer, true);
    unsigned int smallQueuePrefixSumOutput =
            prefixSum(queueSize * 2, smallQueuePrefixSumBuffer, true);

    unsigned int newSplits = getElementFromBuffer<unsigned int>(2 * queueSize - 1, prefixSumOutput);
    unsigned int newSmallSplits = getElementFromBuffer<unsigned int>(2 * queueSize - 1, smallQueuePrefixSumOutput);

    GL::funcs.glDeleteBuffers(1, &prefixSumBuffer);
    GL::funcs.glDeleteBuffers(1, &smallQueuePrefixSumBuffer);

    compactionShader->use();
    compactionShader->setInt("n", queueSize * 2);
    compactionShader->setInt("smallSplitQueueSize", smallQueueSize);
    compactionShader->setInt("lastID", lastID);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, queueSwapBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, queueBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, smallQueueBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, prefixSumOutput);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, smallQueuePrefixSumOutput);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, nodeBuffer);
    GL::funcs.glDispatchCompute(ceiling(queueSize * 2, 1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    GL::funcs.glDeleteBuffers(1, &prefixSumOutput);
    GL::funcs.glDeleteBuffers(1, &smallQueuePrefixSumOutput);

    lastID += newSplits + newSmallSplits;
    queueSize = newSplits;
    smallQueueSize += newSmallSplits;
}

#endif // COMPACTQUEUE_H
