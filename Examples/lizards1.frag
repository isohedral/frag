uniform sampler2D tiletex;

void main() 
{ 
	vec2 r = wpos - floor(wpos);
	col = texture( tiletex, r );
}
