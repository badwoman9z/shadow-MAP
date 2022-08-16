#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform sampler2D texture1;
uniform sampler2D depthMap;



#define NUM_SAMPLES 50
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

highp float rand_1to1(highp float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}

highp float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}
float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    float depth =dot(rgbaDepth, bitShift) ;
    //shadow map 没有深度值的地方默认是0 导致的有噪点
    if(abs(depth)<EPS){
      depth=1.0;
    }

    return  depth;
}
vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples( const in vec2 randomSeed ) {

  float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
  float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

  float angle = rand_2to1( randomSeed ) * PI2;
  float radius = INV_NUM_SAMPLES;
  float radiusStep = radius;

  for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
  }
}
float findBlock(vec2 uv,float depth){
    float texturesize = 400;
    float stride = 10.0;
    float filterRange = stride/texturesize;
    poissonDiskSamples(uv);
    float avgDepth = 0.0;
    int count = 0;
    for(int i=0;i<BLOCKER_SEARCH_NUM_SAMPLES;i++){
    
        float blockDepth = unpack(texture(depthMap,uv+filterRange*poissonDisk[i]));
        if(depth > blockDepth-0.01){
            avgDepth += blockDepth;
            count++;
        }    
    }
    return avgDepth/float(count);

}
float PCSS(vec3 coords){
    poissonDiskSamples(coords.xy);
    float filterSize = 1.0;
    float texturesize = 400;
    float depth = coords.z;
    float block = findBlock(coords.xy,depth);
    float penumbra = (depth - block)/block * 10.0;
    float filterRange = (penumbra*filterSize)/texturesize;
    float count = 0.0;
    for(int i = 0;i<NUM_SAMPLES;i++){
         float closeDepth = unpack(texture(depthMap,coords.xy+poissonDisk[i]*filterRange));
         count+= depth -0.01> closeDepth ? 0.0 : 1.0;
    
    }
    return count/float(NUM_SAMPLES);



}
float PCF(vec3 coords){
    float texturesize = textureSize(depthMap,0).x;
    float stride = 5.0;
    float filterRange = stride/texturesize;
    float depth = coords.z;
    float result = 0.0;
    poissonDiskSamples(coords.xy);
    for(int i=0;i<NUM_SAMPLES;i++){
       vec2 sampleUV = coords.xy + filterRange*poissonDisk[i];
       float closeDepth = unpack(texture(depthMap,sampleUV));
       result += depth -0.01> closeDepth ? 0.0 : 1.0;
    }
    return result/float(NUM_SAMPLES);

}
float shadowMap(vec3 shadowPos ){
    float realDeth = shadowPos.z;
    float shadowDeth = unpack(texture2D(depthMap ,shadowPos.xy));
    if(realDeth>shadowDeth){
        return 0.0;
    }
    return 1.0;
}
vec3 blinPhong(){
   vec3 wo = normalize(viewPos-vs_out.FragPos);
   vec3 wi = normalize(lightPos-vs_out.FragPos);
   vec3 color = texture(texture1,vs_out.TexCoords).rgb;
   vec3 lightColor = vec3(1.0);

   float kd = max(dot(wi,vs_out.Normal),0);
   vec3 diffuse = kd*lightColor;

   vec3 h = normalize(wi+wo);
   float kp = pow(max(dot(h,vs_out.Normal),0),64);
   vec3 speculat = kp*lightColor;

   vec3  ambient = 0.3 * lightColor;
   return (diffuse+speculat+ambient)*color;
   
}

void main(){
  vec3 shadowPos = vs_out.FragPosLightSpace.xyz/vs_out.FragPosLightSpace.w;
  shadowPos  = shadowPos * 0.5 +  0.5;
  //float v = PCF(shadowPos); 
  //float v =  shadowMap(shadowPos); 
  float v = PCSS( shadowPos );
  vec3 color = blinPhong();

  FragColor = vec4(color*v,1.0 );
}
