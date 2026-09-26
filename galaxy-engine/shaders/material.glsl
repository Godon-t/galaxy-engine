#type vertex
#version 420 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 v_texCoords;
out vec3 v_worldPos;
out vec3 v_normal;
out vec3 v_camPos;

void main()
{
    v_texCoords = texCoord;
    gl_Position = projection * view * model * vec4(vertices_position_modelspace, 1);

    v_normal            = normalize(mat3(transpose(inverse(model))) * normal);
    v_worldPos          = vec3(model * vec4(vertices_position_modelspace, 1.0));
    v_camPos            = vec3(inverse(view)[3]);
}

//////////////////////////////////////////////////////////////////////////////////////

#type fragment
#version 420 core

uniform bool useIrradianceMap;
uniform samplerCube irradianceMap;

uniform float zFar = 9999.0;

uniform vec3 albedoVal        = vec3(1.0, 0.f, 0.f);
uniform float metallicVal     = 0.5f;
uniform float roughnessVal    = 0.5f;
uniform float aoVal           = 1.f;
uniform float transparencyVal = 1.0f;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform bool useAlbedoMap    = false;
uniform bool useNormalMap    = false;
uniform bool useMetallicMap  = false;
uniform bool useRoughnessMap = false;
uniform bool useAoMap        = false;


in vec2 v_texCoords;
in vec3 v_worldPos;
in vec3 v_normal;
in vec3 v_camPos;

layout(location = 0) out vec4 gAlbedo;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gMaterial;



vec3 getNormalFromNormalMap()
{
    // Récupère la normale en espace tangent depuis la texture
    vec3 tangentNormal = texture2D(normalMap, v_texCoords).rgb * 2.0 - 1.0;

    // Calcule les dérivées pour construire la matrice TBN
    vec3 Q1  = dFdx(v_worldPos);
    vec3 Q2  = dFdy(v_worldPos);
    vec2 st1 = dFdx(v_texCoords);
    vec2 st2 = dFdy(v_texCoords);

    vec3 N   = normalize(v_normal);
    vec3 T   = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


/*--------------------------------------PBR--------------------------------------*/

void main()
{
    vec3 albedo, normal;
    float metallic, roughness, ao;

    float transparency = useAlbedoMap ? texture(albedoMap, v_texCoords).a : transparencyVal;
    albedo             = useAlbedoMap ? texture(albedoMap, v_texCoords).rgb : albedoVal;
    normal             = useNormalMap ? getNormalFromNormalMap() : v_normal;
    metallic           = useMetallicMap ? texture(metallicMap, v_texCoords).r : metallicVal;
    roughness          = useRoughnessMap ? texture(roughnessMap, v_texCoords).r : roughnessVal;
    ao                 = useAoMap ? texture(aoMap, v_texCoords).r : aoVal;

    if(transparency > 0.1){
        // gDepth = vec4(length(v_camPos - v_worldPos) / zFar, ao, 0, 1);
        gAlbedo.rgb = albedo;
        gAlbedo.a   = transparency;

        gNormal.rgb = (normal + vec3(1.0)) * 0.5;
        gNormal.a = 1.0;

        gMaterial = vec4(roughness, metallic, ao, 0.0);
    } else {
        discard;
    }
}
