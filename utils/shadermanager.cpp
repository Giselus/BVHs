#include "shadermanager.h"

#include "shader.h"

std::map<std::string, Shader*> ShaderManager::shaders;

void ShaderManager::initialize(){
    shaders["gradShaderCompute"] = new Shader("shaders/gradShader.cs");
    shaders["gradShader"] = new Shader("shaders/gradShader.vs", "shaders/gradShader.fs");
    shaders["prefixSumShader"] = new Shader("shaders/prefixSum.cs");
    shaders["addScalarToBlockShader"] = new Shader("shaders/addScalarToBlock.cs");
    shaders["addArraysShader"] = new Shader("shaders/addArrays.cs");
    shaders["substractArraysShader"] = new Shader("shaders/substractArrays.cs");
}

Shader* ShaderManager::getShader(std::string name){
    return shaders[name];
}
