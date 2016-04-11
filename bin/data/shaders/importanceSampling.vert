#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

in vec4  position;
in vec3  normal;
in vec2  texcoord;

out vec4 positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;

void main() {
    normalVarying = normal;
    positionVarying = position;
    gl_Position = modelViewProjectionMatrix * position;
}