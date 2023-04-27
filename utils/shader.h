#ifndef SHADER_H
#define SHADER_H

#include <string>

#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLFunctions_4_3_Core>

class Shader : protected QOpenGLFunctions_4_3_Core
{
public:
    Shader(const std::string vertexPath, const std::string fragmentPath);
    Shader(const std::string computePath);
    void use();

    void setInt(const std::string name, int value);
    void setUInt(const std::string name, unsigned int value);
    void setFloat(const std::string name, float value);
    void setVec3(const std::string name, QVector3D value);
    void setMat4(const std::string name, QMatrix4x4 value);
private:
    unsigned int ID;
};

#endif // SHADER_H
