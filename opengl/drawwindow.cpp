#include "drawwindow.h"

#include "gl.h"
#include "../utils/computeCenters.h"
#include "../utils/computeMortonCodes.h"
#include "../utils/Hybrid.h"
#include "../utils/LBVH.h"
#include "../utils/load_obj.h"
#include "../utils/prefixsum.h"
#include "../utils/radixsort.h"
#include "../utils/SAH.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"
#include "../utils/triangle.h"

#include <chrono>

void DrawWindow::initialize(){
    ShaderManager::initialize();

    GLfloat vertices[] = {
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };

    GLfloat texCoords[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    GLfloat data[6*5];

    for(int i = 0; i < 6; i++){
        data[i*5] = vertices[i*3];
        data[i*5+1] = vertices[i*3+1];
        data[i*5+2] = vertices[i*3+2];

        data[i*5+3] = texCoords[i*2];
        data[i*5+4] = texCoords[i*2+1];
    }

    GL::funcs.glGenVertexArrays(1, &VAO);
    GL::funcs.glGenBuffers(1, &VBO);

    GL::funcs.glBindVertexArray(VAO);

    GL::funcs.glBindBuffer(GL_ARRAY_BUFFER, VBO);
    GL::funcs.glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    // position attribute
    GL::funcs.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    GL::funcs.glEnableVertexAttribArray(0);
    // color attribute
    GL::funcs.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    GL::funcs.glEnableVertexAttribArray(1);

    GL::funcs.glGenTextures(1, &textureID);
    GL::funcs.glActiveTexture(GL_TEXTURE0);
    GL::funcs.glBindTexture(GL_TEXTURE_2D, textureID);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL::funcs.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    GL::funcs.glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    int n = 69451;
    for(int i = 0; i < 1; i++){
        srand(time(NULL));
        float *obj_data = load_obj("media/bunny.obj");
        triangle *randomInput = (triangle*)malloc(sizeof(triangle) * n);
        for(int i = 0; i < n; i++){
            for(int j = 0; j < 12; j++){
                randomInput[i].position[j] = obj_data[i * 12 + j];
            }
            randomInput[i].code = i;
            randomInput[i].color[0] = randomInput[i].color[1] = randomInput[i].color[2] = 0.5;
        }
        GL::funcs.glGenBuffers(1, &triangleBuffer);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleBuffer);
        GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(triangle) * n, randomInput, GL_STATIC_DRAW);
        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangleBuffer);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

        node *nodeInput = (node*)malloc(sizeof(node) * 1);
        nodeInput[0].interval[0] = 0;
        nodeInput[0].interval[1] = n-1;
        nodeInput[0].parent = 0;
        nodeInput[0].children[0] = 0;
        nodeInput[0].children[1] = 0;
        for(int j = 0; j < 8; j++){
            nodeInput[0].box[j] = 0.0f;
        }

        GL::funcs.glGenBuffers(1, &nodeBuffer);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodeBuffer);
        GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(node) * (2 * n), NULL, GL_STATIC_DRAW);
        GL::funcs.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(node), nodeInput);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

        unsigned int *queueInput = (unsigned int*)malloc(sizeof(unsigned int) * 1);
        queueInput[0] = 0;

        GL::funcs.glGenBuffers(1, &queueBuffer);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, queueBuffer);
        GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * n, NULL, GL_STATIC_DRAW);
        GL::funcs.glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int), queueInput);
        GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

        QVector3D cameraFront = QVector3D(0,0,1);
        QVector3D cameraUp = QVector3D(0,1,0);
        QVector3D cameraRight = QVector3D(1,0,0);

        QVector3D cameraPos = QVector3D(0.5,0.5,-0.4);

        unsigned int screenWidth = TEXTURE_WIDTH;
        unsigned int screenHeight = TEXTURE_HEIGHT;

        rayBuffer = createEmptySSBO<ray>(screenWidth * screenHeight);
        Shader *generateRaysShader = ShaderManager::getShader("generateRaysShader");
        generateRaysShader->use();
        generateRaysShader->setInt("pixWidth", screenWidth);
        generateRaysShader->setInt("pixHeight", screenHeight);
        generateRaysShader->setFloat("width", 1);
        generateRaysShader->setFloat("height", 1);
        generateRaysShader->setVec3("cameraPos", cameraPos);
        generateRaysShader->setVec3("cameraFront", cameraFront);
        generateRaysShader->setVec3("cameraUp", cameraUp);
        generateRaysShader->setVec3("cameraRight", cameraRight);

        GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, rayBuffer);
        GL::funcs.glDispatchCompute(ceiling(screenWidth, 8), ceiling(screenHeight, 8), 1);
        GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);


        GLuint query;
        GLuint64 elapsed_time;
        GL::funcs.glGenQueries(1, &query);
        GL::funcs.glBeginQuery(GL_TIME_ELAPSED, query);

        triangleBuffer = computeCenters(n, triangleBuffer);
//        triangleBuffer = computeMortonCodes(n, 6, triangleBuffer);

//        hybrid(n, nodeBuffer, triangleBuffer, queueBuffer);
        SAH(n, 0, 1, nodeBuffer, triangleBuffer, queueBuffer);
//        LBVH(n, 30, nodeBuffer, triangleBuffer);


        GL::funcs.glEndQuery(GL_TIME_ELAPSED);
        int done = 0;
        while(!done){
            GL::funcs.glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
        }
        GL::funcs.glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
        qDebug() << "BVH generation " << (float)elapsed_time/1000000.0 << " ms";

        delete[] randomInput;
        delete[] nodeInput;
        delete[] queueInput;
    }

}

void DrawWindow::render(){
    GL::funcs.glEnable(GL_DEPTH_TEST);
    GL::funcs.glDepthMask(GL_TRUE);

    const qreal retinaScale = devicePixelRatio();
    GL::funcs.glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    GL::funcs.glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    GL::funcs.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint query;
    GLuint64 elapsed_time;
    GL::funcs.glGenQueries(1, &query);
    GL::funcs.glBeginQuery(GL_TIME_ELAPSED, query);

    Shader *rayTracingShader = ShaderManager::getShader("rayTracingShader");
    rayTracingShader->use();
    rayTracingShader->setInt("n", TEXTURE_WIDTH * TEXTURE_HEIGHT);

    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, nodeBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, rayBuffer);
    GL::funcs.glDispatchCompute(ceiling(TEXTURE_WIDTH * TEXTURE_HEIGHT, 32), 1, 1);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);

    GL::funcs.glEndQuery(GL_TIME_ELAPSED);
    int done = 0;
    while(!done){
        GL::funcs.glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
    }
    GL::funcs.glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
    counter++;
    if(counter % 5 == 0){
        qDebug() << "Ray tracing " << (float)elapsed_time/1000000.0 << " ms";
        counter = 0;
    }
    Shader *renderShader = ShaderManager::getShader("renderShader");
    renderShader->use();
    renderShader->setInt("pixWidth", TEXTURE_WIDTH);
    renderShader->setInt("pixHeight", TEXTURE_HEIGHT);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, rayBuffer);
    GL::funcs.glDispatchCompute(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1);
    GL::funcs.glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    Shader *gradShader = ShaderManager::getShader("gradShader");
    gradShader->use();
    gradShader->setInt("tex", 0);
    GL::funcs.glActiveTexture(GL_TEXTURE0);
    GL::funcs.glBindTexture(GL_TEXTURE_2D, textureID);
    GL::funcs.glBindVertexArray(VAO);
    GL::funcs.glDrawArrays(GL_TRIANGLES, 0, 6);
}
