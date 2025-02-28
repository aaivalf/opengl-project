#version 410 core


in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;
in vec2 fragTexCoords;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform	vec3 lightColor;
uniform	vec3 lightDir;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//FLASHLIGHT
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointLightRadius;
uniform int pointLightEnabled;
//END FLASHLIGHT

vec3 ambient;

vec3 diffuse;
vec3 specular;
uniform float specularStrength = 0.5f;
uniform float shininess = 32.0f;
uniform float ambientStrength = 0.2f;

uniform float showFog;
uniform vec3 lightPos;
vec3 o;



void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

	 // Flashlight calculations
    if (pointLightEnabled != 0) { // Check if flashlight is enabled
        vec3 pointLightDir = pointLightPos - fragPosEye.xyz;
        float distance = length(pointLightDir);

        if (distance < pointLightRadius) {
            pointLightDir = normalize(pointLightDir);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance); // Improved attenuation
            float pointDiffuse = max(dot(normalEye, pointLightDir), 0.0f);
            vec3 pointHalfVector = normalize(pointLightDir + viewDirN);
            float pointSpecCoeff = pow(max(dot(pointHalfVector, normalEye), 0.0f), shininess);
            
            ambient += ambientStrength * pointLightColor * attenuation;
            diffuse += pointDiffuse * pointLightColor * attenuation;
            specular += specularStrength * pointSpecCoeff * pointLightColor * attenuation;
        }
    }
    
}

float computeShadow()
{
    float bias = 0.005f;	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

float computeFog()
{
 float fogDensity;
 if(showFog == 0.0f)
	fogDensity = 0.05f;
 else
	fogDensity = 0.0f;
 float fragmentDistance = length(fragPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	computeLightComponents();
	
	float shadow = computeShadow();
	
	//modulate with diffuse map
	ambient *= vec3(texture(diffuseTexture, fragTexCoords));
	diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
	//modulate woth specular map
	specular *= vec3(texture(specularTexture, fragTexCoords));
	
	//modulate with shadow
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	vec4 colorTest = vec4(color, 1.0f);
    	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.48627451f, 0.737254902f, 0.8f, 1.0f);
	fColor = mix(fogColor, colorTest, fogFactor);



}
