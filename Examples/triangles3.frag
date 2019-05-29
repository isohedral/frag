//
// Proper equilateral triangles
//

// Note that matrices in GLSL are described in column-major order.
const mat2 M = mat2(1, 0, 0, 0.5*sqrt(3)) * mat2(1, 0, 0.5, 1);
const mat2 S = inverse(M);

void main() 
{ 
	vec2 nw = S * wpos;
	vec2 r = nw - floor(nw);

	if( (r.x + r.y) < 1.0 ) {
		col = vec4( 0.6, 0.6, 0.8, 1.0 );
	} else {
		col = vec4( 0.2, 0.2, 0.5, 1.0 );
	}
}
