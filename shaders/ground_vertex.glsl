#version 400 core

in vec3 pos;

uniform mat4 object;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * object * vec4(pos, 1.0f);
}