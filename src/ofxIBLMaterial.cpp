#include "ofxIBLMaterial.h"

void ofxIBLMaterial::begin(ofShader * shader){
    this -> shader = shader;
    shader -> setUniform2f("textureRepeatTimes", textureRepeatTimes);
    shader -> setUniform2f("detailTextureRepeatTimes", detailTextureRepeatTimes);
    // baseColor
    if(useGlobalColor){
        shader -> setUniform1i("useGlobalColor", 1);
    } else {
        shader -> setUniform1i("useGlobalColor", 0);
        shader -> setUniform4f("baseColorUniform", baseColor);
    }
    if(useBaseColorMap){
        shader -> setUniform1i("useBaseColorMap", 1);
        shader -> setUniformTexture("baseColorMap", baseColorMap -> getTexture(), 2);
    } else {
        shader -> setUniform1i("useBaseColorMap", 0);
    }
    
    // roughnessMap
    if(useRoughnessMap){
        shader -> setUniform1i("useRoughnessMap", 1);
        shader -> setUniformTexture("roughnessMap", roughnessMap -> getTexture(), 3);
    } else {
        shader -> setUniform1i("useRoughnessMap", 0);
        shader -> setUniform1f("roughness", roughness);
    }
    
    // metallicMap
    if(useMetallicMap){
        shader -> setUniform1i("useMetallicMap", 1);
        shader -> setUniformTexture("metallicMap", metallicMap -> getTexture(), 4);
    } else {
        shader -> setUniform1i("useMetallicMap", 0);
        shader -> setUniform1f("metallic", metallic);
    }
    
    // normalMap
    if(useNormalMap){
        shader -> setUniform1i("useNormalMap", 1);
        shader -> setUniformTexture ("normalMap", normalMap -> getTexture(), 5);
        shader -> setUniform1f ("normalVal", normalVal);
    } else {
        shader -> setUniform1i("useNormalMap", 0);
    }
    
    // occlusionMap
    if(useOcclusionMap){
        shader -> setUniform1i("useOcclusionMap", 1);
        shader -> setUniformTexture("occlusionMap", occlusionMap -> getTexture(), 6);
    } else {
        shader -> setUniform1i("useOcclusionMap", 0);
    }
    
    // emissionMap
    if(useEmissionMap){
        shader -> setUniform1i("useEmissionMap", 1);
        shader -> setUniformTexture("emissionMap", emissionMap -> getTexture(), 7);
    } else {
        shader -> setUniform1i("useEmissionMap", 0);
    }
    
    // detailBaseColor
    if(useDetailBaseColorMap){
        shader -> setUniform1i("useDetailBaseColorMap", 1);
        shader -> setUniformTexture("detailBaseColorMap", detailBaseColorMap -> getTexture(), 8);
    } else {
        shader -> setUniform1i("useDetailBaseColorMap", 0);
    }
    
    // detailNormalMap
    if(useDetailNormalMap){
        shader -> setUniform1i("useDetailNormalMap", 1);
        shader -> setUniformTexture ("detailNormalMap", detailNormalMap -> getTexture(), 9);
    } else {
        shader -> setUniform1i("useDetailNormalMap", 0);
    }
}

void ofxIBLMaterial::end(){
    shader -> setUniform1i("useBaseColorMap", 0);
    shader -> setUniform1i("useRoughnessMap", 0);
    shader -> setUniform1i("useMetallicMap", 0);
    shader -> setUniform1i("useNormalMap", 0);
    shader -> setUniform1i("useOcclusionMap", 0);
    shader -> setUniform1i("useEmissionMap", 0);
    shader -> setUniform1i("useDetailBaseColorMap", 0);
    shader -> setUniform1i("useDetailNormalMap", 0);
    shader -> setUniform1i("useGlobalColor", 0);
    shader -> setUniform2f("textureRepeatTimes", ofVec2f(1.0, 1.0));
    shader -> setUniform2f("detailTextureRepeatTimes", ofVec2f(1.0, 1.0));
}