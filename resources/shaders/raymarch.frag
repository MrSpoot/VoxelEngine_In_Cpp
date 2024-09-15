#version 430 core

#define MAX_STEPS 128
#define MIN_STEP_SIZE 1e-4
#define MAX_STEP_SIZE 32.0
#define VOXEL_SIZE 0.1

struct HitInfo {
    vec3 id;
    float t;
    vec3 n;
    int i;
};

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
uniform bool showSteps;
uniform bool showNormals;

uniform sampler3D sdfTexture;
uniform sampler2D colorTexture;

vec3 turbo(float t) {

    const vec3 c0 = vec3(0.1140890109226559, 0.06288340699912215, 0.2248337216805064);
    const vec3 c1 = vec3(6.716419496985708, 3.182286745507602, 7.571581586103393);
    const vec3 c2 = vec3(-66.09402360453038, -4.9279827041226, -10.09439367561635);
    const vec3 c3 = vec3(228.7660791526501, 25.04986699771073, -91.54105330182436);
    const vec3 c4 = vec3(-334.8351565777451, -69.31749712757485, 288.5858850615712);
    const vec3 c5 = vec3(218.7637218434795, 67.52150567819112, -305.2045772184957);
    const vec3 c6 = vec3(-52.88903478218835, -21.54527364654712, 110.5174647748972);

    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));

}

vec3 triplanar(vec3 pos, vec3 normal) {
    vec3 absNormal = abs(normal);
    vec3 blend = absNormal / (absNormal.x + absNormal.y + absNormal.z);

    vec2 uvX = pos.yz;// Projeter sur l'axe X
    vec2 uvY = pos.xz;// Projeter sur l'axe Y
    vec2 uvZ = pos.xy;// Projeter sur l'axe Z

    vec3 colorX = texture(colorTexture, uvX).rgb;
    vec3 colorY = texture(colorTexture, uvY).rgb;
    vec3 colorZ = texture(colorTexture, uvZ).rgb;

    vec3 finalColor = colorX * blend.x + colorY * blend.y + colorZ * blend.z;

    return finalColor;
}

vec3 getVoxelPos(vec3 p, float s){
    return (floor(p / s) + 0.5) * s;
}

vec3 getVoxelColor(vec3 voxelPos) {
    vec2 uv = fract((voxelPos.xz / size) * 100);
    vec3 color = texture(colorTexture, uv).rgb;
    return color;
}

float getSDF(vec3 p) {
    vec3 texCoord = (p + vec3(size / 2.0)) / size;
    return texture(sdfTexture, texCoord).r;
}

bool trace(vec3 ro, vec3 rd, out HitInfo hit){

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

        if (!voxel){
            t += d;

            if (d < sd)
            {
                vpos = getVoxelPos(ro + rd * max(t - sd, 0.0), s);
                voxel = true;
                vi = 0;
            }

            if (d > MAX_STEP_SIZE){
                return false;
            }
        } else {
            vec3 n = (ro - vpos) * ird;
            vec3 k = ard * s * 0.5;

            vec3 t1 = -n - k;
            vec3 t2 = -n + k;

            float tF = min(min(t2.x, t2.y), t2.z);

            vec3 nrd = t2.x <= t2.y && t2.x <= t2.z ? vec3(srd.x, 0, 0) :
            t2.y <= t2.z ? vec3(0, srd.y, 0) : vec3(0, 0, srd.z);

            if (d < 0.0){
                hit.n = -prd;
                hit.t = t;
                hit.i = i;
                hit.id = vpos;
                return true;
            } else if (d > sd && vi > 2){
                voxel = false;
                t = tF + sd;
                continue;
            }

            vpos += nrd * s;
            prd = nrd;
            t = tF;
            vi++;

            if (t >= MAX_STEP_SIZE) return false;;
        }
    }
    return false;
}

void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rd = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 ro = camPos;

    vec3 color = vec3(0.65, 0.95, 0.98);

    HitInfo hit;
    bool isHit = trace(ro, rd, hit);

    if (isHit){
        vec3 nor = hit.n;
        if (showNormals){
            color = nor * 0.5 + 0.5;
        } else if (showSteps){
            color = turbo(float(hit.i) / float(MAX_STEPS));
        } else {
            color = triplanar(hit.id, nor);
        }
    }

    FragColor = vec4(color, 1.0);
}
