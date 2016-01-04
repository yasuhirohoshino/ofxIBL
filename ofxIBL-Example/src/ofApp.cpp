#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    cubemap[0].load("Barce_Rooftop_C_8k.jpg", 512, true, "filteredMapCache");
    cubemap[1].load("Arches_E_PineTree_8k.jpg", 512, true, "filteredMapCache");
    cubemap[2].load("BasketballCourt_8k.jpg", 512, true, "filteredMapCache");
    ofDisableArbTex();
    
    baseColorTex.load("tex1.png");
    normalTex.load("199_norm.JPG");
    
    material.baseColor = baseColor;
    material.useBaseColorMap = false;
    material.baseColorMap = &baseColorTex;
    material.useNormalMap = false;
    material.normalMap = &normalTex;
    material.normalVal = normalVal;
    material.textureRepeatTimes = ofVec2f(4.0, 2.0);

    baseColor.set("baseColor", ofColor(255), ofColor(0), ofColor(255));
    useBaseColorMap.set("useBaseColorMap", false);
    useNormalMap.set("useNormalMap", false);
    normalVal.set("normalVal", 1.0, 0.0, 1.0);
    environment.set("environment", 0, 0, 2);
    
    gui.setup("IBL");
    gui.add(baseColor);
    gui.add(useBaseColorMap);
    gui.add(useNormalMap);
    gui.add(normalVal);
    gui.add(environment);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    
    material.baseColor = baseColor;
    material.useBaseColorMap = useBaseColorMap;
    material.useNormalMap = useNormalMap;
    material.normalVal = normalVal;
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();
    cam.begin();
    ibl.begin(&cam, &cubemap[environment]);
    ibl.drawEnvironment(4000.0, 0.2);
    for(int i=0;i<10;i++){
        material.roughness = float(i) / 9.0;
        for(int j=0;j<10;j++){
            material.metallic = float(j) / 9.0;
            material.begin(ibl.getShaderPtr());
            ofDrawSphere(i * 100 - 450, 0, j * 100 - 450, 35);
            material.end();
        }
    }
    ibl.end();
    cam.end();
    ofDisableDepthTest();
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
