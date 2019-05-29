//
// Use a shear operation to get isosceles triangles.
//

void main() 
{ 
	vec2 nw = mat2(1.0, 0.0, -0.5, 1.0) * wpos;
	vec2 r = nw - floor(nw);

	if( (r.x + r.y) < 1.0 ) {
		col = vec4( 0.6, 0.6, 0.8, 1.0 );
	} else {
		col = vec4( 0.2, 0.2, 0.5, 1.0 );
	}
}
