
static_function b32
equals(MD_String8 a, MD_String8 b)
{
    return MD_S8Match(a, b, 0);
}

static_function MD_String8
to_md_string8(String f0_string)
{
    MD_String8 result = {f0_string.str, f0_string.size};
    return result;
}

static_function String
to_string(MD_String8 md_string)
{
    String result = {md_string.str, md_string.size};
    return result;
}

static_function String
to_string(Arena *a, MD_CodeLoc code_loc)
{
    String result = stringf(a, "%.*s:%u:%u", MD_S8VArg(code_loc.filename), code_loc.line, code_loc.column);
    return result;
}

static_function void
log_write(MD_String8 text)
{
    log_write(to_string(text));
}

static_function void
log_write_code_loc(MD_Node *node)
{
    Scratch scratch(0);
    log_write(to_string(scratch, MD_CodeLocFromNode(node)));
}

static_function b32
check_if_parsing_had_errors(MD_MessageList list)
{
    b32 result = false;
    
    for (MD_Message *message = list.first;
         message;
         message = message->next)
    {
        result = true;
        start_toast_message(Log_Warning);
        log_write("Config parser error ["_f0);
        log_write(to_string(message->string));
        log_write("], at: "_f0);
        log_write_code_loc(message->node);
        end_toast_message();
    }
    
    return result;
}
