// Générateurs de hash / nombres aléatoires simples
float hash12(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

vec3 hash32(vec2 p)
{
    return vec3(hash12(p + vec2(1.0, 0.0)), hash12(p + vec2(2.0, 0.0)), hash12(p + vec2(3.0, 0.0)));
}
vec3 randomPointInBox(vec2 seed, float r)
{
    vec3 h = hash32(seed);
    return (h * 2.0 - 1.0) * r;
}