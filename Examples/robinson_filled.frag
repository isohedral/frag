// Robinson R1 tiling

float getSquares( vec2 p, float th, float fw )
{
	vec2 m = mod( p, 2.0 );
	m = min( m, 2.0 - m );
	float s = min(m.x,m.y);

	float d = abs( 0.5 - s );

	if( s > 0.5 ) {
		return 0.8 * smoothstep( th - fw, th + fw, d );
	} else {
		return smoothstep( th - fw, th + fw, d );
	}
}

void main() 
{ 
	float c = 1.0;
	float th = 0.125;
	float fw = 0.001;
	vec2 p = wpos;

	for( int idx = 0; idx < 20; ++idx ) {
		c *= getSquares( p, th, fw );

		p *= 0.5;
		th *= 0.5;
		fw *= 0.5;
	}

	col = vec4( c, c, c, 1.0 );
}
