#include "klocki_profile.cpp"

static_function b8
just_pressed(Button button)
{
    return (button.down && !button.was_down);
}

static_function f32
time_elapsed(s64 recent, s64 old)
{
    return time_elapsed(recent, old, plat->inv_perf_freq);
}



static_function Thread_Context *
push_thread_context(Arena *a)
{
    Thread_Context *result = push_struct_clear(a, Thread_Context);
    result->frame_arena = create_virtual_arena();
    for_u64(scratch_index, array_count(result->scratch_arenas))
    {
        result->scratch_arenas[scratch_index] = create_virtual_arena();
    }
    return result;
}

static_function void
free_thread_context(Thread_Context *context)
{
    free_virtual_arena(&context->frame_arena);
    for_u64(scratch_index, array_count(context->scratch_arenas))
    {
        free_virtual_arena(&context->scratch_arenas[scratch_index]);
    }
}

#if 0
static_function void
free_this_thread_context()
{
    Thread_Context *context = platform_get_thread_context();
    free_thread_context(context);
}
#endif



