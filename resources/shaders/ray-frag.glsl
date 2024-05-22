#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 camPos;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

#define MAX_STEPS 100
#define MAX_DIST 100.0
#define SURFACE_DIST 0.01

float map(vec3 p) {
    // Distance to the nearest object (cube in this example)
    vec3 boxSize = vec3(1.0);
    vec3 q = abs(p) - boxSize;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

vec3 getNormal(vec3 p) {
    float d = map(p);
    vec2 e = vec2(0.01, 0.0);

    vec3 n = d - vec3(
    map(p - e.xyy),
    map(p - e.yxy),
    map(p - e.yyx)
    );

    return normalize(n);
}

float rayMarch(vec3 ro, vec3 rd) {
    float dO = 0.0;

    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * dO;
        float dS = map(p);
        dO += dS;

        if (dO > MAX_DIST || dS < SURFACE_DIST)
        break;
    }

    return dO;
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= 1280.0 / 720.0;

    vec3 ro = camPos;
    vec3 rd = normalize(vec3(uv, -1.0));
    rd = (view * vec4(rd, 0.0)).xyz;

    float dist = rayMarch(ro, rd);

    vec3 p = ro + rd * dist;
    vec3 normal = getNormal(p);
    vec3 lightDir = normalize(vec3(0.0, 15.0, 0.0) - p);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 color = vec3(0.36, 0.76, 0.4) * diff;
    FragColor = vec4(color, 1.0);
}
