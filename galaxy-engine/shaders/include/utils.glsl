float linearDepth(float depth, float zNear, float zFar)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

float depthToRadialDistance(float depthValue, vec3 direction, float zNear, float zFar)
{
    float linearZ      = linearDepth(depthValue, zNear, zFar);
    float maxComponent = max(max(abs(direction.x), abs(direction.y)), abs(direction.z));
    return linearZ / maxComponent;
}