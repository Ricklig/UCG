#version 330 core

in vec3 outNormal;
in vec3 fragPosition;
in vec2 outUV;

out vec4 color;

uniform sampler2D cubeTexture;

void main()
{
    vec3 lightColour = vec3(1.0f, 1.0f, 1.0f);
	vec3 cubeColour = vec3(texture(cubeTexture, outUV));
	
	//ambient lighting
	float ambientStrength = 0.15f;
    vec3 ambient_contribution = ambientStrength * lightColour;
	
	//diffuse lighting
	vec3 light_position = vec3(0.0f, 20.0f, 2.0f); //world coords

	vec3 norm = normalize(outNormal);
	
	vec3 light_direction = normalize(light_position - fragPosition);
	float incident_degree = max(dot(norm, light_direction), 0.0f);
	vec3 diffuse_contribution = incident_degree * lightColour;
	
	vec3 resultantColour = (ambient_contribution + diffuse_contribution) * cubeColour;
    color = vec4(resultantColour, 1.0f);
} 