#version 330 core
out vec4 FragColor;

struct DirectionalLight {
    vec3 direction;
    vec3 specular;
    vec3 diffuse;
    vec3 ambient;
};

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_height1;
    sampler2D texture_normal1;
    vec3 specular;
    float shininess;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 Tangent;
in vec3 Bitangent;

uniform DirectionalLight directionalLight;
uniform Material material;
uniform vec3 viewPosition;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    // Obtain height from the height map
    float height = texture(material.texture_height1, texCoords).r;
    // Scale and bias to create the displacement effect
    float parallaxScale = 0.05;  // Adjust this value to control the strength of the effect
    vec2 p = viewDir.xy * (height * parallaxScale);
    return texCoords - p;
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);
}

void main()
{
    // Adjust the texture coordinates using parallax mapping
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec2 parallaxTexCoords = ParallaxMapping(TexCoords, viewDir);

    // Compute tangent space matrix
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    vec3 N = normalize(Normal);

    mat3 TBN = mat3(T, B, N);

    // Fetch normal and diffuse texture after parallax mapping adjustment
    vec3 normal = texture(material.texture_normal1, parallaxTexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);  // Convert to [-1, 1] range
    normal = normalize(TBN * normal); // Transform normal to world space
    vec3 result = CalcDirectionalLight(directionalLight, normal, FragPos, viewDir);
    FragColor = vec4(result, 1.0);
}
