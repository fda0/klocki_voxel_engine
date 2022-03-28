union Vec3_S32
{
    struct {
        s32 x, y, z;
    };
    s32 e[3];
};

//~
static_function void
clamp_ref_bot(s32 min, Vec3_S32 *value)
{
    clamp_ref_bot(min, &value->x);
    clamp_ref_bot(min, &value->y);
    clamp_ref_bot(min, &value->z);
}
static_function void
clamp_ref_top(Vec3_S32 *value, s32 max)
{
    clamp_ref_top(&value->x, max);
    clamp_ref_top(&value->y, max);
    clamp_ref_top(&value->z, max);
}

static_function void
clamp_ref_bot(Vec3_S32 min, Vec3_S32 *value)
{
    clamp_ref_bot(min.x, &value->x);
    clamp_ref_bot(min.y, &value->y);
    clamp_ref_bot(min.z, &value->z);
}
static_function void
clamp_ref_top(Vec3_S32 *value, Vec3_S32 max)
{
    clamp_ref_top(&value->x, max.x);
    clamp_ref_top(&value->y, max.y);
    clamp_ref_top(&value->z, max.z);
}


//~
static_function Vec3_S32
vec3_s32(s32 x, s32 y, s32 z)
{
    return {x, y, z};
}

static_function Vec3_S32
vec3_s32(s32 xyz)
{
    return {xyz, xyz, xyz};
}

static_function b32
operator==(Vec3_S32 a, Vec3_S32 b)
{
    return ((a.x == b.x) &&
            (a.y == b.y) &&
            (a.z == b.z));
}

static_function b32
operator!=(Vec3_S32 a, Vec3_S32 b)
{
    return !(a == b);
}


static_function Vec3_S32
operator*(s32 a, Vec3_S32 b)
{
    Vec3_S32 result;
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    
    return result;
}

static_function Vec3_S32
operator*(Vec3_S32 a, s32 b)
{
    Vec3_S32 result = b * a;
    return result;
}

static_function Vec3_S32
operator-(Vec3_S32 a)
{
    Vec3_S32 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    
    return result;
}

static_function Vec3_S32
operator+(Vec3_S32 a, Vec3_S32 b)
{
    Vec3_S32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

static_function Vec3_S32
operator-(Vec3_S32 a, Vec3_S32 b)
{
    Vec3_S32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

static_function Vec3_S32
operator*=(Vec3_S32 &a, s32 b)
{ 
    a = b * a;
    return a;
}

static_function Vec3_S32
operator+=(Vec3_S32 &a, Vec3_S32 b)
{
    a = a + b;
    return a;   
}

static_function Vec3_S32
operator-=(Vec3_S32 &a, Vec3_S32 b)
{
    a = a - b;
    return a;
}

static_function Vec3_S32
hadamard(Vec3_S32 a, Vec3_S32 b)
{
    Vec3_S32 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y; 
    result.z = a.z * b.z;
    return result;
}

//~
static_function s32
inner(Vec3_S32 a, Vec3_S32 b)
{
    s32 result = ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
    return result;
}

static_function s32
get_length_sq(Vec3_S32 a)
{
    s32 result = ((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
    return result;
}



//~
static_function v3
v3_from_vec3_s32(Vec3_S32 a)
{
    v3 result = {(f32)a.x, (f32)a.y, (f32)a.z};
    return result;
}

