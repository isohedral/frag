//
// Right triangles in a square grid.
//

void main() 
{ 
	vec2 r = wpos - floor(wpos);

	if( (r.x + r.y) < 1.0 ) {
		col = vec4( 0.6, 0.6, 0.8, 1.0 );
	} else {
		col = vec4( 0.2, 0.2, 0.5, 1.0 );
	}
}
