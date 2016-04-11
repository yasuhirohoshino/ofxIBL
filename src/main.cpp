#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
    ofGLWindowSettings settings;
    settings.setGLVersion(4, 0);
    settings.width = 1024;
    settings.height = 768;
    ofCreateWindow(settings);
    ofRunApp(new ofApp());
}
