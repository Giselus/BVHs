#ifndef SAH_H
#define SAH_H

#include "../opengl/gl.h"
#include "../utils/compactQueue.h"
#include "../utils/math.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"
#include "../utils/triangle.h"

#include <qopenglfunctions_4_3_core.h>

#include <set>

void SAH(int n, int lastID, int queueSize, unsigned int nodeBuffer, unsigned int triangleBuffer, unsigned int queueBuffer){
    unsigned int triangleSwapBuffer = createEmptySSBO<triangle>(n);
    unsigned int queueSwapBuffer = createEmptySSBO<possibleSplit>(n);
    unsigned int smallQueueBuffer = createEmptySSBO<unsigned int>(n);

    Shader* SAHShader = ShaderManager::getShader("SAHShader");
    Shader* reorderTrianglesShader = ShaderManager::getShader("reorderTrianglesShader");

    int smallQueueSize = 0;
    int level = 0;
    while(queueSize != 0){
        level++;
        qDebug() << "sizes" << level << queueSize << smallQueueSize;
//        auto x = getArrayFromBuffer<unsigned int>(queueSize, queueBuffer);
//        for(int i = 0; i < queueSize; i++){
//            qDebug() << "queue" << i << x[i];
//        }
//        auto y = getArrayFromBuffer<node>(lastID+1, nodeBuffer);
//        for(int i = 0; i <= lastID; i++){
//            qDebug() << "nodes" << i << y[i].father << y[i].interval[0] << y[i].interval[1];
//        }
        if(level > 10)
            break;

        SAHShader->use();
//        SAHShader->setInt("ss", rand() * rand());
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleSwapBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, queueBuffer);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, queueSwapBuffer);
        GL::funcs.glDispatchCompute(queueSize, 1, 1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

//        reorderTrianglesShader->use();

//        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
//        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer);
//        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleSwapBuffer);
//        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, queueBuffer);
//        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, queueSwapBuffer);
//        GL::funcs.glDispatchCompute(queueSize, 1, 1);
//        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

        compactQueue(n, lastID, queueSize, smallQueueSize, nodeBuffer, queueBuffer, queueSwapBuffer, smallQueueBuffer);
//        qDebug() << "After compaction: " << queueSize << smallQueueSize;
    }
//    std::set<int> tmp;
//    int p = 0;
//    auto x = getArrayFromBuffer<triangle>(n, triangleBuffer);
//    for(int i = 0; i < n; i++){
//        tmp.insert(x[i].code);
////        qDebug() << i << n << x[i].code;
////        qDebug() << "nodes" << i << y[i].father << y[i].interval[0] << y[i].interval[1];
//    }
//    qDebug() << tmp.size();
//    auto y = getArrayFromBuffer<node>(lastID+1, nodeBuffer);
//    for(int i = 0; i <= lastID; i++){
//        if(y[i].interval[1] - y[i].interval[0] + 1 <= 32){
//            p += y[i].interval[1] - y[i].interval[0] + 1;
//        }
////        qDebug() << "nodes" << i << y[i].father << y[i].interval[0] << y[i].interval[1];
//    }
//    qDebug() << p;

}

#endif // SAH_H
