#pragma once
#include "ofMain.h"

struct ofxIBLMaterial{
    
    
    bool useBaseColorMap = false;
    bool useRoughnessMap = false;
    bool useMetallicMap = false;
    bool useNormalMap = false;
    bool useOcclusionMap = false;
    bool useEmissionMap = false;
    bool useDetailBaseColorMap = false;
    bool useDetailNormalMap = false;
    bool useGlobalColor = false;
    
    GLuint baseColorMapIndex;
    GLuint roughnessMapIndex;
    GLuint metallicMapIndex;
    GLuint normalMapIndex;
    GLuint occlusionMapIndex;
    GLuint emissionMapIndex;
    GLuint detailBaseColorMapIndex;
    GLuint DetailNormalMapIndex;
    GLuint GlobalColorIndex;
    
    GLuint subroutineIndexArray[9];
    
    ofImage *baseColorMap;
    ofImage *roughnessMap;
    ofImage *metallicMap;
    ofImage *normalMap;
    ofImage *occlusionMap;
    ofImage *emissionMap;
    ofImage *detailBaseColorMap;
    ofImage *detailNormalMap;
    ofColor baseColor = ofColor(255);
    float roughness = 0.0;
    float metallic = 0.0;
    float normalVal = 1.0;
    ofVec2f textureRepeatTimes = ofVec2f(1.0, 1.0);
    ofVec2f detailTextureRepeatTimes = ofVec2f(1.0, 1.0);
    ofShader *shader;
    
    void begin(ofShader *shader);
    void end();
};