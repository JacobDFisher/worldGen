#version 120

attribute vec4 position;
attribute vec3 color;
varying vec4 color_from_vshader;
uniform float mode;
uniform float enhance;

void main() {
	gl_Position = position;
	if(mode<=0.0){
	  color_from_vshader = vec4(color*enhance, gl_Position.z>=0);
	} else {
	  float color_val;
	  if(mode>=1.0&&mode<2.0) {
	    color_val = enhance*color.x;
	  } else if(mode>=2.0&&mode<3.0){
	    color_val = enhance*color.y;
	  } else if(mode>=3.0&&mode<4.0){
	    color_val = enhance*color.z;
	  } else {
	    color_val = enhance*(color.x+color.y+color.z)/3;
	  }
	  color_from_vshader = vec4(color_val, color_val, color_val, gl_Position.z>=0);
	}
}
