#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct DirectionalLight {
    vec3 direction;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

};

struct Material {
    float shininess;
    vec3 specular;
};


in vec3 Normal;
in vec3 FragPos;

uniform DirectionalLight directionalLight;
uniform Material material;
uniform vec3 viewPosition;
uniform float alpha;
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}
void main() {
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 lighting = CalcDirectionalLight(directionalLight, normal, FragPos, viewDir);
    vec3 whiteColor = vec3(1.0f,1.0f,1.0f);
    vec3 yellowColor = vec3(0.9f,1.0f,0.6f)*0.2;
    vec3 finalColor = mix(whiteColor,yellowColor,0.1);
    vec3 result = finalColor * lighting;
    FragColor = vec4(result, alpha);
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0) {
        BrightColor = vec4(result, 1.0f);
    } else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0f);
    }
}