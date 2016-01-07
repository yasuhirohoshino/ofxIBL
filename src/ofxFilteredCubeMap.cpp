#include "ofxFilteredCubeMap.h"

ofxFilteredCubeMap::ofxFilteredCubeMap() {
    baseSize = 512;
    textureFormat = GL_RGB32F;
    shader.setupShaderFromSource(GL_VERTEX_SHADER, importanceSampling.gl3VertShader);
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, importanceSampling.gl3FragShader);
    shader.bindDefaults();
    shader.linkProgram();
    swapRBShader.setupShaderFromSource(GL_VERTEX_SHADER, swapRBChannel.gl3VertShader);
    swapRBShader.setupShaderFromSource(GL_FRAGMENT_SHADER, swapRBChannel.gl3FragShader);
    swapRBShader.bindDefaults();
    swapRBShader.linkProgram();
    if(ofGetVersionInfo().find("0.9.0-", 0) != string::npos){
        isOF090 = true;
    }
    makeCube();
}

void ofxFilteredCubeMap::load(ofImage * sphereMapImage, int baseSize){
    this->baseSize = baseSize;
    maxMipLevel = log2(baseSize) + 1;
    
    for(int i = 0; i < 6; i++){
        iFilteredImages[i].assign(maxMipLevel, ofImage());
    }
    
    iEnv = *sphereMapImage;
    envTexture = iEnv.getTexture();
    textureFormat = GL_RGB;
    makeCubeMapTextures();
}

void ofxFilteredCubeMap::load(ofFloatImage * sphereMapImage, int baseSize){
    this->baseSize = baseSize;
    maxMipLevel = log2(baseSize) + 1;
    for(int i = 0; i < 6; i++){
        fFilteredImages[i].assign(maxMipLevel, ofFloatImage());
    }
    
    fEnv = *sphereMapImage;
    if(isOF090){
        swapRB();
    }else{
        envTexture = fEnv.getTexture();
    }
    textureFormat = GL_RGB32F;
    makeCubeMapTextures();
}

void ofxFilteredCubeMap::load(string imagePath, int baseSize, bool useCache, string cacheDirectry){
    this->baseSize = baseSize;
    maxMipLevel = log2(baseSize) + 1;
    for(int i = 0; i < 6; i++){
        iFilteredImages[i].assign(maxMipLevel, ofImage());
        fFilteredImages[i].assign(maxMipLevel, ofFloatImage());
    }
    
    bool hasCache = false;
    ofFilePath path;
    ofDisableArbTex();
    
    if(useCache){
        string fileName = path.getFileName(imagePath);
        string chacheName = "FCM_Cache_" + ofToString(baseSize) + "_" + path.getFileName(imagePath);
        ofDirectory dir;
        if(cacheDirectry == ""){
            dir.open(path.getEnclosingDirectory(imagePath));
        }else{
            dir.open(cacheDirectry);
        }
        for(int i=0; i<dir.getFiles().size(); i++){
            if(dir.getName(i) == chacheName){
                loadFromCache(dir.getPath(i));
                hasCache = true;
            }
        }
    }
    
    if(!hasCache){
        if(imagePath.find(".hdr", 0) != string::npos || imagePath.find(".exr", 0) != string::npos ){
            fEnv.load(imagePath);
            if(isOF090){
                swapRB();
            } else {
                envTexture = fEnv.getTexture();
            }
            textureFormat = GL_RGB32F;
        } else {
            iEnv.load(imagePath);
            envTexture = iEnv.getTexture();
            textureFormat = GL_RGB;
        }
        makeCubeMapTextures();
        
        if(useCache){
            string directry = "";
            if(cacheDirectry != ""){
                directry = cacheDirectry + "/";
            }
            makeCache(directry + "FCM_Cache_" + ofToString(baseSize) + "_" + path.getFileName(imagePath));
        }
    }
    ofEnableArbTex();
}

void ofxFilteredCubeMap::loadFromCache(string cachePath){
    ofDisableArbTex();
    if(cachePath.find(".hdr", 0) != string::npos || cachePath.find(".exr", 0) != string::npos ){
        fCacheImage.load(cachePath);
        textureFormat = GL_RGB32F;
        baseSize = fCacheImage.getWidth() / 3;
    } else {
        iCacheImage.load(cachePath);
        textureFormat = GL_RGB;
        baseSize = iCacheImage.getWidth() / 3;
    }
    
    maxMipLevel = log2(baseSize) + 1;
    for(int i = 0; i < 6; i++){
        iFilteredImages[i].assign(maxMipLevel, ofImage());
        fFilteredImages[i].assign(maxMipLevel, ofFloatImage());
    }
        
    int texWidth = baseSize;
    int texHeight = baseSize;
    
    int offsetX = 0;
    int offsetY = 0;
    
    for(int i=0; i<maxMipLevel; i++){
        if(i != 0){
            offsetY = baseSize * 2;
        }
        for(int j=0;j<6;j++){
            envFbo[j].allocate(texWidth, texHeight, textureFormat);
            if(textureFormat == GL_RGB32F) {
                envFbo[j].begin();
                ofClear(0);
                if(isOF090){
                    swapRBShader.begin();
                }
                fCacheImage.draw(-((j % 3) * texWidth + offsetX), -(floor(j / 3) * texHeight + offsetY));
                if(isOF090){
                    swapRBShader.end();
                }
                envFbo[j].end();
                ofFloatPixels _pix;
                envFbo[j].readToPixels(_pix);
                fFilteredImages[j][i].setFromPixels(_pix);
            }else{
                envFbo[j].begin();
                ofClear(0);
                iCacheImage.draw(-((j % 3) * texWidth + offsetX), -(floor(j / 3) * texHeight + offsetY));
                envFbo[j].end();
                ofPixels _pix;
                envFbo[j].readToPixels(_pix);
                iFilteredImages[j][i].setFromPixels(_pix);
            }
        }
        if(i > 0){
            offsetX += texWidth * 3;
        }
        texWidth /= 2;
        texHeight /= 2;
    }
    
    makeFilteredCubeMap();
    ofEnableArbTex();
}

void ofxFilteredCubeMap::swapRB(){
    swapRBFbo.allocate(fEnv.getWidth(), fEnv.getHeight(), GL_RGBA32F);
    swapRBFbo.begin();
    ofClear(0);
    swapRBShader.begin();
    fEnv.draw(0, 0);
    swapRBShader.end();
    swapRBFbo.end();
    envTexture = swapRBFbo.getTexture();
}

void ofxFilteredCubeMap::makeCubeMapTextures(){
    cacheWidth = baseSize * 3;
    cacheHeight = baseSize * 3;
    cacheFbo.allocate(cacheWidth, cacheHeight, textureFormat);
    
    ofPushStyle();
    ofVec3f target[6];
    target[0] = ofVec3f(90,0,0); // posx
    target[1] = ofVec3f(-90,0,0); //negx
    target[2] = ofVec3f(0,90,180); //posy
    target[3] = ofVec3f(0,-90,180); //negy
    target[4] = ofVec3f(-180,0,0); //posz
    target[5] = ofVec3f(0,0,0); //negz
    
    ofEnableDepthTest();
    ofDisableArbTex();
    
    sphereMesh = ofSpherePrimitive(2048, 100).getMesh();
    for(int i=0;i<sphereMesh.getNormals().size();i++){
        sphereMesh.setNormal(i, ofVec3f(-1.0, 1.0, 1.0) * sphereMesh.getVertex(i).normalize());
    }
    envSphereMesh = ofSpherePrimitive(2048, 100).getMesh();
    for(int i=0;i<envSphereMesh.getTexCoords().size();i++){
        envSphereMesh.setTexCoord(i, ofVec2f(1.0 - envSphereMesh.getTexCoord(i).x, 1.0 - envSphereMesh.getTexCoord(i).y));
    }
    
    for(int i=0;i<6;i++){
        envCam[i].setFov(90.0);
        envCam[i].setPosition(0, 0, 0);
        envFbo[i].allocate(baseSize, baseSize, textureFormat);
        envFbo[i].begin();
        ofClear(255);
        envCam[i].pan(target[i].x);
        envCam[i].tilt(target[i].y);
        envCam[i].roll(target[i].z);
        envCam[i].begin();
        envTexture.bind();
        envSphereMesh.draw();
        envTexture.unbind();
        envCam[i].end();
        envFbo[i].end();
        if(textureFormat == GL_RGB32F){
            ofFloatPixels _pix;
            envFbo[i].readToPixels(_pix);
            fEnvMapImages[i].setFromPixels(_pix);
        }else{
            ofPixels _pix;
            envFbo[i].readToPixels(_pix);
            iEnvMapImages[i].setFromPixels(_pix);
        }
    }
    
    ofPopStyle();
    ofDisableDepthTest();
    
    makeCubeMap();
    
    ofPushStyle();
    ofEnableDepthTest();
    
    for(int i=0; i<6; i++){
        int width = baseSize;
        int height = baseSize;
        for(int j=0; j<maxMipLevel; j++){
            envFbo[i].allocate(width, height, textureFormat);
            envFbo[i].begin();
            ofClear(255);
            
            envCam[i].begin();
            
            glActiveTexture( GL_TEXTURE0 + 1 );
            glEnable( GL_TEXTURE_CUBE_MAP );
            glBindTexture( GL_TEXTURE_CUBE_MAP, cubeMapID );
            
            shader.begin();
            shader.setUniform1i("envMap", 1);
            shader.setUniform1f("Roughness", ofMap(j, 0, maxMipLevel-1, 0.0, 1.0 ));
            sphereMesh.draw();
            shader.end();
            
            glActiveTexture( GL_TEXTURE0 + 1 );
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
            glDisable( GL_TEXTURE_CUBE_MAP );
            glActiveTexture( GL_TEXTURE0 );
            
            envCam[i].end();
            
            envFbo[i].end();
            
            if(textureFormat == GL_RGB32F){
                ofFloatPixels _pix;
                envFbo[i].readToPixels(_pix);
                fFilteredImages[i][j].setFromPixels(_pix);
            }else{
                ofPixels _pix;
                envFbo[i].readToPixels(_pix);
                iFilteredImages[i][j].setFromPixels(_pix);
            }
            width /= 2;
            height /= 2;
        }
    }
    ofPopStyle();
    
    makeFilteredCubeMap();
}

void ofxFilteredCubeMap::makeCubeMap(){
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    glGenTextures(1, &cubeMapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapID);
    
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if(textureFormat == GL_RGB32F){
        int width = fEnvMapImages[0].getWidth();
        int height = fEnvMapImages[0].getHeight();
        
        float * data_px, * data_nx, * data_py, * data_ny, * data_pz, * data_nz;
        
        data_px = fEnvMapImages[0].getPixels();
        data_py = fEnvMapImages[2].getPixels();
        data_pz = fEnvMapImages[4].getPixels();
        
        data_nx = fEnvMapImages[1].getPixels();
        data_ny = fEnvMapImages[3].getPixels();
        data_nz = fEnvMapImages[5].getPixels();
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_px); // positive x
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_py); // positive y
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_pz); // positive z
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_nx); // negative x
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_ny); // negative y
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_nz); // negative z
    }else{
        int width = iEnvMapImages[0].getWidth();
        int height = iEnvMapImages[0].getHeight();
        
        unsigned char * data_px, * data_nx, * data_py, * data_ny, * data_pz, * data_nz;
        
        data_px = iEnvMapImages[0].getPixels();
        data_py = iEnvMapImages[2].getPixels();
        data_pz = iEnvMapImages[4].getPixels();
        
        data_nx = iEnvMapImages[1].getPixels();
        data_ny = iEnvMapImages[3].getPixels();
        data_nz = iEnvMapImages[5].getPixels();
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_px); // positive x
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_py); // positive y
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pz); // positive z
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_nx); // negative x
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_ny); // negative y
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_nz); // negative z
    }
}

void ofxFilteredCubeMap::makeFilteredCubeMap(){
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    glGenTextures(1, &filteredCubeMapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, filteredCubeMapID);
    
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, maxMipLevel - 1);
    
    for(int i=0; i<maxMipLevel; i++){
        if(textureFormat == GL_RGB32F){
            int width = fFilteredImages[0][i].getWidth();
            int height = fFilteredImages[0][i].getHeight();
            
            float * data_px, * data_nx, * data_py, * data_ny, * data_pz, * data_nz;
            
            data_px = fFilteredImages[0][i].getPixels();
            data_py = fFilteredImages[2][i].getPixels();
            data_pz = fFilteredImages[4][i].getPixels();
            
            data_nx = fFilteredImages[1][i].getPixels();
            data_ny = fFilteredImages[3][i].getPixels();
            data_nz = fFilteredImages[5][i].getPixels();
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_px); // positive x
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_py); // positive y
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_pz); // positive z
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_nx); // negative x
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_ny); // negative y
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, textureFormat, width, height, 0, GL_RGB, GL_FLOAT, data_nz); // negative z
        }else{
            int width = iFilteredImages[0][i].getWidth();
            int height = iFilteredImages[0][i].getHeight();
            
            unsigned char * data_px, * data_nx, * data_py, * data_ny, * data_pz, * data_nz;
            
            data_px = iFilteredImages[0][i].getPixels();
            data_py = iFilteredImages[2][i].getPixels();
            data_pz = iFilteredImages[4][i].getPixels();
            
            data_nx = iFilteredImages[1][i].getPixels();
            data_ny = iFilteredImages[3][i].getPixels();
            data_nz = iFilteredImages[5][i].getPixels();
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, i, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_px); // positive x
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, i, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_py); // positive y
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, i, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pz); // positive z
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, i, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_nx); // negative x
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, i, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_ny); // negative y
            glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, i, textureFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_nz); // negative z
        }
    }
}

void ofxFilteredCubeMap::makeCache(string chachePath){
    int offsetX = 0;
    cacheFbo.begin();
    ofClear(0);
    if(textureFormat == GL_RGB32F){
        for(int i=0; i<maxMipLevel; i++){
            int texWidth = fFilteredImages[0][i].getWidth();
            int texHeight = fFilteredImages[0][i].getHeight();
            ofPushMatrix();
            if(i > 0){
                ofTranslate(offsetX, baseSize * 2);
                offsetX += texWidth * 3;
            }
            for(int j=0; j<6; j++){
                fFilteredImages[j][i].draw((j % 3) * texWidth, floor(j / 3) * texHeight, texWidth, texHeight);
            }
            ofPopMatrix();
        }
    }else{
        for(int i=0; i<maxMipLevel; i++){
            int texWidth = iFilteredImages[0][i].getWidth();
            int texHeight = iFilteredImages[0][i].getHeight();
            ofPushMatrix();
            if(i > 0){
                ofTranslate(offsetX, baseSize * 2);
                offsetX += texWidth * 3;
            }
            for(int j=0; j<6; j++){
                iFilteredImages[j][i].draw((j % 3) * texWidth, floor(j / 3) * texHeight, texWidth, texHeight);
            }
            ofPopMatrix();
        }
    }
    
    cacheFbo.end();
    if(textureFormat == GL_RGB32F){
        ofFloatImage img;
        ofFloatPixels pix;
        cacheFbo.getTexture().readToPixels(pix);
        img.setFromPixels(pix);
        img.save(chachePath);
    }else{
        ofImage img;
        ofPixels pix;
        cacheFbo.getTexture().readToPixels(pix);
        img.setFromPixels(pix);
        img.save(chachePath);
    }
}

void ofxFilteredCubeMap::bind(int pos){
    textureUnit = pos;
    glActiveTexture( GL_TEXTURE0 + pos );
    glEnable( GL_TEXTURE_CUBE_MAP );
    glBindTexture( GL_TEXTURE_CUBE_MAP, filteredCubeMapID );
}

void ofxFilteredCubeMap::unbind(){
    glActiveTexture( GL_TEXTURE0 + textureUnit );
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0 );
    glDisable( GL_TEXTURE_CUBE_MAP );
    glActiveTexture( GL_TEXTURE0 );
}

void ofxFilteredCubeMap::debug(int level){
    if(textureFormat == GL_RGB32F){
        fFilteredImages[0][level].draw(128 * 2, 128, 128, 128); //posX
        fFilteredImages[1][level].draw(0, 128, 128, 128); //negX
        fFilteredImages[2][level].draw(128, 0, 128, 128); //posY
        fFilteredImages[3][level].draw(128, 128 * 2, 128, 128); //negY
        fFilteredImages[4][level].draw(128, 128, 128, 128); //posZ
        fFilteredImages[5][level].draw(128 * 3, 128, 128, 128); //negZ
    } else {
        iFilteredImages[0][level].draw(128 * 2, 128, 128, 128); //posX
        iFilteredImages[1][level].draw(0, 128, 128, 128); //negX
        iFilteredImages[2][level].draw(128, 0, 128, 128); //posY
        iFilteredImages[3][level].draw(128, 128 * 2, 128, 128); //negY
        iFilteredImages[4][level].draw(128, 128, 128, 128); //posZ
        iFilteredImages[5][level].draw(128 * 3, 128, 128, 128); //negZ
    }
}

void ofxFilteredCubeMap::makeCube(){
    skyboxFaces[0].addVertex(ofVec3f(0.5, 0.5, -0.5));
    skyboxFaces[0].addVertex(ofVec3f(0.5, -0.5, -0.5));
    skyboxFaces[0].addVertex(ofVec3f(0.5, 0.5, 0.5));
    skyboxFaces[0].addVertex(ofVec3f(0.5, -0.5, 0.5));
    
    skyboxFaces[1].addVertex(ofVec3f(-0.5, 0.5, 0.5));
    skyboxFaces[1].addVertex(ofVec3f(-0.5, -0.5, 0.5));
    skyboxFaces[1].addVertex(ofVec3f(-0.5, 0.5, -0.5));
    skyboxFaces[1].addVertex(ofVec3f(-0.5, -0.5, -0.5));
    
    skyboxFaces[2].addVertex(ofVec3f(-0.5, 0.5, 0.5));
    skyboxFaces[2].addVertex(ofVec3f(-0.5, 0.5, -0.5));
    skyboxFaces[2].addVertex(ofVec3f(0.5, 0.5, 0.5));
    skyboxFaces[2].addVertex(ofVec3f(0.5, 0.5, -0.5));
    
    skyboxFaces[3].addVertex(ofVec3f(-0.5, -0.5, -0.5));
    skyboxFaces[3].addVertex(ofVec3f(-0.5, -0.5, 0.5));
    skyboxFaces[3].addVertex(ofVec3f(0.5, -0.5, -0.5));
    skyboxFaces[3].addVertex(ofVec3f(0.5, -0.5, 0.5));
    
    skyboxFaces[4].addVertex(ofVec3f(-0.5, 0.5, -0.5));
    skyboxFaces[4].addVertex(ofVec3f(-0.5, -0.5, -0.5));
    skyboxFaces[4].addVertex(ofVec3f(0.5, 0.5, -0.5));
    skyboxFaces[4].addVertex(ofVec3f(0.5, -0.5, -0.5));
    
    skyboxFaces[5].addVertex(ofVec3f(0.5, 0.5, 0.5));
    skyboxFaces[5].addVertex(ofVec3f(0.5, -0.5, 0.5));
    skyboxFaces[5].addVertex(ofVec3f(-0.5, 0.5, 0.5));
    skyboxFaces[5].addVertex(ofVec3f(-0.5, -0.5, 0.5));
                           
    for(int i=0; i<6; i++){
        skyboxFaces[i].addTriangle(0, 1, 2);
        skyboxFaces[i].addTriangle(2, 1, 3);
        skyboxFaces[i].addTexCoord(ofVec2f(0.0, 0.0));
        skyboxFaces[i].addTexCoord(ofVec2f(0.0, 1.0));
        skyboxFaces[i].addTexCoord(ofVec2f(1.0, 0.0));
        skyboxFaces[i].addTexCoord(ofVec2f(1.0, 1.0));
    }
}

void ofxFilteredCubeMap::drawSkyBox(int level, float size){
    for(int i=0; i<6; i++){
        ofPushMatrix();
        ofScale(size, size, size);
        if(textureFormat == GL_RGB32F){
            fFilteredImages[i][level].getTexture().bind();
            skyboxFaces[i].draw();
            fFilteredImages[i][level].getTexture().unbind();
        }else{
            iFilteredImages[i][level].getTexture().bind();
            skyboxFaces[i].draw();
            iFilteredImages[i][level].getTexture().unbind();
        }
        ofPopMatrix();
    }
}

void ofxFilteredCubeMap::drawSphere(){
    envTexture.bind();
    envSphereMesh.drawFaces();
    envTexture.unbind();
}

bool ofxFilteredCubeMap::isHDR(){
    if(textureFormat == GL_RGB32F){
        return true;
    }else{
        return false;
    }
}

int ofxFilteredCubeMap::getNumMips(){
    return maxMipLevel - 1;
}