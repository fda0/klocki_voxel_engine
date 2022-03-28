union Vec3_S64
{
    struct {
        s64 x, y, z;
    };
    s64 e[3];
};

//~
static_function void
clamp_ref_bot(s64 min, Vec3_S64 *value)
{
    clamp_ref_bot(min, &value->x);
    clamp_ref_bot(min, &value->y);
    clamp_ref_bot(min, &value->z);
}
static_function void
clamp_ref_top(Vec3_S64 *value, s64 max)
{
    clamp_ref_top(&value->x, max);
    clamp_ref_top(&value->y, max);
    clamp_ref_top(&value->z, max);
}

static_function void
clamp_ref_bot(Vec3_S64 min, Vec3_S64 *value)
{
    clamp_ref_bot(min.x, &value->x);
    clamp_ref_bot(min.y, &value->y);
    clamp_ref_bot(min.z, &value->z);
}
static_function void
clamp_ref_top(Vec3_S64 *value, Vec3_S64 max)
{
    clamp_ref_top(&value->x, max.x);
    clamp_ref_top(&value->y, max.y);
    clamp_ref_top(&value->z, max.z);
}


//~
static_function Vec3_S64
vec3_s64(s64 x, s64 y, s64 z)
{
    return {x, y, z};
}

static_function Vec3_S64
vec3_s64(s64 xyz)
{
    return {xyz, xyz, xyz};
}

static_function Vec3_S64
floor_v3_to_vec3_s64(v3 a)
{
    Vec3_S64 result = {
        floor_f32_to_s32(a.x),
        floor_f32_to_s32(a.y),
        floor_f32_to_s32(a.z),
    };
    return result;
}


static_function b32
operator==(Vec3_S64 a, Vec3_S64 b)
{
    return ((a.x == b.x) &&
            (a.y == b.y) &&
            (a.z == b.z));
}

static_function b32
operator!=(Vec3_S64 a, Vec3_S64 b)
{
    return !(a == b);
}


static_function Vec3_S64
operator*(s64 a, Vec3_S64 b)
{
    Vec3_S64 result;
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    
    return result;
}

static_function Vec3_S64
operator*(Vec3_S64 a, s64 b)
{
    Vec3_S64 result = b * a;
    return result;
}

static_function Vec3_S64
operator-(Vec3_S64 a)
{
    Vec3_S64 result;
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    
    return result;
}

static_function Vec3_S64
operator+(Vec3_S64 a, Vec3_S64 b)
{
    Vec3_S64 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

static_function Vec3_S64
operator-(Vec3_S64 a, Vec3_S64 b)
{
    Vec3_S64 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

static_function Vec3_S64
operator*=(Vec3_S64 &a, s64 b)
{ 
    a = b * a;
    return a;
}

static_function Vec3_S64
operator+=(Vec3_S64 &a, Vec3_S64 b)
{
    a = a + b;
    return a;   
}

static_function Vec3_S64
operator-=(Vec3_S64 &a, Vec3_S64 b)
{
    a = a - b;
    return a;
}

static_function Vec3_S64
hadamard(Vec3_S64 a, Vec3_S64 b)
{
    Vec3_S64 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y; 
    result.z = a.z * b.z;
    return result;
}

//~
static_function s64
inner(Vec3_S64 a, Vec3_S64 b)
{
    s64 result = ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
    return result;
}

static_function s64
get_length_sq(Vec3_S64 a)
{
    s64 result = ((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
    return result;
}



//~
static_function v3
v3_from_vec3_s64(Vec3_S64 a)
{
    v3 result = {(f32)a.x, (f32)a.y, (f32)a.z};
    return result;
}

