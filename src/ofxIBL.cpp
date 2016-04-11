#include "ofxIBL.h"

ofxIBL::ofxIBL(){
    IBLShader.load("shaders/ibl");
    
    sphereMesh = ofSpherePrimitive(1, 100).getMesh();
    for(int i=0;i<sphereMesh.getNormals().size();i++){
        sphereMesh.setNormal(i, ofVec3f(1.0, 1.0, -1.0) * sphereMesh.getVertex(i).normalize());
    }
}

void ofxIBL::begin(ofCamera * camera, ofxFilteredCubeMap * cubeMap){
    this->cubeMap = cubeMap;
    cubeMap -> bind(1);
    IBLShader.begin();
    IBLShader.setUniform1i("envMap", 1);
    IBLShader.setUniform1i("numMips", cubeMap -> getNumMips());
    IBLShader.setUniformMatrix4f("viewTranspose", ofMatrix4x4::getTransposedOf(camera -> getModelViewMatrix()));
    IBLShader.setUniform1f("cameraNear", camera -> getNearClip());
    IBLShader.setUniform1f("cameraFar", camera -> getFarClip());
    IBLShader.setUniform1i("isHDR", cubeMap -> isHDR());
}

void ofxIBL::end(){
    IBLShader.end();
    cubeMap -> unbind();
}

void ofxIBL::drawEnvironment(float scale, float level){
    IBLShader.setUniform1i("drawEnv", 1);
    IBLShader.setUniform1f("envLevel", level);
    ofPushMatrix();
    ofScale(scale, scale, scale);
    sphereMesh.draw();
    ofPopMatrix();
    IBLShader.setUniform1i("drawEnv", 0);
}