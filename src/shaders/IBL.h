#pragma once
#include "ofMain.h"

#define STRINGIFY(A) #A

class IBL{
public:
    string gl3VertShader;
    string gl3FragShader;
    
    IBL(){
        gl3VertShader = "#version 330\n";
        gl3VertShader += STRINGIFY(
                                   uniform mat4 projectionMatrix;
                                   uniform mat4 modelViewMatrix;
                                   uniform mat4 textureMatrix;
                                   uniform mat4 modelViewProjectionMatrix;
                                   uniform vec4 globalColor;
                                   
                                   uniform mat4 viewTranspose;
                                   uniform int useNormalMap;
                                   
                                   in vec4  position;
                                   in vec4  color;
                                   in vec3  normal;
                                   in vec2  texcoord;
                                   
                                   out vec4 colorVarying;
                                   out vec2 texCoordVarying;
                                   
                                   out vec3 normalVarying;
                                   out vec4 positionVarying;
                                   
                                   out vec3 v_normalVarying;
                                   out vec4 v_positionVarying;
                                   
                                   out vec3 reflectVec;
                                   out mat3 mView;
                                   out vec3 viewVec;
                                   out mat4 normalMatrix;
                                   
                                   void main() {
                                       normalVarying = normal;
                                       positionVarying = position;
                                       
                                       normalMatrix = transpose(inverse(modelViewMatrix));
                                       
                                       v_normalVarying = normalize(vec3(normalMatrix * vec4(normal, 1.0)));
                                       v_positionVarying = modelViewMatrix * vec4(position.xyz, 1.0);
                                       
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
                                           viewVec.x = dot(tang, -position.xyz);
                                           viewVec.y = dot(binormal, -position.xyz);
                                           viewVec.z = dot(norm, -position.xyz);
                                           viewVec = normalize(viewVec);
                                       }
                                       
                                       vec3 relfect0 = reflect(normalize(v_positionVarying.xyz), v_normalVarying);
                                       reflectVec = vec3(viewTranspose * vec4(relfect0, 0.0)) * vec3(1, 1, -1);
                                       
                                       texCoordVarying = texcoord;
                                       colorVarying = globalColor;
                                       
                                       gl_Position = modelViewProjectionMatrix * position;
                                   });
    
        gl3FragShader = "#version 330\n";
        gl3FragShader += STRINGIFY(
                                   const float PI = 3.14159265358979;
                                   const float TwoPI = 6.28318530718;
                                   
                                   uniform samplerCube envMap;
                                   uniform vec4 baseColorUniform;
                                   uniform float roughness;
                                   uniform float metallic;
                                   uniform float normalVal;
                                   uniform float cameraNear;
                                   uniform float cameraFar;
                                   uniform mat4 viewTranspose;
                                   
                                   uniform int numMips;
                                   
                                   uniform int useBaseColorMap;
                                   uniform sampler2D baseColorMap;
                                   uniform int useRoughnessMap;
                                   uniform sampler2D roughnessMap;
                                   uniform int useMetallicMap;
                                   uniform sampler2D metallicMap;
                                   uniform int useNormalMap;
                                   uniform sampler2D normalMap;
                                   uniform int useOcclusionMap;
                                   uniform sampler2D occlusionMap;
                                   uniform int useEmissionMap;
                                   uniform sampler2D emissionMap;
                                   uniform int useDetailBaseColorMap;
                                   uniform sampler2D detailBaseColorMap;
                                   uniform int useDetailNormalMap;
                                   uniform sampler2D detailNormalMap;
                                   uniform int useGlobalColor;
                                   
                                   uniform vec2 textureRepeatTimes;
                                   uniform vec2 detailTextureRepeatTimes;
                                   
                                   uniform int isHDR;
                                   
                                   uniform int drawEnv;
                                   uniform float envLevel;
                                   
                                   in vec4 colorVarying;
                                   in vec2 texCoordVarying;
                                   
                                   in vec3 normalVarying;
                                   in vec4 positionVarying;
                                   in vec3 v_normalVarying;
                                   in vec4 v_positionVarying;
                                   
                                   in vec3 reflectVec;
                                   in mat3 mView;
                                   in vec3 viewVec;
                                   in mat4 normalMatrix;
                                   
                                   layout (location = 0) out vec4 fragColor;
                                   layout (location = 1) out vec4 normalOut;
                                   layout (location = 2) out vec4 depthOut;
                                   
                                   vec3 PrefilterEnvMap(float Roughness, vec3 R) {
                                       vec4 color = mix(textureLod( envMap, R, int(Roughness * numMips) ), textureLod( envMap, R, min(int(Roughness * numMips) + 1, numMips)), fract(Roughness * numMips));
                                       return color.rgb;
                                   }
                                   
                                   vec3 EnvBRDFApprox( vec3 SpecularColor, float Roughness, float NoV ) {
                                       vec4 c0 = vec4( -1, -0.0275, -0.572, 0.022 );
                                       vec4 c1 = vec4( 1, 0.0425, 1.04, -0.04 );
                                       vec4 r = Roughness * c0 + c1;
                                       float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
                                       vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
                                       return SpecularColor * AB.x + AB.y;
                                   }
                                   
                                   vec3 ApproximateSpecularIBL(vec3 SpecularColor, float Roughness, vec3 N, vec3 V, vec3 ReflectDir) {
                                       float NoV = dot(N, V);
                                       vec3 PrefilteredColor = PrefilterEnvMap( Roughness, ReflectDir );
                                       return SpecularColor * PrefilteredColor * EnvBRDFApprox(SpecularColor, Roughness, NoV);
                                   }
                                   
                                   vec3 Fresnel(vec3 N, vec3 V, float Roughness, vec3 ReflectDir, float f0) {
                                       float base = 1.0 - clamp(dot(N, V), 0.0, 0.99);
                                       float exponential = pow(base, 5.0);
                                       float fresnel = f0 + (1.0 - f0) * exponential;
                                       vec3 reflectColor = PrefilterEnvMap(Roughness, ReflectDir);
                                       return reflectColor * fresnel;
                                   }
                                   
                                   vec3 blendSoftLight(vec3 Base, vec3 Blend) {
                                       return mix(
                                                  sqrt(Base) * (2.0 * Blend - 1.0) + 2.0 * Base * (1.0 - Blend),
                                                  2.0 * Base * Blend + Base * Base * (1.0 - 2.0 * Blend),
                                                  step(Base, vec3(0.5))
                                                  );
                                   }
                                   
                                   float luma(vec3 Color){
                                       return dot(Color, vec3(0.2126, 0.7152, 0.0722));
                                   }
                                   
                                   void main (void){
                                       if(drawEnv == 1){
                                           fragColor = vec4(PrefilterEnvMap(envLevel, normalVarying), 1.0);
                                           normalOut = vec4(1.0);
                                           depthOut = vec4(1.0);
                                       } else {

                                           float gamma = 1.0;
                                           if(isHDR == 1){
                                               gamma = 2.2;
                                           }
                                           
                                           vec3 baseColor = vec3(1.0);
                                           vec3 baseColorFromParam = vec3(1.0);
                                           if(useGlobalColor == 1){
                                               baseColorFromParam = colorVarying.rgb;
                                           }else{
                                               baseColorFromParam = baseColorUniform.rgb;
                                           }
                                           if(useBaseColorMap == 1){
                                               vec3 baseColorFromTex = texture(baseColorMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).rgb;
                                               if(useDetailBaseColorMap == 1){
                                                   vec3 detailBaseColor = texture(detailBaseColorMap, mod(texCoordVarying * detailTextureRepeatTimes, 1.0)).rgb;
                                                   baseColor = pow(blendSoftLight(baseColorFromTex, detailBaseColor) * baseColorFromParam.rgb, vec3(gamma));
                                               } else {
                                                   baseColor = pow(baseColorFromTex * baseColorFromParam.rgb, vec3(gamma));
                                               }
                                           } else {
                                               baseColor = pow(baseColorFromParam.rgb, vec3(gamma));
                                           }
                                           
                                           float roughnessVal = 0.0;
                                           if(useRoughnessMap == 1){
                                               roughnessVal = texture(roughnessMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).r;
                                           } else {
                                               roughnessVal = roughness;
                                           }
                                           
                                           float metallicVal = 1.0;
                                           if(useMetallicMap == 1){
                                               metallicVal = texture(metallicMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).r;
                                           } else {
                                               metallicVal = metallic;
                                           }
                                           
                                           vec3 normal = vec3(0.0);
                                           vec3 reflectDir = vec3(0.0);
                                           vec3 viewDir = normalize(-v_positionVarying.xyz);
                                           if(useNormalMap == 1){
                                               vec3 normalMapVec = texture(normalMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).rgb;
                                               if(useDetailNormalMap == 1){
                                                   vec3 detailNormalMapVec = texture(detailNormalMap, mod(texCoordVarying * detailTextureRepeatTimes, 1.0)).rgb;
                                                   normal = mView * ((mix(vec3(0.5,0.5,1.0), blendSoftLight(normalMapVec, detailNormalMapVec), vec3(normalVal)) - 0.5) * 2.0).rgb;
                                               }else{
                                                   normal = mView * ((mix(vec3(0.5,0.5,1.0), normalMapVec, vec3(normalVal)) - 0.5) * 2.0).rgb;
                                               }
                                               vec3 relfect0 = reflect(normalize(v_positionVarying.xyz), normal);
                                               reflectDir = vec3(viewTranspose * vec4(relfect0, 0.0)) * vec3(1, 1, -1);
                                           } else {
                                               normal = v_normalVarying;
                                               reflectDir = reflectVec;
                                           }
                                           
                                           float occlusion = 1.0;
                                           if(useOcclusionMap == 1){
                                               occlusion = pow(texture(occlusionMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).r, gamma);
                                           }
                                           
                                           vec3 diffuseColor = baseColor - baseColor * metallicVal;
                                           vec3 specularColor = mix(vec3(0.01), baseColor, metallicVal);
                                           vec3 diffuse = textureLod(envMap, normal, int(numMips * 2 / 3)).rgb * diffuseColor;
                                           vec3 specular = ApproximateSpecularIBL(specularColor, roughnessVal, normal, viewDir, reflectDir);
                                           vec3 fresnel = Fresnel(normal, normalize(-v_positionVarying.xyz), roughnessVal, reflectVec, 0.02) * (1.0 - metallicVal);
                                           
                                           if(useEmissionMap == 1){
                                               vec3 emissionColor = pow(texture(emissionMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).rgb, vec3(gamma));
                                               if(luma(emissionColor) > 0.0 ){
                                                   fragColor = vec4(emissionColor, 1.0);
                                               } else{
                                                   fragColor = vec4(vec3(diffuse + specular + fresnel) * occlusion, 1.0);
                                               }
                                           } else {
                                               fragColor = vec4(vec3(diffuse + specular + fresnel) * occlusion, 1.0);
                                           }
                                           normalOut = vec4(normal, 1.0);
                                           depthOut = vec4(-v_positionVarying.z / (cameraFar - cameraNear), 1.0, 1.0, 1.0);
                                       }
                                   });
    }
};