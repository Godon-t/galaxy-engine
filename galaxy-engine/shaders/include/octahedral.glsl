// https://www.shadertoy.com/view/MdsfzN

vec2 octahedral_mapping(vec3 co)
{
    // Projection onto octahedron
    co /= dot(vec3(1), abs(co));

    // Out-folding of the downward faces
    if (co.z < 0.0) {
        co.xy = (1.0 - abs(co.yx)) * sign(co.xy);
    }

    // Mapping to [0;1]^2 texture space
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
int computeOctahedralIntersections(vec3 p0, vec3 p1, out float ts[8]) // Taille doublée !
{
    float temp[8];
    temp[0]   = 0.0;
    temp[1]   = 1.0;
    int count = 2;

    // Les 3 plans principaux x=0, y=0, z=0
    for (int i = 0; i < 3; ++i) {
        float a = p0[i];
        float b = p1[i];

        float d = b - a;
        if (abs(d) > 1e-6) {
            float t = -a / d;
            if (t > 1e-5 && t < 1.0 - 1e-5) {
                temp[count] = t;
                count       = count + 1;
            }
        }
    }

    // Trier temp[0..count-1] (insertion sort par exemple)
    for (int i = 1; i < count; i++) {
        float key = temp[i];
        int j     = i - 1;
        while (j >= 0 && temp[j] > key) {
            temp[j + 1] = temp[j];
            j--;
        }
        temp[j + 1] = key;
    }

    // Convertir en segments : dupliquer les points intermédiaires
    int outCount = 0;
    for (int i = 0; i < count - 1; i++) {
        ts[outCount++] = temp[i]; // Début du segment
        ts[outCount++] = temp[i + 1] - 0.01; // Fin du segment
    }

    return outCount; // Retourne le nombre de valeurs (= 2 * nombre de segments)
}