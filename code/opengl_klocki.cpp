struct Gl_Buffer
{
    u32 vao;
    u32 vbo;
    
    GLsync sync;
};

struct Buffer_Voxel
{
    Gl_Buffer gl;
    u32 face_buffer_obj;
    u32 face_texture_obj;
    Frame_Voxel frame;
    
    
    Buffer_Voxel *next; // used for free list
};

struct Buffer_Ui
{
    Gl_Buffer gl;
    Frame_Ui frame;
};

struct Buffer_Debug
{
    Gl_Buffer gl;
    Frame_Debug frame;
};

struct Buffer_Line
{
    Gl_Buffer gl;
    Frame_Line frame;
};


struct Gl_State
{
    Buffer_Ui ui_buffers[Render_Buffered_Frames];
    Buffer_Debug debug_buffers[Render_Buffered_Frames];
    Buffer_Line line_buffers[Render_Buffered_Frames];
    
    u32 biggest_possible_quad_ebo;
    
    u32 blocks_texture_array;
    u32 font_texture_atlas;
    
    u32 voxel_program;
    u32 ui_program;
    u32 debug_program;
    u32 line_program;
    
    m4x4 world_transform;
    
    Virtual_Array<Buffer_Voxel> mesh_buffers;
    Buffer_Voxel *first_free_mesh_buffer;
};

static_global Gl_State gl_state = {};



//~
static_function void
gl_check_for_shader_error(u32 shader_id)
{
    s32 compilation_success;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compilation_success);
    
    if (!compilation_success)
    {
        char error_buffer[512];
        glGetShaderInfoLog(shader_id, sizeof(error_buffer), NULL, error_buffer);
        assert(0);
        exit_error();
    }
}

static_function void
gl_check_for_program_error(u32 program_id)
{
    s32 compilation_success;
    glGetProgramiv(program_id, GL_LINK_STATUS, &compilation_success);
    
    if (!compilation_success)
    {
        char error_buffer[512];
        glGetProgramInfoLog(program_id, sizeof(error_buffer), NULL, error_buffer);
        assert(0);
        exit_error();
    }
}

static_function u32
gl_compile_shader(String shader_source, GLenum shader_type)
{
    String shader_define = {};
    switch (shader_type)
    {
        case GL_VERTEX_SHADER: {
            shader_define = l2s("#version 330 core\n"
                                "#define VERTEX_SHADER\n");
        } break;
        
        case GL_FRAGMENT_SHADER: {
            shader_define = l2s("#version 330 core\n"
                                "#define FRAGMENT_SHADER\n");
        } break;
        
        default: { assert(0); } break;
    }
    
    char *shader_strs[] = {
        (char *)shader_define.str,
        (char *)shader_source.str,
    };
    s32 shader_sizes[] = {
        safe_truncate_to_s32(shader_define.size),
        safe_truncate_to_s32(shader_source.size),
    };
    
    u32 shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 2, shader_strs, shader_sizes);
    glCompileShader(shader_id);
    
    gl_check_for_shader_error(shader_id);
    return shader_id;
}


static_function u32
gl_create_program(String shaders_source)
{
    u32 vertex_shader = gl_compile_shader(shaders_source, GL_VERTEX_SHADER);
    u32 fragment_shader = gl_compile_shader(shaders_source, GL_FRAGMENT_SHADER);
    
    u32 program = glCreateProgram();
    
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    gl_check_for_program_error(program);
    return program;
}


static_function size_t
ceil_power_of_two(size_t a)
{
    size_t result = 1ull << (1 + find_most_significant_bit(a).index);
    return result;
}


template <class T> static_function void
gl_setup_buffer_shared(T *buffer, s32 face_count)
{
    glGenVertexArrays(1, &buffer->gl.vao);
    glGenBuffers(1, &buffer->gl.vbo);
    glBindVertexArray(buffer->gl.vao);
    
    GLbitfield flags = (GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_state.biggest_possible_quad_ebo);
    
    // verts
    auto vert_array = (decltype(buffer->frame.verts))nullptr;
    glBindBuffer(GL_ARRAY_BUFFER, buffer->gl.vbo);
    size_t vert_count = face_count*Block_Verts_Per_Face;
    size_t vert_size = vert_count*sizeof(*vert_array);
    glBufferStorage(GL_ARRAY_BUFFER, vert_size, 0, flags);
    buffer->frame.verts = (decltype(vert_array))glMapBufferRange(GL_ARRAY_BUFFER, 0, vert_size, flags);
    
    buffer->frame.face_count = face_count;
    
    plat->stats.total_vram_buffers_size += vert_size;
}


static_function void
gl_setup_ui_buffer(Buffer_Ui *buffer, s32 face_count)
{
    gl_setup_buffer_shared(buffer, face_count);
    
    // vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert_Ui), (void*)offset_of(Vert_Ui, p));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vert_Ui), (void*)offset_of(Vert_Ui, tex));
    glEnableVertexAttribArray(1);
    
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vert_Ui), (void*)offset_of(Vert_Ui, texid14_color12));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}


static_function void
gl_setup_voxel_buffer(Buffer_Voxel *buffer, s32 face_count)
{
    gl_setup_buffer_shared(buffer, face_count);
    
    // vertex attributes
    glVertexAttribIPointer(0, 1, GL_UNSIGNED_BYTE, sizeof(Vert_Voxel), (void*)offset_of(Vert_Voxel, packed));
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    {
        GLbitfield flags = (GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        glGenBuffers(1, &buffer->face_buffer_obj);
        glBindBuffer(GL_ARRAY_BUFFER, buffer->face_buffer_obj);
        
        u64 face_size = face_count*sizeof(u32);
        glBufferStorage(GL_ARRAY_BUFFER, face_size, 0, flags);
        buffer->frame.face_array = (u32* )glMapBufferRange(GL_ARRAY_BUFFER, 0, face_size, flags);
        
        plat->stats.total_vram_buffers_size += face_count*sizeof(u32);
    }
    
}


static_function void
gl_setup_debug_buffer(Buffer_Debug *buffer, s32 face_count)
{
    gl_setup_buffer_shared(buffer, face_count);
    
    // vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert_Debug), (void*)offset_of(Vert_Debug, p));
    glEnableVertexAttribArray(0);
    
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(Vert_Debug), (void*)offset_of(Vert_Debug, rgb18_texid8_texpos2));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}


static_function void
gl_setup_line_buffer(Buffer_Line *buffer, s32 line_count)
{
    {
        glGenVertexArrays(1, &buffer->gl.vao);
        glGenBuffers(1, &buffer->gl.vbo);
        glBindVertexArray(buffer->gl.vao);
        
        GLbitfield flags = (GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        
        // verts
        glBindBuffer(GL_ARRAY_BUFFER, buffer->gl.vbo);
        size_t vert_count = line_count*2;
        size_t vert_size = vert_count*sizeof(Vert_Line);
        glBufferStorage(GL_ARRAY_BUFFER, vert_size, 0, flags);
        buffer->frame.verts = (Vert_Line *)glMapBufferRange(GL_ARRAY_BUFFER, 0, vert_size, flags);
        
        buffer->frame.line_max_count = line_count;
        plat->stats.total_vram_buffers_size += vert_size;
    }
    
    // vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vert_Line), (void*)offset_of(Vert_Line, p));
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vert_Line), (void*)offset_of(Vert_Line, color));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}




static_function void
gl_setup_shaders()
{
    gl_state.voxel_program = gl_create_program(gl_shader_chunk);
    gl_state.ui_program = gl_create_program(gl_shader_ui);
    gl_state.debug_program = gl_create_program(gl_shader_debug);
    gl_state.line_program = gl_create_program(gl_shader_line);
    
    // create biggest possible ebo buffer that will be reused for all quad draw calls
    {
        GLbitfield flags = (GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
        glGenBuffers(1, &gl_state.biggest_possible_quad_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_state.biggest_possible_quad_ebo);
        
        u64 biggest_ebo_count = (Chunk_Dim_X*Chunk_Dim_Y*Chunk_Dim_Z * 6/*faces*/ * 6/*indices per quad*/);
        u64 biggest_ebo_size = biggest_ebo_count*sizeof(s32);
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, biggest_ebo_size, 0, flags);
        
        s32 *ebo_indicies = (s32 *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, biggest_ebo_size, flags);
        
        s32 vertex_index = 0;
        for (u64 ebo_index = 0;
             ebo_index < biggest_ebo_count;
             ebo_index += 6, vertex_index += 4)
        {
            ebo_indicies[ebo_index + 0] = vertex_index + 0;
            ebo_indicies[ebo_index + 1] = vertex_index + 1;
            ebo_indicies[ebo_index + 2] = vertex_index + 2;
            ebo_indicies[ebo_index + 3] = vertex_index + 2;
            ebo_indicies[ebo_index + 4] = vertex_index + 1;
            ebo_indicies[ebo_index + 5] = vertex_index + 3;
        }
        
        plat->stats.total_vram_buffers_size += biggest_ebo_size;
    }
    
    
    //
    //runtime_assert(Render_Buffered_Frames == array_count(gl_state.world_buffers));
    runtime_assert(Render_Buffered_Frames == array_count(gl_state.ui_buffers));
    
    for_s32(buffer_index, Render_Buffered_Frames)
    {
        gl_setup_debug_buffer(gl_state.debug_buffers + buffer_index, kilobytes(32));
        gl_setup_ui_buffer(gl_state.ui_buffers + buffer_index, kilo(16));
        gl_setup_line_buffer(gl_state.line_buffers + buffer_index, megabytes(1));
    }
    
    
    gl_state.mesh_buffers = create_virtual_array<Buffer_Voxel>(1); // index 0 => nil
}




//~
static_function PLATFORM_CREATE_MESH_BUFFER(gl_create_mesh_buffer)
{
    profile_function();
    
    Buffer_Voxel *buffer = gl_state.first_free_mesh_buffer;
    if (buffer)
    {
        gl_state.first_free_mesh_buffer = buffer->next;
    }
    else
    {
        buffer = gl_state.mesh_buffers.grow();
    }
    
    *buffer = {};
    gl_setup_voxel_buffer(buffer, face_count);
    
    s64 pointer_dist = ((s64)buffer - (s64)gl_state.mesh_buffers.array) / sizeof(*buffer);
    Gpu_Mesh_Buffer result = {};
    result.frame = &buffer->frame;
    result.mesh_id = safe_truncate_to_u32(pointer_dist);
    
    return result;
}

static_function PLATFORM_FINALIZE_MESH_BUFFER(gl_finalize_mesh_buffer)
{
    profile_function();
    
    if (mesh_id)
    {
        Buffer_Voxel *buffer = gl_state.mesh_buffers.at(mesh_id);
        
        glGenTextures(1, &buffer->face_texture_obj);
        glBindTexture(GL_TEXTURE_BUFFER, buffer->face_texture_obj);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, buffer->face_buffer_obj);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
        
        if (unmap_buffers)
        {
            glBindBuffer(GL_ARRAY_BUFFER, buffer->face_buffer_obj);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            glBindBuffer(GL_ARRAY_BUFFER, buffer->gl.vbo);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
}

static_function PLATFORM_DELETE_MESH_BUFFER(gl_delete_mesh_buffer)
{
    profile_function();
    
    if (mesh_id)
    {
        Buffer_Voxel *buffer = gl_state.mesh_buffers.at(mesh_id);
        
        
        glDeleteTextures(1, &buffer->face_texture_obj);
        glDeleteVertexArrays(1, &buffer->gl.vao);
        glDeleteBuffers(1, &buffer->gl.vbo);
        glDeleteBuffers(1, &buffer->face_buffer_obj);
        
        
        plat->stats.total_vram_buffers_size -= buffer->frame.face_count*sizeof(u32);
        plat->stats.total_vram_buffers_size -= 4*buffer->frame.face_count*sizeof(Vert_Voxel);
        
        
        
        if (mesh_id + 1 == gl_state.mesh_buffers.count)
        {
            gl_state.mesh_buffers.reset(gl_state.mesh_buffers.count - 1);
        }
        else
        {
            buffer->next = gl_state.first_free_mesh_buffer;
            gl_state.first_free_mesh_buffer = buffer;
        }
    }
}



static_function PLATFORM_CREATE_TEXTURE_2D(gl_create_texture_2d)
{
    u32 *atlas = (target == Atlas_Font ? &gl_state.font_texture_atlas : &gl_state.blocks_texture_array);
    assert(!(*atlas));
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, atlas);
    glBindTexture(GL_TEXTURE_2D, *atlas);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0,
                 GL_BGRA_EXT, GL_UNSIGNED_BYTE, memory);
    
    //-
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
#if 1
    //runtime_assert(1 << 4 == Block_Side);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
    glGenerateMipmap(GL_TEXTURE_2D);
#endif
}




static_function PLATFORM_CREATE_TEXTURE_3D(gl_create_texture_3d)
{
    u32 *atlas = (target == Atlas_Font ? &gl_state.font_texture_atlas : &gl_state.blocks_texture_array);
    assert(!(*atlas));
    
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, atlas);
    glBindTexture(GL_TEXTURE_2D_ARRAY, *atlas);
    
    s32 mip_level = 5;
    runtime_assert((1 << (mip_level-1)) == Block_Side);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mip_level, GL_RGBA8,
                   Block_Side, Block_Side,
                   255);
    
    //-
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
}


static_function PLATFORM_FILL_TEXTURE_3D(gl_fill_texture_3d)
{
    u32 *atlas = (target == Atlas_Font ? &gl_state.font_texture_atlas : &gl_state.blocks_texture_array);
    assert(*atlas);
    glBindTexture(GL_TEXTURE_2D_ARRAY, *atlas);
    
    assert(depth_index >= 0);
    assert(depth_index < 255);
    
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0 /*mipmap level*/,
                    0, 0 /*x y offsets*/, depth_index,
                    Block_Side, Block_Side /*width, height*/, 1,
                    GL_BGRA_EXT, GL_UNSIGNED_BYTE, memory);
}

static_function PLATFORM_GENERATE_MIPMAP_3D(gl_generate_mipmap_3d)
{
    u32 atlas = (target == Atlas_Font ? gl_state.font_texture_atlas : gl_state.blocks_texture_array);
    assert(atlas);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
}






template <class T> static_function void
gl_lock_buffer(T *buffer)
{
    if (buffer->gl.sync)
    {
        glDeleteSync(buffer->gl.sync);
    }
    buffer->gl.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

template <class T> static_function void
gl_wait_buffer(T *buffer, char *name)
{
    // @todo skip render instead of waiting
    if (buffer->gl.sync)
    {
        s64 start_time = time_perf();
        
        for (;;)
        {
            
            GLenum wait_res = glClientWaitSync(buffer->gl.sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (wait_res == GL_ALREADY_SIGNALED || wait_res == GL_CONDITION_SATISFIED)
            {
                glDeleteSync(buffer->gl.sync);
                buffer->gl.sync = {};
                break;
            }
            else
            {
                os_sleep_ms(0);
            }
        }
        
        
        s64 end_time = time_perf();
        f32 waited_ms = 1000.f*time_elapsed(end_time, start_time);
        if (waited_ms > 0.5f)
        {
            Scratch scratch(0);
            char *message = cstrf(scratch, "Waited for %s: %.3fms\n", name, waited_ms);
            OutputDebugStringA(message);
        }
    }
}



static_function
PLATFORM_SYNC_FRAME_RENDERING(gl_sync_frame_rendering)
{
    profile_function();
    
    assert(render_buffer_index < Render_Buffered_Frames);
    gl_wait_buffer(gl_state.debug_buffers + render_buffer_index, "debug_buffer");
    gl_wait_buffer(gl_state.line_buffers + render_buffer_index, "line_buffer");
    gl_wait_buffer(gl_state.ui_buffers + render_buffer_index, "ui_buffer");
}





static_function void
gl_run_voxel_shader(Buffer_Voxel *buffer, f32 mesh_age_t)
{
    Frame_Voxel *frame = &buffer->frame;
    
#if Def_Internal
    if (plat->debug_show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
#endif
    
    
    u32 uni_mesh_translate = glGetUniformLocation(gl_state.voxel_program, "uni_mesh_translate");
    glUniform3fv(uni_mesh_translate, 1, frame->mesh_translate.e);
    u32 uni_mesh_age = glGetUniformLocation(gl_state.voxel_program, "uni_mesh_age_t");
    glUniform1f(uni_mesh_age, mesh_age_t);
    
    glBindVertexArray(buffer->gl.vao);
    glBindTexture(GL_TEXTURE_BUFFER, buffer->face_texture_obj);
    glDrawElements(GL_TRIANGLES, frame->face_count*6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    
#if Def_Internal
    if (plat->debug_show_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif
    
    
    plat->stats.game_quads_last_frame += frame->face_count;
}


static_function void
gl_run_ui_shader(Buffer_Ui *buffer)
{
    Frame_Ui *frame = &buffer->frame;
    
    if (frame->face_index)
    {
        glBindVertexArray(buffer->gl.vao);
        assert(frame->face_index);
        glDrawElements(GL_TRIANGLES, frame->face_index*6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        gl_lock_buffer(buffer);
    }
    
    frame->face_index = 0;
}



static_function void
gl_run_debug_shader(Buffer_Debug *buffer)
{
    Frame_Debug *frame = &buffer->frame;
    
    if (frame->face_index)
    {
#if Def_Internal
        if (plat->debug_show_wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
#endif
        
        
        glBindVertexArray(buffer->gl.vao);
        glDrawElements(GL_TRIANGLES, frame->face_index*6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        
        
#if Def_Internal
        if (plat->debug_show_wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
#endif
    }
    plat->stats.game_quads_last_frame += frame->face_index;
    frame->face_index = 0;
}


static_function void
gl_run_line_shader(Buffer_Line *buffer)
{
    Frame_Line *frame = &buffer->frame;
    
    if (frame->line_index)
    {
        glBindVertexArray(buffer->gl.vao);
        glDrawArrays(GL_LINES, 0, frame->line_index*2);
        glBindVertexArray(0);
    }
    plat->stats.game_quads_last_frame += frame->line_index/2;
    frame->line_index = 0;
}





static_function PLATFORM_RENDER_MESH_BUFFER(gl_render_mesh_buffer)
{
    //profile_function();
    if (mesh_id)
    {
        Buffer_Voxel *buffer = gl_state.mesh_buffers.at(mesh_id);
        gl_run_voxel_shader(buffer, mesh_age_t);
    }
    else
    {
        assert(0);
    }
}



static_function PLATFORM_SET_WORLD_TRANSFORM(gl_set_world_transform)
{
    u32 uni_world_transform = glGetUniformLocation(gl_state.voxel_program, "uni_world_transform");
    glUniformMatrix4fv(uni_world_transform, 1, GL_FALSE, mat->flat);
    gl_state.world_transform = *mat;
    
    u32 uni_fog = glGetUniformLocation(gl_state.voxel_program, "uni_fog_translate");
    glUniform3fv(uni_fog, 1, camera_rel_p.e);
}







static_function void
gl_setup_world_settings()
{
    profile_function();
    
    glUseProgram(gl_state.voxel_program);
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, gl_state.blocks_texture_array);
    
    // @todo test if changing glViewport on changes only affects perfomance?
    glViewport(0, 0, plat->window_width, plat->window_height);
    
    //~
    glClearColor(.53f, .63f, .75f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    //glEnable(GL_MULTISAMPLE); // @todo do ssaa instead of msaa? or research other antialiasing techniques. fxaa?
    
    plat->stats.game_quads_last_frame = 0;
}




static_function void
gl_render_all(u32 render_buffer_index)
{
    profile_function();
    
    Buffer_Ui *ui_buffer = gl_state.ui_buffers + render_buffer_index;
    Buffer_Debug *debug_buffer = gl_state.debug_buffers + render_buffer_index;
    Buffer_Line *line_buffer = gl_state.line_buffers + render_buffer_index;
    
    
    //~
    {
        profile_scope("Opengl Debug draw call");
        glDisable(GL_CULL_FACE);
        
        glUseProgram(gl_state.debug_program);
        glBindTexture(GL_TEXTURE_2D_ARRAY, gl_state.blocks_texture_array);
        
        u32 uni_transform = glGetUniformLocation(gl_state.debug_program, "uni_transform");
        glUniformMatrix4fv(uni_transform, 1, GL_FALSE, gl_state.world_transform.flat);
        
        gl_run_debug_shader(debug_buffer);
    }
    
    {
        profile_scope("Opengl Line draw call");
        //glDisable(GL_CULL_FACE);
        
        glUseProgram(gl_state.line_program);
        
#if 1
        u32 uni_transform = glGetUniformLocation(gl_state.line_program, "uni_transform");
        glUniformMatrix4fv(uni_transform, 1, GL_FALSE, gl_state.world_transform.flat);
#else
        {
            f32 a = safe_ratio_1(2.f, (f32)plat->window_width);
            f32 b = safe_ratio_1(2.f, (f32)plat->window_height);
            m4x4 ui_transform =
            {
                a,  0,  0, -1,
                0,  -b, 0, 1,
                0,  0,  1, 0,
                0,  0,  0, 1
            };
            
            u32 transform_uniform = glGetUniformLocation(gl_state.line_program, "uni_transform");
            glUniformMatrix4fv(transform_uniform, 1, GL_FALSE, ui_transform.flat);
        }
#endif
        
        gl_run_line_shader(line_buffer);
    }
    
    
    //~
    {
        profile_scope("Opengl Ui draw call");
        
        glUseProgram(gl_state.ui_program);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        u32 uni_tex_array = glGetUniformLocation(gl_state.ui_program, "uni_tex_array");
        u32 uni_tex_atlas = glGetUniformLocation(gl_state.ui_program, "uni_tex_atlas");
        glUniform1i(uni_tex_array, 0);
        glUniform1i(uni_tex_atlas, 1);
        
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, gl_state.blocks_texture_array);
        
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, gl_state.font_texture_atlas);
        
        
        {
            f32 a = safe_ratio_1(2.f, (f32)plat->window_width);
            f32 b = safe_ratio_1(2.f, (f32)plat->window_height);
            m4x4 ui_transform =
            {
                a,  0,  0, -1,
                0,  -b, 0, 1,
                0,  0,  1, 0,
                0,  0,  0, 1
            };
            
            u32 transform_uniform = glGetUniformLocation(gl_state.ui_program, "uni_transform");
            glUniformMatrix4fv(transform_uniform, 1, GL_FALSE, ui_transform.flat);
        }
        
        gl_run_ui_shader(ui_buffer);
    }
    
    
    //~
    {
        profile_scope("Opengl Swap buffers");
        SwapBuffers(state32.window_dc);
    }
}

