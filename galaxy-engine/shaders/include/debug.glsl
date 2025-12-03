#include octahedral.glsl

// Convertit une distance UV en alpha pour tracer une ligne
// Prend en compte la résolution d'écran pour une épaisseur de ligne constante
float lineAlphaFromDist(float minDist, samplerCube cubemap)
{
    vec2 iResolution = textureSize(cubemap, 0).xy;
    // Échelle de pixel pour adapter l'épaisseur de ligne
    float pixelScale = length(fwidth(gl_FragCoord.xy / iResolution.xy)) * length(iResolution.xy);
    float lineWidth  = 0.8 * pixelScale / iResolution.x;
    lineWidth        = max(lineWidth, 0.005);
    float lineAlpha  = 1.0 - smoothstep(0.0, lineWidth, minDist);
    return pow(lineAlpha, 0.9);
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

float drawPointMarkerAlpha(vec2 fragUV, vec3 point, float radiusPx)
{
    // projeter le point 3D en UV
    vec2 pUV = octahedral_mapping(normalize(point));

    // convertir le rayon pixel en coordonnées UV
    float radiusUV = radiusPx;
    float d        = length(fragUV - pUV);
    float a        = 1.0 - smoothstep(0.0, radiusUV, d);
    return pow(clamp(a, 0.0, 1.0), 0.8);
}

void showTrianglesOverlay(vec2 uv, inout vec3 outColor, samplerCube cubemap)
{
    // calculer la distance aux arêtes des triangles et tracer les contours
    // pour voir les différen triangles octaédraux
    float minDist   = computeMinEdgeDistance(uv);
    float lineAlpha = lineAlphaFromDist(minDist, cubemap);
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

// Calcule la distance minimale d'un point UV à un segment 3D (p0->p1)
// projeté sur la carte octahedral. La trajectoire est subdivisée finement
// pour capturer la courbure de la projection tout en détectant les discontinuités
// aux bords de l'UV map (quand le rayon passe d'un côté à l'autre du au voisin).
float computeMinDistLinePolyline(vec3 p0, vec3 p1, vec3 probePos, float t, vec2 scale, vec2 probeTextureUpperLeft, vec2 fragUV)
{
    float minDist    = 1e6;
    vec3 fallbackDir = normalize(p1 - p0 + vec3(1e-6));

    // subdiviser le segment 3D pour avoir une courbe continue
    int totalSubdivisions = 32;

    // centrer les positions par rapport à la probe
    vec3 centeredP0 = p0 - probePos;
    vec3 centeredP1 = p1 - probePos;

    // calculer les t ou le rayon change de triangle octahédral
    float ts[8];
    int numSegments = computeOctahedralIntersections(centeredP0, centeredP1, ts);
    numSegments     = numSegments - 1;

    // parcourir chaque segment entre les changements de triangle
    for (int segIdx = 0; segIdx < numSegments; segIdx += 2) {
        float t0 = ts[segIdx];
        float t1 = ts[segIdx + 1];

        // calculer les positions 3D aux extrémités du segment
        vec3 p3d0 = mix(centeredP0, centeredP1, t0);
        vec3 p3d1 = mix(centeredP0, centeredP1, t1);

        // calculer les distances aux extrémités
        float depth0 = length(p3d0);
        float depth1 = length(p3d1);

        // calculer les directions et projeter en 2D octahedral
        vec3 fallbackDir = normalize(centeredP1 - centeredP0 + vec3(1e-6));
        vec3 dir0        = safeNormalize(p3d0, fallbackDir);
        vec3 dir1        = safeNormalize(p3d1, fallbackDir);

        vec2 uv0 = octahedral_mapping(dir0);
        vec2 uv1 = octahedral_mapping(dir1);

        // mettre a l'échelle et décaler les UV dans la texture globale
        uv0 = (uv0 * scale) + probeTextureUpperLeft;
        uv1 = (uv1 * scale) + probeTextureUpperLeft;

        for (int i = 0; i < totalSubdivisions; ++i) {
            float s = float(i) / float(totalSubdivisions);

            vec2 samplingPos = mix(uv0, uv1, s);

            if (length(fragUV - samplingPos) < 0.01) {
                return 0.0;
            }
        }
    }
    return 1.0;
}