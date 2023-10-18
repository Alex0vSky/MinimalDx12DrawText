cbuffer PerFrameConstants : register (b0) {float iTime;}

//static const float2 iResolution = float2( 1280, 720 );
//static const float2 iResolution = float2( 800, 600 );
static const float2 iResolution = float2( 640, 480 );
//static const float2 iResolution = float2( 320, 200 );



// camera @insp https://www.shadertoy.com/view/4lVXRm
static const float PI = 3.141592;
static const int2 POSITION = int2(1, 0);
static const int2 VMOUSE = int2(1, 1);
//#define load(P) texelFetch(iChannel1, int2(P), 0)
float3x3 CameraRotation( float2 m )
{
    m.y = -m.y;
    
    float2 s = sin(m);
    float2 c = cos(m);
    float3x3 rotX = float3x3(1.0, 0.0, 0.0, 0.0, c.y, s.y, 0.0, -s.y, c.y);
    float3x3 rotY = float3x3(c.x, 0.0, -s.x, 0.0, 1.0, 0.0, s.x, 0.0, c.x);
    
    return rotY * rotX;
}
float3 Grid(float3 ro, float3 rd) {
	float d = -ro.y/rd.y;
    
    if (d <= 0.0) return float3(0.4, 0, 0);
    
   	float2 p = (ro.xz + rd.xz*d);
    
    float2 e = min(float2(1.0, 0), fwidth(p));
    
    float2 l = smoothstep(float2(1.0, 0), 1.0 - e, frac(p)) + smoothstep(float2(0.0, 0), e, frac(p)) - (1.0 - e);

    return lerp(float3(0.4, 0, 0), float3(0.8, 0, 0) * (l.x + l.y) * 0.5, exp(-d*0.01));
}
void Camera(in float2 fragCoord, out float3 ro, out float3 rd) 
{
    //ro = load(POSITION).xyz;
    ro = float3( 0, 0, 0);
	
    //float2 m = load(VMOUSE).xy/iResolution.x;
    float2 m = float2( 1, 1 ) *iTime;

    float a = 1.0/max(iResolution.x, iResolution.y);
    rd = normalize(float3((fragCoord - iResolution.xy*0.5)*a, 0.5));
    
//	rd = CameraRotation(m) * rd;
    rd = mul( rd, CameraRotation(m) );
}



// Rotation matrix around the Y axis.
float3x3 rotateY(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return float3x3(
        float3(c, 0, s),
        float3(0, 1, 0),
        float3(-s, 0, c)
    );
}


static const float3x3 rotationMatrix = float3x3( 1.0,0.0,0.0, 0.0,0.47,-0.88, 0.0,0.88,0.47 );
	
float hash(float p)
{
    uint x = uint(p  + 16777041.);
    x = 1103515245U*((x >> 1U)^(x));
    uint h32 = 1103515245U*((x)^(x>>3U));
    uint n =  h32^(h32 >> 16);
    return float(n)*(1.0/float(0xffffffffU));
}

float noise( float3 x )
{
    float3 p = floor(x);
    float3 f = frac(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;
    return lerp(lerp(lerp( hash(n+0.0  ), hash(n+1.0),f.x),lerp( hash(n+57.0 ), hash(n+58.0 ),f.x),f.y),
           lerp(lerp( hash(n+113.0), hash(n+114.0),f.x),lerp( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
} 

float4 map( float3 p )
{
	float d = 0.2 - p.y;	
	float3 q = p  - float3(0.0,1.0,0.0)*iTime;
	float f  = 0.50000*noise( q ); q = q*2.02 - float3(0.0,1.0,0.0)*iTime;
	f += 0.25000*noise( q ); q = q*2.03 - float3(0.0,1.0,0.0)*iTime;
	f += 0.12500*noise( q ); q = q*2.01 - float3(0.0,1.0,0.0)*iTime;
	f += 0.06250*noise( q ); q = q*2.02 - float3(0.0,1.0,0.0)*iTime;
	f += 0.03125*noise( q );
	d = clamp( d + 4.5*f, 0.0, 1.0 );
	float3 col = lerp( float3(1.0,0.9,0.8), float3(0.4,0.1,0.1), d ) + 0.05*sin(p);
	return float4( col, d );
}

float3 raymarch( float3 ro, float3 rd )
{
	float4 s = float4( 0,0,0,0 );
	float t = 0.0;	
//	for( int i=0; i<128; i++ )
	for( int i=0; i<64; i++ )
	{
		if( s.a > 0.99 ) break;
		float3 p = ro + t*rd;
		float4 k = map( p );
		k.rgb *= lerp( float3(3.0,1.5,0.15), float3(0.5,0.5,0.5), clamp( (p.y-0.2)/2.0, 0.0, 1.0 ) );
		k.a *= 0.5;
		k.rgb *= k.a;
		s = s + k*(1.0-s.a);	
		t += 0.05;
	}
	return clamp( s.xyz, 0.0, 1.0 );
}

float4 main(float4 position : SV_POSITION) : SV_TARGET
{
	// shadertoy spec
	float4 fragColor = float4( 0, 0, 0, 0 );
	float2 fragCoord = position.xy;


//*
	{
	//float3 ro = float3( (0.0),(4.9),(-40.));
	//float3 ro = float3( (0.0+cos(iTime) ),(4.9+2*sin(iTime)),(-40.));
//	float3 ro = float3( (0.0 ),(3.9+1.5*cos(iTime)),(-40.));
	float3 ro = float3( (0.0),(3.9),(-40.));

	// float3 rd = normalize( float3( ( 2.0*fragCoord.xy - iResolution.xy ) / iResolution.y, 2.0 ) ) * rotationMatrix;
	
	float2 xResolution = iResolution;
//	xResolution.y = 1 - xResolution.y; // DirectX/HLSL rotate
//	fragCoord.y = 1 - fragCoord.y; // DirectX/HLSL rotate

	float3 rd_ = normalize( float3( ( 2.0*fragCoord.xy - xResolution.xy ) / xResolution.y, 2.0 ) );
//	static const float some = 1.0;
//	float3 rd_ = normalize( float3( ( some*fragCoord.xy - xResolution.xy ) / xResolution.y, some ) );
//	float3 rd_ = normalize( float3( ( fragCoord.xy - xResolution.xy ) / xResolution.y, 1 ) );
	float3 rd = mul( rotationMatrix, rd_ );

//		float2 uv = fragCoord.xy / xResolution.xy;
//		uv.y = 1 - uv.y; // DirectX/HLSL rotate
////	rd = normalize( float3( uv, -1 ) );
//		rd = mul( rotateY( sin( iTime ) * 0.5 ), rd );

	float3 volume = raymarch( ro, rd );
	volume = volume*0.5 + 0.5*volume*volume*(3.0-2.0*volume);
	fragColor += float4( volume, 1.0 );
	}
//*/

/*
	{
	float3 ro = float3(0.0, 0, 0);
    float3 rd = float3(0.0, 0, 0);
    Camera( fragCoord, ro, rd );
    float3 color = Grid(ro, rd);
    fragColor += float4(pow(color, float3(0.4545,0,0)), 1.0);
	}
//*/
	
	// shadertoy spec
	return fragColor;
}