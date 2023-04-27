#ifndef GL_H
#define GL_H

#include <QOpenGLFunctions_4_3_Core>
#include <QVector3D>

class GL{
public:
    static QOpenGLFunctions_4_3_Core funcs;
};

template <typename T>
unsigned int createEmptySSBO(int n){
    unsigned int result;

    GL::funcs.glGenBuffers(1, &result);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, result);
    GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * n, nullptr, GL_STATIC_COPY);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return result;
}

template <typename T>
T getElementFromBuffer(int id, unsigned int bufferID){
    T tmp;
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    GL::funcs.glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * (id), sizeof(T), &tmp);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
    return tmp;
}

template <typename T>
T* getArrayFromBuffer(int n, unsigned int bufferID){
    T* tmp = (T*)malloc(sizeof(T) * n);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    GL::funcs.glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(T) * n, tmp);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
    return tmp;
}

template <typename T>
void printBuffer(int n, unsigned int bufferID){
    T *tmp = (T*)malloc(sizeof(T) * n);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    GL::funcs.glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * n, tmp);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
    for(int i = 0; i < n; i++){
        qDebug() << i << tmp[i];
    }
    qDebug() << "";
}

template <typename T>
void printBuffer(int l, int n, unsigned int bufferID){
    T *tmp = (T*)malloc(sizeof(T) * n);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferID);
    GL::funcs.glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * l, sizeof(T) * n, tmp);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
    for(int i = 0; i < n; i++){
        qDebug() << i << tmp[i];
    }
    qDebug() << "";
}

#endif // GL_H
