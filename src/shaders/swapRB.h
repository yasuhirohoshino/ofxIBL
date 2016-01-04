#pragma once
#include "ofMain.h"

#define STRINGIFY(A) #A

class SwapRB{
public:
    string gl3VertShader;
    string gl3FragShader;
    
    SwapRB(){
        gl3VertShader = "#version 150\n";
        gl3VertShader += STRINGIFY(uniform mat4 modelViewProjectionMatrix;
                                   
                                   in vec4  position;
                                   in vec4  color;
                                   in vec3  normal;
                                   in vec2  texcoord;
                                   
                                   out vec2 texCoordVarying;
                                   
                                   void main() {
                                       gl_Position = modelViewProjectionMatrix * position;
                                       texCoordVarying = texcoord;
                                   });
        
        gl3FragShader = "#version 150\n";
        gl3FragShader += STRINGIFY(uniform sampler2D tex;
                                   in vec2 texCoordVarying;
                                   
                                   out vec4 fragColor;
                                   
                                   void main (void) {
                                       fragColor = texture(tex, texCoordVarying).bgra;
                                   });
    }
};