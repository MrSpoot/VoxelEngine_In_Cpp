#version 430 core

#define MAX_STEPS 128
#define MIN_STEP_SIZE 1e-4
#define MAX_STEP_SIZE 32.0
#define VOXEL_SIZE 0.25

out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 camPos;
uniform vec3 camDir;
uniform vec3 camRight;
uniform vec3 camUp;
uniform float fov;
uniform float aspectRatio;
uniform float time;
uniform float size;

uniform sampler3D sdfTexture;

float sdTorus( vec3 p, vec2 t )
{
    vec2 q = vec2(length(p.xz)-t.x,p.y);
    return length(q)-t.y;
}

float sdSphere( vec3 p, float s )
{
    return length(p)-s;
}

vec3 getVoxelPos(vec3 p, float s)
{
    return (floor(p / s) + 0.5) * s;
}

float getSDF(vec3 p) {
    vec3 texCoord = (p + vec3(size / 2.0)) / size;
    return texture(sdfTexture, texCoord).r;
    //return min(sdSphere(p,2.0),sdTorus(p,vec2(8.0,2.0)));
}

float trace(vec3 ro, vec3 rd){

    const float s = VOXEL_SIZE;
    const float sd = s * sqrt(3.0);

    vec3 ird = 1.0 / rd;
    vec3 iro = ro * ird;
    vec3 srd = sign(ird);
    vec3 ard = abs(ird);

    float t = 0.0;

    vec3 vpos = getVoxelPos(ro, s);

    bool voxel = false;
    int vi = 0;
    vec3 prd = vec3(0);

    for (int i = 0; i < MAX_STEPS; i++){

        vec3 pos = ro + rd * t;

        float d = getSDF(voxel ? vpos : pos);

        if(!voxel){
            t += d;

            if (d < sd)
            {
                vpos = getVoxelPos(ro + rd * max(t - sd, 0.0), s);
                voxel = true;
                vi = 0;
            }
        }else{
            vec3 n = (ro - vpos) * ird;
            vec3 k = ard * s * 0.5;

            vec3 t1 = -n - k;
            vec3 t2 = -n + k;

            float tF = min(min(t2.x, t2.y), t2.z);

            vec3 nrd = t2.x <= t2.y && t2.x <= t2.z ? vec3(srd.x,0,0) :
            t2.y <= t2.z ? vec3(0,srd.y,0) : vec3(0,0,srd.z);

            if(d < 0.0){
                return t;
            }else if(d > sd && vi > 2){
                voxel = false;
                t = tF + sd;
                continue;
            }

            vpos += nrd * s;
            prd = nrd;
            t = tF;
            vi++;

            if (t >= MAX_STEP_SIZE) return -1.0;
        }
    }
    return -1.0;
}

vec3 normal(vec3 p) {
    float eps = 0.01;
    vec3 n;
    n.x = getSDF(p + vec3(eps, 0.0, 0.0)) - getSDF(p - vec3(eps, 0.0, 0.0));
    n.y = getSDF(p + vec3(0.0, eps, 0.0)) - getSDF(p - vec3(0.0, eps, 0.0));
    n.z = getSDF(p + vec3(0.0, 0.0, eps)) - getSDF(p - vec3(0.0, 0.0, eps));
    return normalize(n);
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rd = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 ro = camPos;

    vec3 color = vec3(0.0);
    float d = trace(ro, rd);

    if (d < MAX_STEP_SIZE && d > 0.0){
        vec3 p = ro + rd * d;
        vec3 nor = normal(p);
        color = nor * 0.5 + 0.5;
    }

    FragColor = vec4(color, 1.0);
}
