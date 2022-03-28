union Vec2_S64
{
    struct {
        s64 x, y;
    };
    s64 e[2];
};

//~
static_function void
clamp_ref_bot(s64 min, Vec2_S64 *value)
{
    clamp_ref_bot(min, &value->x);
    clamp_ref_bot(min, &value->y);
}
static_function void
clamp_ref_top(Vec2_S64 *value, s64 max)
{
    clamp_ref_top(&value->x, max);
    clamp_ref_top(&value->y, max);
}

static_function void
clamp_ref_bot(Vec2_S64 min, Vec2_S64 *value)
{
    clamp_ref_bot(min.x, &value->x);
    clamp_ref_bot(min.y, &value->y);
}
static_function void
clamp_ref_top(Vec2_S64 *value, Vec2_S64 max)
{
    clamp_ref_top(&value->x, max.x);
    clamp_ref_top(&value->y, max.y);
}


//~
static_function Vec2_S64
vec2_s64(s64 x, s64 y)
{
    return {x, y};
}

static_function Vec2_S64
vec2_s64(s64 xy)
{
    return {xy, xy};
}

static_function Vec2_S64
operator*(s64 a, Vec2_S64 b)
{
    Vec2_S64 result;
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

static_function Vec2_S64
operator*(Vec2_S64 a, s64 b)
{
    Vec2_S64 result = b * a;
    return result;
}

static_function Vec2_S64
operator-(Vec2_S64 a)
{
    Vec2_S64 result;
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

static_function Vec2_S64
operator+(Vec2_S64 a, Vec2_S64 b)
{
    Vec2_S64 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

static_function Vec2_S64
operator-(Vec2_S64 a, Vec2_S64 b)
{
    Vec2_S64 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

static_function Vec2_S64
operator*=(Vec2_S64 &a, s64 b)
{ 
    a = b * a;
    return a;
}

static_function Vec2_S64
operator+=(Vec2_S64 &a, Vec2_S64 b)
{
    a = a + b;
    return a;   
}

static_function Vec2_S64
operator-=(Vec2_S64 &a, Vec2_S64 b)
{
    a = a - b;
    return a;
}

static_function Vec2_S64
hadamard(Vec2_S64 a, Vec2_S64 b)
{
    Vec2_S64 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    return result;
}


//~
static_function v2
v2_from_vec2_s64(Vec2_S64 a)
{
    v2 result = {(f32)a.x, (f32)a.y};
    return result;
}

