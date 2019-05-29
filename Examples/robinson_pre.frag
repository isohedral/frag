// Robinson R1 tiling, just one level.

float getSquares( vec2 p, float th, float fw )
{
	vec2 m = mod( p, 2.0 );
	m = min( m, 2.0 - m );
	float s = min(m.x,m.y);
	float d = abs( 0.5 - s );
	return smoothstep( th - fw, th + fw, d );
}

void main() 
{ 
	float th = 0.125;
	float fw = 0.001;

	float c = getSquares( wpos, th, fw ) * getSquares( wpos/2.0, th/2.0, fw/2.0 );

	col = vec4( c, c, c, 1.0 );
}
