#pragma once
#include "ofMain.h"
//#include "shaders/importanceSampling.h"
//#include "shaders/swapRB.h"

class ofxFilteredCubeMap{
private:
    ofCamera envCam[6];
    ofFbo envFbo[6];
    ofTexture envTexture;
    
    ofImage iEnv;
    ofImage iEnvMapImages[6];
    vector <ofImage> iFilteredImages[6];
    
    ofFloatImage fEnv;
    ofFloatImage fEnvMapImages[6];
    vector <ofFloatImage> fFilteredImages[6];
    
    ofShader shader;
//    ImportanceSampling importanceSampling;
    ofMesh sphereMesh, envSphereMesh;
    unsigned int cubeMapID;
    unsigned int filteredCubeMapID;
    int textureUnit;
    int baseSize;
    
    int cacheWidth, cacheHeight;
    ofFbo cacheFbo;
    
    int textureFormat;
    ofImage iCacheImage;
    ofFloatImage fCacheImage;
    
    ofMesh skyboxFaces[6];
    
    int maxMipLevel;
    
    void makeCubeMapTextures();
    void makeCubeMap();
    void makeFilteredCubeMap();
    void makeCache(string cachePath);
    void makeCube();
    
public:
    ofxFilteredCubeMap();
    
    void load(ofImage * sphereMapImage, int baseSize = 512);
    void load(ofFloatImage * sphereMapImage, int baseSize = 512);
    void load(string imagePath, int baseSize = 512, bool useCache = false, string cacheDirectry = "");
    void loadFromCache(string cachePath);
    void bind(int pos);
    void unbind();
    void debug(int level);
    void drawSkyBox(int level, float size);
    void drawSphere();
    bool isHDR();
    int getNumMips();
};