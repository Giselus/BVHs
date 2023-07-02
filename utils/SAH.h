#ifndef SAH_H
#define SAH_H

#include "../opengl/gl.h"
#include "../utils/compactQueue.h"
#include "../utils/math.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"
#include "../utils/triangle.h"

#include <qopenglfunctions_4_3_core.h>

void SAH(int n, int lastID, int queueSize, unsigned int nodeBuffer, unsigned int triangleBuffer, unsigned int queueBuffer){
    unsigned int triangleSwapBuffer = createEmptySSBO<triangle>(n);
    unsigned int queueSwapBuffer = createEmptySSBO<possibleSplit>(n);
    unsigned int smallQueueBuffer = createEmptySSBO<unsigned int>(n);

    Shader* SAHShader = ShaderManager::getShader("SAHShader");

    int smallQueueSize = 0;
    int level = 0;
    while(queueSize != 0){
        level++;
        SAHShader->use();

        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleSwapBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, queueBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, queueSwapBuffer);
        GL::funcs.glDispatchCompute(queueSize, 1, 1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

        compactQueue(n, lastID, queueSize, smallQueueSize, nodeBuffer, queueBuffer, queueSwapBuffer, smallQueueBuffer);
    }


    Shader *setParentsShader = ShaderManager::getShader("setParentsShader");
    unsigned int atomicCountersBuffer = createEmptySSBO<unsigned int>(2 * n, true);

    setParentsShader->use();
    setParentsShader->setInt("n", lastID + 1);

    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, atomicCountersBuffer);

    GL::funcs.glDispatchCompute(ceiling(lastID + 1, 1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    GL::funcs.glDeleteBuffers(1, &atomicCountersBuffer);

    atomicCountersBuffer = createEmptySSBO<unsigned int>(2 * n, true);

    Shader *recalculateBoxesShader = ShaderManager::getShader("recalculateBoxesShader");
    recalculateBoxesShader->use();
    recalculateBoxesShader->setInt("n", lastID + 1);

    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, atomicCountersBuffer);

    GL::funcs.glDispatchCompute(ceiling(lastID + 1, 1024), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    GL::funcs.glDeleteBuffers(1, &atomicCountersBuffer);

    auto y = getArrayFromBuffer<node>(lastID+1, nodeBuffer);
    float SAHScore = 0;
    for(int i = 0; i <= lastID; i++){
        if(y[i].children[0] != 0){
            int l = y[i].children[0];
            int r = y[i].children[1];
            float area = 1.0f;
            float areaL = 1.0f;
            float areaR = 1.0f;
            for(int j = 0; j < 3; j++){
                area *= y[i].box[j + 4] - y[i].box[j];
                areaL *= y[l].box[j + 4] - y[l].box[j];
                areaR *= y[r].box[j + 4] - y[r].box[j];
            }
            SAHScore += areaL/area * (y[l].interval[1] - y[l].interval[0] + 1);
            SAHScore += areaR/area * (y[r].interval[1] - y[r].interval[0] + 1);

            if(y[y[i].children[0]].parent != i || y[y[i].children[1]].parent != i){
                qDebug() << i << "Bad parent!\n" << y[y[i].children[0]].parent << y[y[i].children[1]].parent;
                break;
            }
            if(y[i].interval[0] != std::min(y[y[i].children[0]].interval[0] ,y[y[i].children[1]].interval[0]) ||
                    y[i].interval[1] != std::max(y[y[i].children[0]].interval[1], y[y[i].children[1]].interval[1])){
                qDebug() << i << "Bad intervals\n";
                break;
            }
            for(int j = 0; j < 3; j++){
                float p = std::min(y[y[i].children[0]].box[j], y[y[i].children[1]].box[j]);
                float p2 = std::max(y[y[i].children[0]].box[4 + j], y[y[i].children[1]].box[4 + j]);
                if(p != y[i].box[j]){
                    qDebug() << "Bad boxes!\n" << i;
                    break;
                }
                if(p2 != y[i].box[4 + j]){
                    qDebug() << "Bad boxes!\n" << i;
                    break;
                }
            }
        }
    }

    qDebug() << "SAH score" << SAHScore;
}

#endif // SAH_H
