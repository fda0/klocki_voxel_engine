#pragma once

static_function Rect2
glyph_tex_from_xy(s32 x_coord, s32 y_coord)
{
    y_coord = (Glyph_Atlas_Y/Glyph_Side) - y_coord - 1;
    
    v2 side = {
        (f32)Glyph_Side / (f32)Glyph_Atlas_X,
        (f32)Glyph_Side / (f32)Glyph_Atlas_Y,
    };
    f32 x = (f32)x_coord * side.x;
    f32 y = (f32)y_coord * side.y;
    
    Rect2 result = {
        x, y,
        x + side.x, y + side.y
    };
    return result;
}

static_function Rect2
glyph_tex_from_codepoint(u32 codepoint)
{
    Rect2 result = {};
    
    if (codepoint >= '!' && codepoint <= 'Z')
    {
        u32 cell = codepoint - '!' + 1;
        u32 cell_y = cell / 10;
        u32 cell_x = cell - (10*cell_y);
        result = glyph_tex_from_xy(cell_x, cell_y);
    }
    else
    {
        result = glyph_tex_from_xy(9, 5);
    }
    
    return result;
}



static_function void
render_glyph_tex(Rect2 rect, Rect2 tex, v4 color = V4(1))
{
    Frame_Ui *frame = plat->frame_ui;
    
    u32 color12 = pack_color12(color);
    Vert_Ui *verts = frame->verts + frame->face_index*Block_Verts_Per_Face;
    frame->face_index += 1;
    
    u32 packed = (Bitmask_14 << 12) | color12;
    
    verts[0] = {{rect.min.x, rect.min.y, 0}, {tex.min.x, tex.max.y}, packed};
    verts[1] = {{rect.min.x, rect.max.y, 0}, {tex.min.x, tex.min.y}, packed};
    verts[2] = {{rect.max.x, rect.min.y, 0}, {tex.max.x, tex.max.y}, packed};
    verts[3] = {{rect.max.x, rect.max.y, 0}, {tex.max.x, tex.min.y}, packed};
}


static_function void
render_glyph(v2 p, u32 codepoint, f32 scale = Default_Text_Scale, v4 color = V4(1))
{
    p.x = round_f32(p.x);
    p.y = round_f32(p.y);
    Rect2 rect = rect_min_dim(p, V2((f32)Glyph_Side*scale));
    Rect2 tex = glyph_tex_from_codepoint(codepoint);
    
    render_glyph_tex(rect, tex, color);
}

