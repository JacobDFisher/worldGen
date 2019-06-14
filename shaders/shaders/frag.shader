#version 120

varying vec4 color_from_vshader;

void main() {
	gl_FragColor = color_from_vshader;
}
