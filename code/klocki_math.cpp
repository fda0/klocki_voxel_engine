

//~
static_function f32
cos01(f32 t)
{
    return 0.5f + 0.5f*cos(t);
}



static_function v2
normalize(v2 a)
{
    v2 result = a * (1.f / get_length(a));
    return result;
}
static_function v3
normalize(v3 a)
{
    v3 result = a * (1.f / get_length(a));
    return result;
}
static_function v4
normalize(v4 a)
{
    v4 result = a * (1.f / get_length(a));
    return result;
}


static_function v2
normalize0(v2 a)
{
    v2 result = {};
    f32 len = get_length(a);
    if (len)
    {
        result = a * (1.f / len);
    }
    return result;
}
static_function v3
normalize0(v3 a)
{
    v3 result = {};
    f32 len = get_length(a);
    if (len)
    {
        result = a * (1.f / len);
    }
    return result;
}
static_function v4
normalize0(v4 a)
{
    v4 result = {};
    f32 len = get_length(a);
    if (len)
    {
        result = a * (1.f / len);
    }
    return result;
}






static_function v3
cross(v3 a, v3 b)
{
    v3 result = {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
    return result;
}








//~
static_function v3
operator*(m4x4 mat, v3 p)
{
    return transform(mat, V4(p, 1)).xyz;
}

static_function m4x4
translate(m4x4 mat, v3 t)
{
    mat.e[0][3] += t.x;
    mat.e[1][3] += t.y;
    mat.e[2][3] += t.z;
    return mat;
}

static_function m4x4
rows3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 result =
    {
        X.x, X.y, X.z, 0,
        Y.x, Y.y, Y.z, 0,
        Z.x, Z.y, Z.z, 0,
        0,   0,   0, 1,
    };
    
    return result;
}

static_function m4x4
columns3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 result =
    {
        X.x, Y.x, Z.x, 0,
        X.y, Y.y, Z.y, 0,
        X.z, Y.z, Z.z, 0,
        0,   0,   0, 1
    };
    return result;
}

static_function v3
get_column3(m4x4 mat, u32 col)
{
    assert(col < 4);
    v3 result = {mat.e[0][col], mat.e[1][col], mat.e[2][col]};
    return result;
}


static_function v4
get_column4(m4x4 mat, u32 col)
{
    assert(col < 4);
    v4 result = {mat.e[0][col], mat.e[1][col], mat.e[2][col], mat.e[3][col]};
    return result;
}








//~
static_function m4x4_inv
perspective_projection(f32 aspect_width_over_height, f32 focal_length, f32 near_clip_plane, f32 far_clip_plane)
{
    f32 a = 1.0f;
    f32 b = aspect_width_over_height;
    f32 c = focal_length; // NOTE(casey): This should really be called "film back distance"
    
    f32 n = near_clip_plane; // NOTE(casey): Near clip plane _distance_
    f32 f = far_clip_plane; // NOTE(casey): Far clip plane _distance_
    
    // NOTE(casey): These are the perspective correct terms, for when you divide by -z
    f32 d = (n+f) / (n-f);
    f32 e = (2*f*n) / (n-f);
    
    
    m4x4_inv result = {};
    result.forward =
    {
        a*c, 0,   0,  0,
        0,   b*c, 0,  0,
        0,   0,   d,  e,
        0,   0,   -1, 0,
    };
    result.inverse =
    {
        1/(a*c), 0,       0,   0,
        0,       1/(b*c), 0,   0,
        0,       0,       0,   -1,
        0,       0,       1/e, d/e,
    };
    
    
#if Def_SuperSlow
    m4x4 I = result.inverse*result.forward;
    assert(debug_eq(I, identity()));
#endif
    
    
    return result;
}


static_function m4x4_inv
camera_transform(v3 x, v3 y, v3 z, v3 p)
{
    m4x4_inv result;
    
    // TODO(casey): It seems really suspicious that unary negation binds first
    // to the m4x4... is that actually the C++ grammar?  I guess it is :(
    m4x4 a = rows3x3(x, y, z);
    v3 ap = -(a*p);
    result.forward = translate(a, ap);
    
    v3 ix = x * (1.f/get_length_sq(x));
    v3 iy = y * (1.f/get_length_sq(y));
    v3 iz = z * (1.f/get_length_sq(z));
    v3 ip = {
        ap.x*ix.x + ap.y*iy.x + ap.z*iz.x,
        ap.x*ix.y + ap.y*iy.y + ap.z*iz.y,
        ap.x*ix.z + ap.y*iy.z + ap.z*iz.z
    };
    
    m4x4 b = columns3x3(ix, iy, iz);
    result.inverse = translate(b, -ip);
    
#if Def_SuperSlow
    m4x4 I = result.inverse*result.forward;
    assert(debug_eq(I, identity()));
#endif
    
    return result;
}
