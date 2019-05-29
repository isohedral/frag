//
// Hexagons
//

const vec2 T1 = vec2( 0.25 * sqrt(3.0), 0.25 );
const vec2 T2 = vec2( 0.25 * sqrt(3.0), -0.25 );
const mat2 B = mat2( T1, T2 );
const mat2 CB = inverse( B );

const int cells[18] = 
	int[18]( 0, 0, 2, 2, 2, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2 );

void main() 
{ 
	vec2 p = CB * wpos;
	vec2 fp = floor(p);
	vec2 r = p - fp;

	int a = int(fp.x) - (fp.x<0.0?1:0);
	int b = int(fp.y) - (fp.y<0.0?1:0);

	int idx = cells[(a%3)*6 + (b%3)*2 + ((r.x+r.y)<1.0 ? 0 : 1)];

	// ivec2 ip = ivec2(fp+1e7);
	// int idx = cells[(ip.x%3)*6 + (ip.y%3)*2 + ((r.x+r.y)<1.0 ? 0 : 1)];

	if( idx == 0 ) {
		col = vec4( 206.0 / 255.0, 236.0 / 255.0, 242.0 / 255.0, 1.0 );
	} else if( idx == 1 ) {
		col = vec4( 2.0 / 255.0, 48.0 / 255.0, 89.0 / 255.0, 1.0 );
	} else {
		col = vec4( 105.0 / 255.0, 167.0 / 255.0, 191.0 / 255.0, 1.0 );
	}
}
