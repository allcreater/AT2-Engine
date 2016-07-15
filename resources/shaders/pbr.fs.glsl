vec3 computeLighting (in vec3 lightDir, in vec3 normal, in vec3 viewDir)
{
	return vec3(max(dot (normal, -lightDir), 0.0));
}