#version 400

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 normalMatrix;
uniform vec4 globalColor;

uniform mat4 viewTranspose;
uniform int useNormalMap;

in vec4  position;
in vec3  normal;
in vec2  texcoord;

out vec4 positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;

out vec4 colorVarying;
out vec3 v_normalVarying;
out vec4 v_positionVarying;

out vec3 reflectVec;
out mat3 mView;

void main() {
    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix)));
    
    normalVarying = normal;
    positionVarying = position;
    texCoordVarying = texcoord;
    colorVarying = globalColor;
    
    v_normalVarying = normalize(vec3(normalMatrix * normal));
    v_positionVarying = modelViewMatrix * position;
    
    if(useNormalMap == 1){
        vec3 norm = v_normalVarying;
        vec3 c1 = normalize(cross(normal, vec3(0.0, 0.0, 1.0)));
        vec3 c2 = normalize(cross(normal, vec3(0.0, 1.0, 0.0)));
        vec3 tang = vec3(0.0);
        if(length(c1) > length(c2)){
            tang = c1;
        }else{
            tang = c2;
        }
        vec3 binormal = normalize(cross(norm, tang));
        mView = mat3(tang, binormal, norm);
    }
    
    vec3 relfect0 = reflect(normalize(v_positionVarying.xyz), v_normalVarying);
    reflectVec = vec3(viewTranspose * vec4(relfect0, 0.0)) * vec3(1, 1, -1);
    
    texCoordVarying = texcoord;
    colorVarying = globalColor;
    
    gl_Position = modelViewProjectionMatrix * position;
}