#type vertex
#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 texCoord;

out vec2 texCoords;

void main()
{
    texCoords   = texCoord;
    gl_Position = vec4(vertices_position_modelspace, 1);
}

#type fragment
#version 330 core

// https://www.shadertoy.com/view/MdsfzN

vec2 octahedral_mapping(vec3 co)
{
    // projection onto octahedron
    co /= dot(vec3(1), abs(co));

    // out-folding of the downward faces
    if (co.y < 0.0) {
        co.xy = (1.0 - abs(co.zx)) * sign(co.xz);
    }

    // mapping to [0;1]ˆ2 texture space
    return co.xy * 0.5 + 0.5;
}

vec3 octahedral_unmapping(vec2 co)
{
    co = co * 2.0 - 1.0;

    vec2 abs_co = abs(co);
    vec3 v      = vec3(co, 1.0 - (abs_co.x + abs_co.y));

    if (abs_co.x + abs_co.y > 1.0) {
        v.xy = (abs(co.yx) - 1.0) * -sign(co.xy);
    }

    return v;
}
////////////////////////////////////////////////

uniform samplerCube radianceCubemap;
uniform samplerCube depthCubemap;

uniform vec3 debugStart;
uniform vec3 debugEnd;
uniform vec3 debugProbePos;

in vec2 texCoords;
out vec4 color;

uniform float zNear = 0.1;
uniform float zFar  = 9999.0;
float linearDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

/////////////////////////////////////////////////////////////////////
const int HIT     = 0;
const int MISS    = 1;
const int UNKNOWN = 2;

// retourne le vecteur non normalisé (avant normalize)
vec3 octahedral_unmapping_unnorm(vec2 co)
{
    co = co * 2.0 - 1.0;

    vec2 abs_co = abs(co);
    vec3 v      = vec3(co, 1.0 - (abs_co.x + abs_co.y));

    if (abs_co.x + abs_co.y > 1.0) {
        v.xy = (abs(co.yx) - 1.0) * -sign(co.xy);
    }

    return v;
}

// Calcule la distance d'un point p à un segment ab en 2D
float distPointSegment(vec2 p, vec2 a, vec2 b)
{
    vec2 pa   = p - a;
    vec2 ba   = b - a;
    float h   = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    vec2 proj = a + ba * h;
    return length(p - proj);
}

// Applique un effet miroir aux bords et retourne des UV dans [0,1]
// Permet de gérer les continuités entre triangles adjacents
vec2 mirrorAndWrapUV(vec2 uv)
{
    vec2 m  = abs(uv - 0.5) + 0.5;
    vec2 f  = floor(m);
    float x = f.x - f.y;
    if (x != 0.0) {
        uv = 1.0 - uv;
    }
    return fract(uv);
}

// Échantillonne la cubemap en utilisant le dépliage octaédral
vec3 sampleCubemapFromOctaUV(vec2 uv, samplerCube cubemap)
{
    vec3 unnorm = octahedral_unmapping_unnorm(uv);
    vec3 dir    = normalize(unnorm);
    return texture(cubemap, dir).rgb;
}

// Calcule la distance minimale d'un point UV aux arêtes des triangles octaédraux
// Permet de dessiner les bords des 8 triangles de la carte octaédrale
float computeMinEdgeDistance(vec2 uv)
{
    float minDist = 1e6;
    // Parcourir les 8 triangles (combinaisons de signes pour x, y, z)
    for (int sx = -1; sx <= 1; sx += 2) {
        for (int sy = -1; sy <= 1; sy += 2) {
            for (int sz = -1; sz <= 1; sz += 2) {
                // Sommets du triangle sur l'octaèdre
                vec3 v0 = vec3(float(sx), 0.0, 0.0);
                vec3 v1 = vec3(0.0, float(sy), 0.0);
                vec3 v2 = vec3(0.0, 0.0, float(sz));

                // Projeter en UV
                vec2 t0 = octahedral_mapping(v0);
                vec2 t1 = octahedral_mapping(v1);
                vec2 t2 = octahedral_mapping(v2);

                // Distance aux 3 arêtes du triangle
                float d0 = distPointSegment(uv, t0, t1);
                float d1 = distPointSegment(uv, t1, t2);
                float d2 = distPointSegment(uv, t2, t0);

                minDist = min(minDist, min(d0, min(d1, d2)));
            }
        }
    }
    return minDist;
}

// Convertit une distance UV en alpha pour tracer une ligne
// Prend en compte la résolution d'écran pour une épaisseur de ligne constante
float lineAlphaFromDist(float minDist)
{
    vec2 iResolution = textureSize(radianceCubemap, 0).xy;
    // Échelle de pixel pour adapter l'épaisseur de ligne
    float pixelScale = length(fwidth(gl_FragCoord.xy / iResolution.xy)) * length(iResolution.xy);
    float lineWidth  = 0.8 * pixelScale / iResolution.x;
    lineWidth        = max(lineWidth, 0.005);
    float lineAlpha  = 1.0 - smoothstep(0.0, lineWidth, minDist);
    return pow(lineAlpha, 0.9);
}

float safeLength(vec3 v)
{
    return length(v);
}

// Normalisation sécurisée : utilise un vecteur de secours si v est trop petit
vec3 safeNormalize(vec3 v, vec3 fallback)
{
    float l = length(v);
    if (l < 1e-5)
        return normalize(fallback);
    return v / l;
}

// détermine dans quel triangle octahedral se trouve une direction 3D
// retourne un indice de 0 à 7 (8 triangles)

/*        schémat si je dis pas de bétise c'est les bon indices
|-----------------------------------------------------------------------|
|           4           /           |           \           5           |
|                  /                |                \                  |
|        /               0          |          1               \        |
|-----------------------------------|-----------------------------------|
|        \               2          |          3               /        |
|                  \                |                /                  |
|           6           \           |           /           7           |
|-----------------------------------------------------------------------|


ou


|-----------------------------------------------------------------------|
|           5           /           |           \           4           |
|                  /                |                \                  |
|        /               1          |          0               \        |
|-----------------------------------|-----------------------------------|
|        \               3          |          2               /        |
|                  \                |                /                  |
|           7           \           |           /           6           |
|-----------------------------------------------------------------------|

*/
int getOctahedralTriangle(vec3 dir)
{
    vec3 absDir = abs(dir);
    int idx     = 0;
    if (dir.x < 0.0)
        idx |= 1;
    if (dir.y < 0.0)
        idx |= 2;
    if (dir.z < 0.0)
        idx |= 4;
    return idx;
}

// Calcule l'intersection d'un segment 3D avec les faces de l'octaèdre
// Retourne tous les paramètres t où le segment change de triangle
int computeOctahedralIntersections(vec3 p0, vec3 p1, out float ts[9])
{
    ts[0]     = 0.0;
    ts[1]     = 1.0;
    int count = 2;

    // Les 3 plans principaux x=0, y=0, z=0
    for (int i = 0; i < 3; ++i) {
        float a = (i == 0) ? p0.x : (i == 1) ? p0.y
                                             : p0.z;
        float b = (i == 0) ? p1.x : (i == 1) ? p1.y
                                             : p1.z;
        float d = b - a;
        if (abs(d) > 1e-6) {
            float t = -a / d;
            if (t > 1e-5 && t < 1.0 - 1e-5) {
                ts[count] = t;
                count     = count + 1;
            }
        }
    }

    // Les 6 plans diagonaux qui séparent les triangles adjacents
    // Ces plans passent par l'origine et ont pour normales les combinaisons de signes
    vec3 normals[6];
    normals[0] = vec3(1.0, 1.0, 1.0);
    normals[1] = vec3(1.0, 1.0, -1.0);
    normals[2] = vec3(1.0, -1.0, 1.0);
    normals[3] = vec3(1.0, -1.0, -1.0);
    normals[4] = vec3(-1.0, 1.0, 1.0);
    normals[5] = vec3(-1.0, 1.0, -1.0);

    for (int i = 0; i < 6; ++i) {
        vec3 normal = normals[i];
        float a     = dot(p0, normal);
        float b     = dot(p1, normal);
        float d     = b - a;
        if (abs(d) > 1e-6) {
            float t = -a / d;
            if (t > 1e-5 && t < 1.0 - 1e-5) {
                ts[count] = t;
                count     = count + 1;
            }
        }
    }

    // Tri par insertion
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (ts[i] > ts[j]) {
                float tmp = ts[i];
                ts[i]     = ts[j];
                ts[j]     = tmp;
            }
        }
    }

    // Suppression des doublons
    int m = 0;
    for (int i = 0; i < count; ++i) {
        if (i == 0 || abs(ts[i] - ts[m - 1]) > 1e-4) {
            ts[m] = ts[i];
            m     = m + 1;
        }
    }

    return m;
}

// Calcule la distance minimale d'un point UV à un segment 3D (p0->p1)
// projeté sur la carte octahedral. La trajectoire est subdivisée finement
// pour capturer la courbure de la projection tout en détectant les discontinuités
// aux bords de l'UV map (quand le rayon passe d'un côté à l'autre du au voisin).
float computeMinDistLinePolyline(vec2 uv, vec3 p0, vec3 p1)
{
    float minDist    = 1e6;
    vec3 fallbackDir = normalize(p1 - p0 + vec3(1e-6));

    // subdiviser le segment 3D pour avoir une courbe continue
    int totalSubdivisions = 256;

    // création des segments pour former le segments final
    for (int i = 0; i < totalSubdivisions; ++i) {
        float t0 = float(i) / float(totalSubdivisions);
        float t1 = float(i + 1) / float(totalSubdivisions);

        // points 3D le long du segment actuelle
        vec3 p3d0 = mix(p0, p1, t0);
        vec3 p3d1 = mix(p0, p1, t1);

        // obtenir des directions
        vec3 dir0 = safeNormalize(p3d0, fallbackDir);
        vec3 dir1 = safeNormalize(p3d1, fallbackDir);

        // projeter sur la carte octahedral
        vec2 uv0 = octahedral_mapping(dir0);
        vec2 uv1 = octahedral_mapping(dir1);

        // vérifier si on traverse un bord (discontinuiter)
        vec2 delta     = uv1 - uv0;
        float maxDelta = max(abs(delta.x), abs(delta.y));

        if (maxDelta < 0.5) {
            // subdiviser encore ce micro-segment pour capturer la courbure en UV
            // (la projection octahedral n'est pas linéaire)
            int microSubs = 4;
            for (int j = 0; j < microSubs; ++j) {

                float s0 = float(j) / float(microSubs);
                float s1 = float(j + 1) / float(microSubs);

                float ta = mix(t0, t1, s0);
                float tb = mix(t0, t1, s1);

                vec3 pa = mix(p0, p1, ta);
                vec3 pb = mix(p0, p1, tb);

                vec3 dira = safeNormalize(pa, fallbackDir);
                vec3 dirb = safeNormalize(pb, fallbackDir);

                vec2 uva = octahedral_mapping(dira);
                vec2 uvb = octahedral_mapping(dirb);

                float d = distPointSegment(uv, uva, uvb);
                minDist = min(minDist, d);
            }
        }
        // Sinon, c'est une discontinuité aux bords de l'UV map, on ne trace pas
    }

    return minDist;
}

int traceRayInProbe(vec3 p0, vec3 p1, sampler2D depthTex)
{
    vec3 fallbackDir      = normalize(p1 - p0 + vec3(1e-6));
    int totalSubdivisions = 8;

    for (int i = 0; i < totalSubdivisions; ++i) {
        float t  = float(i) / float(totalSubdivisions);
        vec3 p3d = mix(p0, p1, t);

        // direction normalisée
        vec3 dir = safeNormalize(p3d, fallbackDir);

        // projection octahedral
        vec2 uv = octahedral_mapping(dir);

        // profondeur du rayon (distance radiale depuis le centre du probe)
        float depthRay = length(p3d);

        // profondeur stockée dans la probe (texture de distances radiales)
        float depthProbe = texture(depthTex, uv).r;

        // --- logique HIT / MISS / UNKNOWN ---
        if (depthRay <= depthProbe + 1e-3) {
            return HIT; // HIT
        } else if (depthRay > depthProbe) {
            return UNKNOWN; // UNKNOWN (rayon derrière surface)
        }
    }

    return MISS; // MISS (aucune intersection trouvée)
}

float drawPointMarkerAlpha(vec2 fragUV, vec3 point, float radiusPx)
{
    // projeter le point 3D en UV
    vec2 pUV = octahedral_mapping(normalize(point));
    pUV      = mirrorAndWrapUV(pUV);

    // convertir le rayon pixel en coordonnées UV
    float radiusUV = radiusPx;
    float d        = length(fragUV - pUV);
    float a        = 1.0 - smoothstep(0.0, radiusUV, d);
    return pow(clamp(a, 0.0, 1.0), 0.8);
}

// ===================== RANDOM UTILITIES =========================

// Générateurs de hash / nombres aléatoires simples
float hash12(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

vec3 hash32(vec2 p)
{
    return vec3(hash12(p + vec2(1.0, 0.0)), hash12(p + vec2(2.0, 0.0)), hash12(p + vec2(3.0, 0.0)));
}

// Génère un point aléatoire dans un cube [-r, r]³ à partir d'une graine 2D
vec3 randomPointInBox(vec2 seed, float r)
{
    vec3 h = hash32(seed);
    return (h * 2.0 - 1.0) * r;
}

// ===================== MAIN =========================
bool showTriangleIndexOverlay() { return false; } // Mettre à false pour désactiver
/////////////////////////////////////////////////////////////////////

void main()
{
    vec3 dir      = octahedral_unmapping(texCoords);
    vec3 envColor = texture(radianceCubemap, dir).rgb;
    color         = vec4(envColor, 1.0);
    gl_FragDepth  = linearDepth(texture(depthCubemap, dir).r) / zFar;

    if (length(texCoords - debugStart.xy) < 0.01)
        color = vec4(1, 0, 0, 1);

    //////////////////////////////////////////////////////////////////////:

    // coordonnées UV normalisées [0,1]
    vec2 uv = texCoords;

    // appliquer le mirroring pour gérer les bords de la carte octahedral car les coins sont voisins
    uv = mirrorAndWrapUV(uv);

    // echantillonner la cubemap en utilisant le décodage octaédral
    vec3 baseColor = sampleCubemapFromOctaUV(uv, radianceCubemap);

    vec3 outColor = baseColor;

    // Optionnel : overlay couleur selon l'indice du triangle octaédral ainsi que les bord des triangles
    if (showTriangleIndexOverlay()) {
        // calculer la distance aux arêtes des triangles et tracer les contours
        // pour voir les différen triangles octaédraux
        float minDist   = computeMinEdgeDistance(uv);
        float lineAlpha = lineAlphaFromDist(minDist);
        vec3 lineColor  = vec3(0.70, 0.00, 0.70);
        outColor        = mix(outColor, lineColor, clamp(lineAlpha, 0.0, 1.0));

        vec3 dir      = octahedral_unmapping(uv);
        int triIdx    = getOctahedralTriangle(dir);
        vec3 triColor = vec3(0.0);
        if (triIdx == 0)
            triColor = vec3(1, 0, 0); // Rouge
        else if (triIdx == 1)
            triColor = vec3(0, 1, 0); // Vert
        else if (triIdx == 2)
            triColor = vec3(0, 0, 1); // Bleu
        else if (triIdx == 3)
            triColor = vec3(1, 1, 0); // Jaune
        else if (triIdx == 4)
            triColor = vec3(1, 1, 1); // Blanc
        else if (triIdx == 5)
            triColor = vec3(0, 1, 1); // Cyan
        else if (triIdx == 6)
            triColor = vec3(1, 0.5, 0); // Orange
        else if (triIdx == 7)
            triColor = vec3(0, 0, 0); // Noir
        // Mélange overlay (25%)
        outColor = mix(outColor, triColor, 0.25);
    }

    // tracer le rayon projeté sur la carte octaédrale
    float minDistRay = computeMinDistLinePolyline(uv, debugStart - debugProbePos, debugEnd - debugProbePos);
    float rayAlpha   = lineAlphaFromDist(minDistRay);
    vec3 rayCol      = vec3(1.0, 0.2, 0.1);
    // superposer le rayon par-dessus la cubemap et les bords
    outColor = mix(outColor, rayCol, clamp(rayAlpha, 0.0, 1.0));

    // dessiner les points de début et de fin avec des couleurs distinctes
    // float startAlpha = drawPointMarkerAlpha(uv, debugStart, 6.0);
    // vec3 startCol = vec3(0.2, 1.0, 0.2);
    // outColor = mix(outColor, startCol, startAlpha);

    // float endAlpha = drawPointMarkerAlpha(uv, debugEnd, 5.0);
    // vec3 endCol = vec3(0.2, 0.6, 1.0);
    // outColor = mix(outColor, endCol, endAlpha);
    color.rgb = outColor;
}