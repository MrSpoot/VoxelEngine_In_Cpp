#version 430 core

#define MAX_STEPS 128
#define MIN_STEP_SIZE 0.001
#define MAX_STEP_SIZE 64

out vec4 FragColor;
in vec2 TexCoords;

// Paramètres de la caméra
uniform vec3 camPos;      // Position de la caméra
uniform vec3 camDir;      // Direction de la caméra
uniform vec3 camRight;    // Vecteur à droite de la caméra
uniform vec3 camUp;       // Vecteur vers le haut de la caméra
uniform float fov;        // Champ de vision (en radians)
uniform float aspectRatio; // Rapport d'aspect de l'écran (width / height)
uniform float time;

vec2 dBox(vec3 p, vec3 s, float i){

    vec3 diff = abs(p) - s;
    float de = length(max(diff,0.0));

    float di = min(max(diff.x,max(diff.y,diff.z)),0.0);

    float d = de + di;

    return vec2(i,d);
}

vec2 dPlane(vec3 p, float h, float i){
    return vec2(i,p.y - h);
}

vec2 dSphere(vec3 p, float r,float i){
    return vec2(i,length(p) - r);
}

vec2 sdTorus( vec3 p, vec2 t, float i){
    vec2 q = vec2(length(p.xz)-t.x,p.y);
    float d = length(q)-t.y;

    return vec2(i,d);
}

vec2 minVec2(vec2 a, vec2 b){
    return a.y < b.y ? a : b;
}

vec2 scene(vec3 p){

    vec2 dp = dPlane(p,-0.5,0.0);
    vec2 ds = dSphere(p - vec3(0.0,-0.5,1.5),0.25,1.0);
    vec2 db = dBox(p - vec3(0.0,0.0,1.5),vec3(0.2),2.0);
    vec2 dt = sdTorus(p - vec3(2.0,0.0,1.5),vec2(0.5,0.2),1.0);

    return minVec2(dp,minVec2(ds,minVec2(db,dt)));
}

vec3 material(float i){

    vec3 col = vec3(0.0);

    if(i < 0.5){
        col = vec3(0.7);
    }else if(i < 1.5){
        col =  vec3(1,0,0);
    }else if(i < 2.5){
        col =  vec3(0,1,0);
    }else if(i < 3.5){
        col =  vec3(0,0,1);
    }

    return col * vec3(0.2);
}

vec2 march(vec3 ro, vec3 rd){
    vec3 cp = ro;
    float d = 0.0;
    vec2 s = vec2(0.0);

    for(int i = 0; i < MAX_STEPS; i++){
        cp = ro + rd * d;
        s = scene(cp);
        d += s.y;

        if(s.y < MIN_STEP_SIZE){
            break;
        }

        if(d > MAX_STEP_SIZE){
            return vec2(100.0,MAX_STEP_SIZE + 10.0);
        }
    }
    s.y = d;
    return s;
}

vec3 normal(vec3 p){
    float dp = scene(p).y;
    vec2 eps = vec2(0.01,0);

    float dx = scene(p + eps.xyy).y - dp;
    float dy = scene(p + eps.yxy).y - dp;
    float dz = scene(p + eps.yyx).y - dp;

    return normalize(vec3(dx,dy,dz));
}


float lighting(vec3 p, vec3 n){
    vec3 lp = vec3(cos(time) * 2.0,1.0,sin(time));
    //vec3 lp = vec3(3,2,-0.5);
    vec3 ld = lp - p;
    vec3 ln = normalize(ld);

    if(march(p + n * 0.01,ln).y < length(ld)){
        return 0.0;
    }


    return max(0.0,dot(n,ln));
}


void main() {
    vec2 uv = TexCoords * 2.0 - 1.0;
    uv.x *= aspectRatio;

    vec3 rd = normalize(camDir + uv.x * tan(fov / 2.0) * camRight + uv.y * tan(fov / 2.0) * camUp);
    vec3 ro = camPos;

    vec2 s = march(ro,rd);

    vec3 skyCol = vec3(1.0);

    vec3 color = mix(vec3(1.0),vec3(0.0,0.2,1.0),uv.y + 0.6);
    float d = s.y;

    if(d < MAX_STEP_SIZE){
        color = material(s.x);
        vec3 p = ro + rd * d;
        vec3 nor = normal(p);
        float l = lighting(p,nor);
        vec3 a = vec3(10.0,7.0,2.0) * 0.01;
        vec3 as = (nor.y * skyCol) * 0.5;
        color = color * (a + l + as);
        //color = nor;
    }

    color = pow(color,vec3(0.4545));

    FragColor = vec4(color,1.0);
}
