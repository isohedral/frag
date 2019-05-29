uniform sampler2D tiletex;

const mat2 S = mat2(1.0,0.0,0.5,1.0) * mat2(1.0,0.0,0.0,2.0/sqrt(3.0));

void main() 
{ 
	vec2 nw = S * wpos;
	vec2 r = mod( nw, 1.0 ); 

	col = texture( tiletex, r.xy );
}
