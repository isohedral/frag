//
// Hexagon outlines
//

const vec2 T0 = vec2( 0.0, 0.0 );
const vec2 T1 = vec2( 0.25 * sqrt(3.0), 0.25 );
const vec2 T2 = vec2( 0.25 * sqrt(3.0), -0.25 );
const vec2 T3 = T1 + T2;
const mat2 B = mat2( T1, T2 );
const mat2 CB = inverse( B );

void main() 
{ 
	vec2 p = CB * wpos;
	vec2 fp = floor(p);
	vec2 r = p - fp;

	if( r.x + r.y > 1.0 ) {
		r = vec2(1.0,1.0) - r;
	}
		
	vec2 sr = B * r;

	float d1 = distance( sr, vec2( 0.0, 0.0 ) );
	float d2 = distance( sr, T1 );
	float d3 = distance( sr, T2 );
	float dmin = min( min(d1,d2), d3 );

	float ds = max( max( min(d1,d2), min(d2,d3) ), min(d1,d3) );

	if( abs(dmin - ds) < 0.05 ) {
		col = vec4( 0.0, 0.0, 0.0, 1.0 );
	} else {
		col = vec4( 1.0, 1.0, 1.0, 1.0 );
	}
}
