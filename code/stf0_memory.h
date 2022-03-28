#pragma once

static_global Arena *
get_frame_arena()
{
    Thread_Context *thread = platform_get_thread_context();
    Arena *result = &thread->frame_arena;
    return result;
}


struct Scratch
{
    Arena *arena;
    u64 position_checkpoint_;
    
    explicit Scratch(Arena *conflicting_arena)
    {
        this->arena = nullptr;
        
        Thread_Context *thread = platform_get_thread_context();
        
        for_u64(scratch_index, array_count(thread->scratch_arenas))
        {
            this->arena = thread->scratch_arenas + scratch_index;
            assert(this->arena->base);
            
            if (this->arena != conflicting_arena)
            {
                break;
            }
        }
        
        assert(this->arena);
        assert(this->arena != conflicting_arena);
        
        this->position_checkpoint_ = this->arena->position;
    }
    
    inline void reset()
    {
        reset_arena_position(this->arena, this->position_checkpoint_);
    }
    
    ~Scratch()
    {
        reset_arena_position(this->arena, this->position_checkpoint_);
    }
    
    operator Arena *()
    {
        return this->arena;
    }
    
    Scratch(const Scratch &) = delete;
};
