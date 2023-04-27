#ifndef RADIXSORT_H
#define RADIXSORT_H

#include "../opengl/gl.h"
#include "../utils/math.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"

#include <qopenglfunctions_4_3_core.h>

unsigned int radixSort(int n, int bits, unsigned int inputID){
    Shader *radixSortShader = ShaderManager::getShader("radixSortShader");
    radixSortShader->use();
    radixSortShader->setInt("n", n);
    radixSortShader->setInt("bits", bits);

    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputID);
    GL::funcs.glDispatchCompute(ceiling(n,1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
//    return inputID;

    Shader *bitonicSortShader = ShaderManager::getShader("bitonicSortShader");
    bitonicSortShader->use();
    bitonicSortShader->setInt("n", n);
    unsigned int outputID = createEmptySSBO<int>(n);
    for(int chunkSize = 1024; chunkSize < n; chunkSize *= 2){
        bitonicSortShader->setInt("chunkSize", chunkSize);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputID);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputID);
        GL::funcs.glDispatchCompute(ceiling(ceiling(n, chunkSize),2), 1, 1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
        std::swap(inputID, outputID);
    }
    GL::funcs.glDeleteBuffers(1, &outputID);
    return inputID;
}

#endif // RADIXSORT_H
