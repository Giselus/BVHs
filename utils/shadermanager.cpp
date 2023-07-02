#include "shadermanager.h"

#include "shader.h"

std::map<std::string, Shader*> ShaderManager::shaders;

void ShaderManager::initialize(){
    shaders["gradShader"] = new Shader("shaders/gradShader.vs", "shaders/gradShader.fs");
    shaders["prefixSumShader"] = new Shader("shaders/prefixSum.cs");
    shaders["addScalarToBlockShader"] = new Shader("shaders/addScalarToBlock.cs");
    shaders["substractArraysShader"] = new Shader("shaders/substractArrays.cs");
    shaders["radixSortShader"] = new Shader("shaders/radixSort.cs");
    shaders["bitonicSortShader"] = new Shader("shaders/bitonicSort.cs");
    shaders["clearBufferShader"] = new Shader("shaders/clearBuffer.cs");
    shaders["computeCenterShader"] = new Shader("shaders/computeCenter.cs");
    shaders["computeMortonCodesShader"] = new Shader("shaders/generateMortonCodes.cs");

    shaders["SAHShader"] = new Shader("shaders/SAH.cs");
    shaders["preCompactionShader"] = new Shader("shaders/preCompaction.cs");
    shaders["compactionShader"] = new Shader("shaders/compaction.cs");
    shaders["rayTracingShader"] = new Shader("shaders/rayTracing.cs");
    shaders["setParentsShader"] = new Shader("shaders/setParents.cs");
    shaders["recalculateBoxesShader"] = new Shader("shaders/recalculateBoxes.cs");
    shaders["generateRaysShader"] = new Shader("shaders/generateRays.cs");
    shaders["renderShader"] = new Shader("shaders/render.cs");
    shaders["LBVHShader"] = new Shader("shaders/LBVH.cs");
    shaders["getActiveSplitsShader"] = new Shader("shaders/getActiveSplits.cs");
    shaders["relabelActiveSplitsShader"] = new Shader("shaders/relabelActiveSplits.cs");
}

Shader* ShaderManager::getShader(std::string name){
    return shaders[name];
}
