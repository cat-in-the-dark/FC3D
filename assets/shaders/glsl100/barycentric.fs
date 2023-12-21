#version 100
#extension GL_OES_standard_derivatives : enable

precision mediump float;

// WARNING!!!!!
// Please do not delete unused varying from base shaders
// otherwise you'll get warnings in WebGL.

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 vbc;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

const float lineWidth = 1.0;

// This function is not currently used, but it can be useful
// to achieve a fixed width wireframe regardless of z-depth
float computeScreenSpaceWireframe (vec3 barycentric, float width) {
  vec3 dist = fwidth(barycentric);
  vec3 smoothed = smoothstep(dist * ((width * 0.5) - 0.5), dist * ((width * 0.5) + 0.5), barycentric);
  return 1.0 - min(min(smoothed.x, smoothed.y), smoothed.z);
}

void main() {
  float w = computeScreenSpaceWireframe(vbc, lineWidth);
  gl_FragColor = vec4(0.0, w, 0.0, 1.0);

  // vec4 texelColor = texture2D(texture0, fragTexCoord);
  // texelColor.x = texelColor.x * vbc.x;
  // gl_FragColor = texelColor*colDiffuse;
}