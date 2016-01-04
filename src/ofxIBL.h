#pragma once
#include "ofMain.h"
#include "ofxIBLMaterial.h"
#include "ofxFilteredCubeMap.h"
#include "IBL.h"

class ofxIBL{
public:
    ofxIBL();
    
    void setup();
    void begin(ofCamera * camera, ofxFilteredCubeMap * cubeMap);
    void end();
    void drawEnvironment(float scale, float level);
    
    ofShader * getShaderPtr(){
        return &IBLShader;
    }
    
private:
    ofShader IBLShader;
    ofMesh sphereMesh;
    ofxFilteredCubeMap * cubeMap;
    IBL iblShaderStrings;
};