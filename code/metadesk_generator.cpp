#pragma warning(push)
#pragma warning(disable: 4101)
#pragma warning(disable: 4244)
#include "md.h"
#include "md_c_helpers.h"
#include "md.c"
#include "md_c_helpers.c"
#pragma warning(pop)

#include "stf0.h"

struct Thread_Context
{
    Arena frame_arena;
    Arena scratch_arenas[2];
};

//
static_global MD_Arena *a_md = 0;
static_global int program_exit_code = 0;
static_global Thread_Context global_thread_context;

//
static_global Thread_Context *
platform_get_thread_context()
{
    return &global_thread_context;
}
static_function void
log_write(String string)
{
    printf("%.*s", string_expand(string));
}
#define start_toast_message(...)
#define end_toast_message() printf("\n")
#define profile_function()
#define profile_scope(...)

//-
#include "stf0_memory.h"
#include "metadesk_helpers.h"
#include "plat_win32_helpers.h"
#include "plat_win32_helpers.cpp"




//~
static_function void
set_error_nl()
{
    program_exit_code = 1;
    printf("\n");
}


static_function void
print_indent(FILE *file, s32 count=1)
{
    for_s32(i, count)
    {
        fprintf(file, "    ");
    }
}



//~
static_function void
process_named_enum(FILE *file, MD_Node *root)
{
    MD_Node *named_enum_tag = MD_TagFromString(root, "named_enum"_md, 0);
    
    if (!MD_NodeIsNil(named_enum_tag))
    {
        b32 has_any_custom_value_member = false;
        
        MD_Node *enum_type = MD_ChildFromString(named_enum_tag, "type"_md, 0)->first_child;
        
        fprintf(file, "//-\nenum %.*s", MD_S8VArg(root->string));
        
        if (enum_type->string.size)
        {
            fprintf(file, " : %.*s", MD_S8VArg(enum_type->string));
        }
        
        fprintf(file, "\n{\n");
        
        
        MD_Node *prefix = MD_ChildFromString(named_enum_tag, "prefix"_md, 0)->first_child;
        
        for (MD_EachNode(child, root->first_child))
        {
            print_indent(file);
            fprintf(file, "%.*s%.*s", MD_S8VArg(prefix->string), MD_S8VArg(child->string));
            
            MD_Node *value = MD_ChildFromString(child, "value"_md, 0)->first_child;
            if (!MD_NodeIsNil(value))
            {
                fprintf(file, " = %.*s", MD_S8VArg(value->string));
                has_any_custom_value_member = true;
            }
            
            fprintf(file, ",\n");
        }
        
        
        
        //- add _Count element
        b32 ignore_count = false;
        MD_Node *count_key = MD_ChildFromString(named_enum_tag, "count"_md, 0);
        if (!MD_NodeIsNil(count_key))
        {
            MD_Node *count_value = count_key->first_child;
            if (!MD_NodeIsNil(count_value))
            {
                B32_Result parse = parse_b32_from_string(to_string(count_value->string));
                if (parse.success)
                {
                    ignore_count = parse.value;
                }
                else
                {
                    printf("Failed to parse b32, at: ");
                    log_write_code_loc(count_value);
                    set_error_nl();
                }
            }
            else
            {
                printf("No value for [count] setting found, at: ");
                log_write_code_loc(count_key);
                set_error_nl();
            }
        }
        
        if (!ignore_count)
        {
            print_indent(file);
            if (prefix->string.size)
            {
                fprintf(file, "%.*sCount\n", MD_S8VArg(prefix->string));
            }
            else
            {
                fprintf(file, "%.*s_Count\n", MD_S8VArg(root->string));
            }
        }
        
        //-
        fprintf(file, "};\n\n");
        
        
        
        if (!has_any_custom_value_member)
        {
            fprintf(file, "static_global String %.*s_names[] =\n", MD_S8VArg(root->string));
            fprintf(file, "{\n");
            
            for (MD_Node *child = root->first_child;
                 !MD_NodeIsNil(child);
                 child = child->next)
            {
                print_indent(file);
                
                MD_Node *name = MD_ChildFromString(child, "name"_md, 0)->first_child;
                
                if (!MD_NodeIsNil(name)) {
                    fprintf(file, "\"%.*s\"_f0,\n", MD_S8VArg(name->string));
                } else {
                    fprintf(file, "\"%.*s\"_f0,\n", MD_S8VArg(child->string));
                }
            }
            
            fprintf(file, "};\n\n");
            
            fprintf(file, "static_function String get_name(%.*s value)\n{\n"
                    "    assert(value >= 0 && value < array_count(%.*s_names));\n"
                    "    s64 index = clamp((s64)0, (s64)value, (s64)array_count(%.*s_names));\n"
                    "    return %.*s_names[index];\n}\n\n\n",
                    MD_S8VArg(root->string), MD_S8VArg(root->string),
                    MD_S8VArg(root->string), MD_S8VArg(root->string));
        }
        else
        {
            fprintf(file, "static_function String get_name(%.*s value)\n{\n"
                    "    String result = {};\n"
                    "    switch (value)\n    {\n",
                    MD_S8VArg(root->string));
            
            
            for (MD_EachNode(child, root->first_child))
            {
                MD_Node *name = MD_ChildFromString(child, "name"_md, 0)->first_child;
                
                print_indent(file, 2);
                fprintf(file, "case ");
                
                if (prefix->string.size) {
                    fprintf(file, "%.*s", MD_S8VArg(prefix->string));
                } else {
                    fprintf(file, "%.*s_", MD_S8VArg(root->string));
                }
                
                fprintf(file, "%.*s: {result = ", MD_S8VArg(child->string));
                
                
                if (!MD_NodeIsNil(name)) {
                    fprintf(file, "\"%.*s\"_f0", MD_S8VArg(name->string));
                } else {
                    fprintf(file, "\"%.*s\"_f0", MD_S8VArg(child->string));
                }
                
                fprintf(file, ";} break;\n");
            }
            
            print_indent(file, 2);
            fprintf(file, "default: {result = \"*invalid*\"_f0; assert(0);} break;\n"
                    "    }\n\n"
                    "    return result;\n}\n\n");
        }
        
        
        
        MD_Node *name_value_mappings_key = MD_ChildFromString(named_enum_tag, "name_value_mappings"_md, 0);
        if (!MD_NodeIsNil(name_value_mappings_key))
        {
            MD_Node *name_value_mappings = name_value_mappings_key->first_child;
            if (!MD_NodeIsNil(name_value_mappings))
            {
                B32_Result parse = parse_b32_from_string(to_string(name_value_mappings->string));
                if (parse.success)
                {
                    if (parse.value)
                    {
                        fprintf(file, "static_global struct{String name; %.*s value;} "
                                "%.*s_name_value_mappings[] =\n",
                                MD_S8VArg(root->string),
                                MD_S8VArg(root->string));
                        fprintf(file, "{\n");
                        
                        for (MD_Node *child = root->first_child;
                             !MD_NodeIsNil(child);
                             child = child->next)
                        {
                            print_indent(file);
                            fprintf(file, "{");
                            
                            MD_Node *name = MD_ChildFromString(child, "name"_md, 0)->first_child;
                            if (!MD_NodeIsNil(name)) {
                                fprintf(file, "\"%.*s\"_f0", MD_S8VArg(name->string));
                            } else {
                                fprintf(file, "\"%.*s\"_f0", MD_S8VArg(child->string));
                            }
                            
                            fprintf(file, ", ");
                            if (prefix->string.size) {
                                fprintf(file, "%.*s", MD_S8VArg(prefix->string));
                            } else {
                                fprintf(file, "%.*s_", MD_S8VArg(root->string));
                            }
                            fprintf(file, "%.*s", MD_S8VArg(child->string));
                            
                            fprintf(file, "},\n");
                        }
                        
                        fprintf(file, "};\n\n");
                    }
                }
                else
                {
                    printf("Failed to parse b32, at: ");
                    log_write_code_loc(name_value_mappings_key);
                    set_error_nl();
                }
            }
            else
            {
                printf("Missing value, at: ");
                log_write_code_loc(name_value_mappings_key);
                set_error_nl();
            }
        }
        
        
        
    }
}





static_function void
process_load_raw_file(FILE *file, MD_Node *root)
{
    MD_Node *load_raw_file_tag = MD_TagFromString(root, "load_raw_file"_md, 0);
    if (!MD_NodeIsNil(load_raw_file_tag))
    {
        MD_Node *child = root->first_child;
        if (!child->string.size)
        {
            printf("Missing file path, at: ");
            log_write_code_loc(root);
            set_error_nl();
        }
        
        MD_String8 entire_file = MD_LoadEntireFile(a_md, child->string);
        if (!entire_file.size)
        {
            printf("File [%.*s] is empty", MD_S8VArg(child->string));
            set_error_nl();
        }
        
        fprintf(file, "static_global String %.*s = R\"__Raw--String__(", MD_S8VArg(root->string));
        fprintf(file, "%.*s", MD_S8VArg(entire_file));
        fprintf(file, ")__Raw--String__\"_f0;\n\n");
    }
}



static_function String
push_pascal_case_to_snake_scase(Arena *a, String source)
{
    String result = {};
    if (source.size)
    {
        result = push_copy(a, source);
        u8 *r = result.str;
        
        for_u64(i, source.size)
        {
            u8 s = source.str[i];
            
            if (is_upper(s))
            {
                if (i)
                {
                    push_struct(a, u8);
                    result.size += 1;
                    *r++ = '_';
                }
                *r++ = to_lower(s);
            }
            else
            {
                *r++ = s;
            }
        }
    }
    return result;
}
static_function String
push_pascal_case_to_snake_scase(Arena *a, MD_String8 source)
{
    return push_pascal_case_to_snake_scase(a, to_string(source));
}



static_function String
push_pascal_case_to_pretty_name(Arena *a, String source)
{
    String result = {};
    if (source.size)
    {
        result = push_copy(a, source);
        u8 *r = result.str;
        
        for_u64(i, source.size)
        {
            u8 s = source.str[i];
            
            if (is_upper(s))
            {
                if (i)
                {
                    push_struct(a, u8);
                    result.size += 1;
                    *r++ = ' ';
                }
                *r++ = s;
            }
            else
            {
                *r++ = s;
            }
        }
    }
    return result;
}
static_function String
push_pascal_case_to_pretty_name(Arena *a, MD_String8 source)
{
    return push_pascal_case_to_pretty_name(a, to_string(source));
}





enum Dir_Flags
{
    DirFlag_Up    = (1 << 0),
    DirFlag_Down  = (1 << 1),
    DirFlag_North = (1 << 2),
    DirFlag_South = (1 << 3),
    DirFlag_East  = (1 << 4),
    DirFlag_West  = (1 << 5),
};

enum Dir_Index
{
    DirIndex_Up,
    DirIndex_Down,
    DirIndex_North,
    DirIndex_South,
    DirIndex_East,
    DirIndex_West,
    
    DirIndex_Count
};



struct Klocki_Texture_Table_Item
{
    String file_name;
    u32 texture_id; // +1 indexing so 0 == null; (1 == valid index 0)
    Klocki_Texture_Table_Item *next;
};

struct Klocki_Texture_Table
{
    Klocki_Texture_Table_Item *slots[2048];
    Klocki_Texture_Table_Item items[4096];
    s32 item_count;
    
    u32 previous_texture_id;
    u32 stats_repeats_count;
};

static_function u32
klocki_add_to_texture_table(Klocki_Texture_Table *table, String file_name)
{
    assert(file_name.size);
    u64 hash = str_hash(file_name);
    u64 key = hash % array_count(table->slots);
    
    u32 result_texture_id = 0;
    
    Klocki_Texture_Table_Item **slot = table->slots + key;
    for (;;)
    {
        if (!(*slot))
        {
            assert(table->item_count < array_count(table->items));
            *slot = table->items + table->item_count;
            table->item_count += 1;
            
            table->previous_texture_id += 1;
            
            **slot = {
                file_name,
                table->previous_texture_id
            };
            
            result_texture_id = (*slot)->texture_id;
            break;
        }
        else
        {
            if (equals(file_name, (*slot)->file_name))
            {
                result_texture_id = (*slot)->texture_id;
                table->stats_repeats_count += 1;
                break;
            }
            else
            {
                slot = &((*slot)->next);
            }
        }
    }
    
    return result_texture_id;
}



static_function void
process_klocki_blocks(FILE *file, MD_Node *root)
{
    MD_Node *root_tag = MD_TagFromString(root, "klocki_blocks"_md, 0);
    if (!MD_NodeIsNil(root_tag))
    {
        // enum
        {
            fprintf(file, "enum Block_Type : u8\n{\n");
            fprintf(file, "    Block_Air,\n\n");
            
            for (MD_EachNode(block, root->first_child))
            {
                fprintf(file, "    Block_%.*s,\n", string_expand(block->string));
            }
            
            fprintf(file, "\n    Block_Count,\n");
            fprintf(file, "};\n\n");
        }
        
        
        // names array
        {
            fprintf(file,
                    "static_function String\n"
                    "get_block_name_string(u32 block_type)\n"
                    "{\n"
                    "    String result = {};\n"
                    "    switch (block_type)\n"
                    "    {\n"
                    "        Invalid_Default_Case;\n"
                    "        case Block_Air: result = \"Air\"_f0; break;\n");
            
            for (MD_EachNode(block, root->first_child))
            {
                fprintf(file, "        case Block_%.*s: result = \"%.*s\"_f0; break;\n",
                        MD_S8VArg(block->string), MD_S8VArg(block->string));
            }
            
            fprintf(file,
                    "    }\n"
                    "    return result;\n"
                    "}\n\n");
        }
        
        
        // pretty names array
        {
            fprintf(file,
                    "static_function String\n"
                    "get_block_pretty_name_string(u32 block_type)\n"
                    "{\n"
                    "    String result = {};\n"
                    "    switch (block_type)\n"
                    "    {\n"
                    "        Invalid_Default_Case;\n"
                    "        case Block_Air: result = \"Air\"_f0; break;\n");
            
            for (MD_EachNode(block, root->first_child))
            {
                Scratch scratch(0);
                String pretty_name = push_pascal_case_to_pretty_name(scratch, block->string);
                fprintf(file, "        case Block_%.*s: result = \"%.*s\"_f0; break;\n",
                        MD_S8VArg(block->string), string_expand(pretty_name));
            }
            
            fprintf(file,
                    "    }\n"
                    "    return result;\n"
                    "}\n\n");
        }
        
        
        
        
        // map blocks to texture ids; file names to texture ids
        {
            fprintf(file, "static_global Block_Tex_Ids map_block_to_tex_id[] =\n"
                    "{\n"
                    "    {},\n");
            
            struct
            {
                String name;
                u8 flags;
            } sides[] = {
                {"top"_f0,    DirFlag_Up},
                {"up"_f0,     DirFlag_Up},
                {"bottom"_f0, DirFlag_Down},
                {"down"_f0,   DirFlag_Down},
                {"side"_f0,   DirFlag_North|DirFlag_East|DirFlag_South|DirFlag_West},
                {"front"_f0,  DirFlag_North},
                {"north"_f0,  DirFlag_North},
                {"east"_f0,   DirFlag_East},
                {"south"_f0,  DirFlag_South},
                {"west"_f0,   DirFlag_West},
                {"yaxis"_f0, DirFlag_Up|DirFlag_Down},
                {"xaxis"_f0, DirFlag_East|DirFlag_West},
                {"zaxis"_f0, DirFlag_North|DirFlag_South},
                {"all"_f0,    DirFlag_Up|DirFlag_Down|DirFlag_North|DirFlag_East|DirFlag_South|DirFlag_West},
            };
            
            
            Scratch scratch(0);
            Klocki_Texture_Table table = {};
            
            for (MD_EachNode(block, root->first_child))
            {
                String block_snake = push_pascal_case_to_snake_scase(scratch, block->string);
                u8 dir_flags = 0;
                u32 dir_texture_ids[DirIndex_Count] = {};
                
                for (MD_EachNode(face, block->first_child))
                {
                    String face_str = to_string(face->string);
                    
                    for_s32(side_index, array_count(sides))
                    {
                        auto side = sides[side_index];
                        if (equals(face_str, side.name))
                        {
                            u8 new_flags = side.flags & ~dir_flags;
                            if (new_flags)
                            {
                                String file_name = {};
                                if (MD_NodeIsNil(face->first_child))
                                {
                                    String strings[] = {block_snake, "_"_f0, side.name, ".png"_f0};
                                    file_name = str_concatenate_array(scratch, strings, array_count(strings));
                                }
                                else
                                {
                                    file_name = to_string(face->first_child->string);
                                }
                                
                                
                                dir_flags |= new_flags;
                                u32 texture_id = klocki_add_to_texture_table(&table, file_name);
                                
                                while (new_flags)
                                {
                                    u32 dir_index = find_least_significant_bit(new_flags).index;
                                    dir_texture_ids[dir_index] = texture_id;
                                    new_flags &= (new_flags - 1);
                                }
                            }
                            else
                            {
                                printf("These faces are already set: ");
                                log_write_code_loc(face);
                                set_error_nl();
                            }
                            
                            break;
                        }
                    }
                }
                
                
                fprintf(file, "    {");
                
                String default_file_name = str_concatenate(scratch, block_snake, ".png"_f0);
                u32 default_tex_id = 0;
                for_s32(dir_index, array_count(dir_texture_ids))
                {
                    if (!dir_texture_ids[dir_index])
                    {
                        if (!default_tex_id)
                        {
                            default_tex_id = klocki_add_to_texture_table(&table, default_file_name);
                        }
                        dir_texture_ids[dir_index] = default_tex_id;
                    }
                    
                    fprintf(file, "%u,", dir_texture_ids[dir_index]-1);
                }
                
                fprintf(file, "},\n");
            }
            
            fprintf(file, "};\n\n");
            
            
            fprintf(file, "static_global To_Load_Texture array_to_load_textures[] =\n{\n");
            for_s32(item_index, table.item_count)
            {
                Klocki_Texture_Table_Item item = table.items[item_index];
                fprintf(file, "    {\"%.*s\"_f0, %u},\n", string_expand(item.file_name), item.texture_id-1);
            }
            fprintf(file, "};\n\n");
        }
    }
}





int main()
{
    a_md = MD_ArenaAlloc(1ull << 40);
    global_thread_context.frame_arena = create_virtual_arena();
    global_thread_context.scratch_arenas[0] = create_virtual_arena();
    global_thread_context.scratch_arenas[1] = create_virtual_arena();
    Scratch scratch(0);
    
    {
        Platform_String cwd = platform_get_current_directory(scratch);
        String cwd8 = win32_push_platform_string_to_string8(scratch, cwd);
        Directory d = directory_from_string(scratch, cwd8);
        if (filesystem_str_equals(d.last(), "build"_f0))
        {
            d.name_count -= 1;
        }
        if (!filesystem_str_equals(d.last(), "code"_f0))
        {
            d = directory_append(scratch, d, "code"_f0);
        }
        String new_dir_str = to_string(scratch, d);
        Platform_String new_cwd = win32_push_string8_to_platform_string(scratch, new_dir_str);
        platform_set_current_directory(new_cwd);
    }
    
    //-
    MD_ParseResult parse = MD_ParseWholeFile(a_md, MD_S8Lit("metadesk_data.mdesk"));
    program_exit_code = (check_if_parsing_had_errors(parse.errors) ? 1 : 0);
    
    {
        FILE *file = fopen("metadesk_generated.h", "wb");
        for (MD_EachNode(root, parse.node->first_child))
        {
            process_named_enum(file, root);
            process_load_raw_file(file, root);
            process_klocki_blocks(file, root);
        }
        fclose(file);
    }
    
    
    debug_break();
    return program_exit_code;
}