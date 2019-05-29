//
// Breath of the Compassionate
//

float hr2 = 0.5*sqrt(2.0);
vec2 o = vec2(0.0, 0.0);
vec2 l45 = vec2(hr2, hr2);
vec2 l22 = vec2(0.5*(l45.x+1.0), 0.5*l45.y);

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

void main()
{
    vec2 p = abs(mod(wpos,2.0) - 1.0);
    p = forceRight( p, o, l45 );
    p = forceRight( p, l22, o );
    float d = abs(p.x - hr2);

    if( d < 0.05 ) {
        col = vec4(0,0,0,1);
    } else if( p.x < hr2 ) {
        col = vec4(121.0/255.0, 86.0/255.0, 163.0/255.0, 1.0);
    } else {
        col = vec4(1,1,1,1);
    }
}
