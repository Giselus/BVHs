#ifndef PREFIXSUM_H
#define PREFIXSUM_H

#include "../opengl/gl.h"
#include "../utils/math.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"

#include <qopenglfunctions_4_3_core.h>

unsigned int prefixSum(int n, int inputID, bool inclusive){
    unsigned int outputID;

    GL::funcs.glGenBuffers(1, &outputID);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputID);
    GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * nextMultiple(n,32), nullptr, GL_STATIC_COPY);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    Shader *shader = ShaderManager::getShader("prefixSumShader");
    shader->use();
    shader->setInt("n",n);

    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputID);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputID);

    if(n <= 32){
        shader->setInt("copyPartialSums", 0);
        GL::funcs.glDispatchCompute(ceiling(n,32),1,1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }else{
        unsigned int partialResultsID;
        GL::funcs.glGenBuffers(1, &partialResultsID);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, partialResultsID);
        GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * ceiling(n,32), nullptr, GL_STATIC_COPY);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, partialResultsID);

        shader->setInt("copyPartialSums", 1);

        GL::funcs.glDispatchCompute(ceiling(n,32), 1, 1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

        unsigned int resultID = prefixSum(ceiling(n,32), partialResultsID, false);

        Shader *addScalarShader = ShaderManager::getShader("addScalarToBlockShader");
        addScalarShader->use();
        addScalarShader->setInt("n", n);

        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outputID);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, resultID);
        GL::funcs.glDispatchCompute(ceiling(n,32), 1, 1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
        GL::funcs.glDeleteBuffers(1, &partialResultsID);
        GL::funcs.glDeleteBuffers(1, &resultID);
    }
    if(inclusive){
        Shader *addShader = ShaderManager::getShader("addArraysShader");
        addShader->use();
        addShader->setInt("n", n);

        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outputID);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inputID);
        GL::funcs.glDispatchCompute(ceiling(n,32), 1, 1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }
    return outputID;
}

#endif // PREFIXSUM_H
