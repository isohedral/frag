//
// Hexagon outlines.  Try to use more clever functions to fold out
// symmetries.
//

const vec2 T0 = vec2( 0.0, 0.0 );
const vec2 T1 = vec2( 0.25 * sqrt(3.0), 0.25 );
const vec2 T2 = vec2( 0.25 * sqrt(3.0), -0.25 );
const vec2 T01 = mix( T0, T1, 0.5 );
const vec2 T02 = mix( T0, T2, 0.5 );
const vec2 T12 = mix( T1, T2, 0.5 );

const vec2 T3 = T1 + T2;
const mat2 B = mat2( T1, T2 );
const mat2 CB = inverse( B );

// Force P to lie on the right side of the line from A to B, reflecting
// across the line if necessary.
vec2 forceRight( vec2 P, vec2 A, vec2 B )
{
	mat3 M = mat3(
		B.x-A.x, B.y-A.y, 0.0,
		A.y-B.y, B.x-A.x, 0.0,
		A.x, A.y, 1.0 );
	mat3 Mi = inverse( M );

	vec3 p = Mi * vec3(P,1.0);
	return vec2( M * vec3(p.x, -abs(p.y), 1.0) );
}

float d2l( vec2 P, vec2 A, vec2 B )
{
    vec2 n = normalize(vec2( B.y - A.y, A.x - B.x ));
	return abs(dot( P - A, n ));
}

void main() 
{ 
	vec2 p = CB * wpos;
	vec2 fp = floor(p);
	vec2 r = p - fp;
	vec2 sr = B * r;
	sr = forceRight( vec2( min(sr.x, 2.0*T1.x-sr.x), -abs(sr.y) ), T01, T2 );

	float d = smoothstep( 0.03, 0.04, d2l( sr, T1, T02 ) );
	col = vec4( d, d, d, 1.0 );
}
