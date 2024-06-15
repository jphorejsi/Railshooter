#version 330 core
out vec4 FragColor;

uniform float lifeSpan;

void main()
{
	float opacity = lifeSpan / 3.0f; // Calculate opacity based on lifeSpan
	FragColor = vec4(1.0, 0.0f, 0.0f, opacity);
}