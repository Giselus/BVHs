#ifndef LOAD_OBJ_H
#define LOAD_OBJ_H

#include <QDebug>
#include <QImage>
#include <QOpenGLFunctions_3_3_Core>
#include <QPixmap>
#include <QString>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

float* load_obj(string path){
    vector<float> positions;
    vector<float> normals;
    vector<float> texCoords;

    vector<float> data;
    ifstream file(path);

    string line;
    float minValue = 1e10;
    float maxValue = -1e10;
    while(getline(file, line)){
        istringstream iss(line);
        string type;
        iss >> type;
        float x,y,z;
        if(type == "v"){
            iss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);

            minValue = min(minValue, x);
            minValue = min(minValue, y);
            minValue = min(minValue, z);

            maxValue = max(maxValue, x);
            maxValue = max(maxValue, y);
            maxValue = max(maxValue, z);

        }else if(type == "vt"){
            iss >> x >> y;
            texCoords.push_back(x);
            texCoords.push_back(y);
        }else if(type == "vn"){
            iss >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }else if(type == "f"){
            int v0, v1, v2;
            char c, c2;
            float scale = 1.0/(maxValue - minValue);
            float offset = -minValue;
            iss >> v0 >> v1 >> v2;
            data.push_back((positions[(v0-1) * 3] + offset) * scale);
            data.push_back((positions[(v0-1) * 3 + 1] + offset) * scale);
            data.push_back((positions[(v0-1) * 3 + 2] + offset) * scale);
            data.push_back(0.0f);

            data.push_back((positions[(v1-1) * 3] + offset) * scale);
            data.push_back((positions[(v1-1) * 3 + 1] + offset) * scale);
            data.push_back((positions[(v1-1) * 3 + 2] + offset) * scale);
            data.push_back(0.0f);

            data.push_back((positions[(v2-1) * 3] + offset) * scale);
            data.push_back((positions[(v2-1) * 3 + 1] + offset) * scale);
            data.push_back((positions[(v2-1) * 3 + 2] + offset) * scale);
            data.push_back(0.0f);

        }else{
            continue;
        }
    }
//    qDebug() << data.size();
    file.close();
    float *result = (float*)malloc(sizeof(float) * data.size());
    for(int i = 0; i < data.size(); i++){
        result[i] = data[i];
    }
    return result;
}

float* load_obj_extended(string path){
    vector<float> positions;
    vector<float> normals;
    vector<float> texCoords;

    vector<float> data;
    ifstream file(path);

    string line;
    float minValue = 1e10;
    float maxValue = -1e10;
    while(getline(file, line)){
        istringstream iss(line);
        string type;
        iss >> type;
        float x,y,z;
        if(type == "v"){
            iss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);

            minValue = min(minValue, x);
            minValue = min(minValue, y);
            minValue = min(minValue, z);

            maxValue = max(maxValue, x);
            maxValue = max(maxValue, y);
            maxValue = max(maxValue, z);
        }else if(type == "vt"){
            iss >> x >> y;
            texCoords.push_back(x);
            texCoords.push_back(y);
        }else if(type == "vn"){
            iss >> x >> y >> z;
            normals.push_back(x);
            normals.push_back(y);
            normals.push_back(z);
        }else if(type == "f"){
            int v0, v1, v2;
            char c, c2;
            for(int i = 0; i < 3; i++){
                float scale = 1.0/(maxValue - minValue);
                float offset = -minValue;
                iss >> v0 >> c  >> c2 >> v2;
                data.push_back((positions[(v0-1) * 3] + offset) * scale);
                data.push_back((positions[(v0-1) * 3 + 1] + offset) * scale);
                data.push_back((positions[(v0-1) * 3 + 2] + offset) * scale);
                data.push_back(0.0f);

//                data.push_back(normals[(v2-1) * 3]);
//                data.push_back(normals[(v2-1) * 3 + 1]);
//                data.push_back(normals[(v2-1) * 3 + 2]);

//                data.push_back(texCoords[(v1-1) * 2]);
//                data.push_back(texCoords[(v1-1) * 2 + 1]);
            }
        }else{
            continue;
        }
    }
    qDebug() << data.size()/12;
//    exit(0);
    file.close();
    float *result = (float*)malloc(sizeof(float) * data.size());
    for(int i = 0; i < data.size(); i++){
        result[i] = data[i];
    }
    return result;
}

#endif // LOAD_OBJ_H
