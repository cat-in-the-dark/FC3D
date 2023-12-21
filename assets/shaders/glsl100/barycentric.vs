#version 100

// WARNING!!!!!
// Please do not delete unused varying from base shaders
// otherwise you'll get warnings in WebGL.

// Input vertex attributes
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

attribute vec2 barycentric;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

varying vec3 vbc;

void main() {
  fragTexCoord = vertexTexCoord;
  fragColor = vertexColor;

  vbc.xy = barycentric.xy;
  vbc.z = 1.0 - barycentric.x - barycentric.y;  
  gl_Position = mvp*vec4(vertexPosition, 1.0);
}