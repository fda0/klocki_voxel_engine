
#define Extern_C_Start extern "C" {
#define Extern_C_End }

#define GLEW_STATIC

#include "glew.h"
#include "wglew.h"
#pragma comment(lib, "../libs/glew32s.lib")




static_function void
gl_error_check()
{
    GLenum gl_error = glGetError();
    if (gl_error)
    {
        debug_break();
    }
}



//~
void APIENTRY
gl_custom_debug_callback(GLenum source, GLenum type, unsigned int id, GLenum severity, 
                         GLsizei length, const char *message, const void *userParam)
{
    Scratch scratch(0);
    String_List l = {};
    
    *l.push(scratch) = l2s("[GL DEBUG CALLBACK] id: ");
    *l.push(scratch) = to_string(scratch, id);
    *l.push(scratch) = l2s("; message: ");
    *l.push(scratch) = get_string((char *)message);
    *l.push(scratch) = l2s("\n");
    
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             *l.push(scratch) = l2s("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   *l.push(scratch) = l2s("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: *l.push(scratch) = l2s("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     *l.push(scratch) = l2s("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     *l.push(scratch) = l2s("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           *l.push(scratch) = l2s("Source: Other"); break;
    }
    *l.push(scratch) = l2s("\n");
    
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               *l.push(scratch) = l2s("Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: *l.push(scratch) = l2s("Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  *l.push(scratch) = l2s("Type: Undefined Behaviour"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         *l.push(scratch) = l2s("Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         *l.push(scratch) = l2s("Type: Performance"); break;
        case GL_DEBUG_TYPE_MARKER:              *l.push(scratch) = l2s("Type: Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          *l.push(scratch) = l2s("Type: Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           *l.push(scratch) = l2s("Type: Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER:               *l.push(scratch) = l2s("Type: Other"); break;
    }
    *l.push(scratch) = l2s("\n");
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         *l.push(scratch) = l2s("Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       *l.push(scratch) = l2s("Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW:          *l.push(scratch) = l2s("Severity: low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: *l.push(scratch) = l2s("Severity: notification"); break;
    }
    *l.push(scratch) = l2s("\n");
    *l.push(scratch) = l2s("\n");
    *l.push(scratch) = l2s("\0");
    
    String m = to_string(scratch, l);
    char *ms = (char *)m.str;
    OutputDebugStringA(ms);
    
    debug_break();
}




//~
static_function void
gl_initialize_debug()
{
    //-
    glDebugMessageCallback(gl_custom_debug_callback, nullptr);
    gl_error_check();
    
    //-
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    gl_error_check();
    
    //-
    glEnable(GL_DEBUG_OUTPUT);
    gl_error_check();
    
    //-
    //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    gl_error_check();
    
    
    s32 flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
    }
    else
    {
        //assert(0);
    }
}






//~
static_function HGLRC
win32_gl_initialize(HDC window_dc)
{
    PIXELFORMATDESCRIPTOR desired_pixel_format = {};
    desired_pixel_format.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
    desired_pixel_format.nVersion   = 1;
    desired_pixel_format.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cDepthBits = 32;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
    
    b32 pixel_format_id = ChoosePixelFormat(window_dc, &desired_pixel_format);
    if (!pixel_format_id)
    {
        platform_throw_error_and_exit("Opengl ChoosePixelFormat failed");
        //return false;
    }
    
    
    b32 set_pixel_format_result = SetPixelFormat(window_dc, pixel_format_id, &desired_pixel_format);
    if (!set_pixel_format_result)
    {
        platform_throw_error_and_exit("Opengl SetPixelFormat failed");
        //return false;
    }
    
    
    
    //-
    HGLRC temp_context = wglCreateContext(window_dc);
    wglMakeCurrent(window_dc, temp_context);
    
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        assert(0);
        return nullptr;
    }
    
    
    //-
    gl_error_check();
    
    
    int attribs[] =
    {
        //WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        //WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 4,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        
        //WGL_SAMPLE_BUFFERS_ARB,     GL_TRUE, // @todo @msaa
        //WGL_SAMPLES_ARB,            16,
        
        
        0, 0
    };
    
    
    //-
    HGLRC result = {};
    
    if (wglewIsSupported("WGL_ARB_create_context") == 1)
    {
        result = wglCreateContextAttribsARB(window_dc, 0, attribs);
        wglMakeCurrent(NULL,NULL);
        wglDeleteContext(temp_context);
        wglMakeCurrent(window_dc, result);
    }
    else
    {	//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
        result = temp_context;
        assert(0);
    }
    
    
    //-
    gl_error_check();
    
    
    //- Checking GL version
    const GLubyte *GLVersionString = glGetString(GL_VERSION);
    
    // Or better yet, use the GL3 way to get the version number
    int OpenGLVersion[2];
    glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
    
    //-
    gl_error_check();
    
    
    //~
#if Def_Internal
    gl_initialize_debug();
#endif
    
    //-
    if(wglSwapIntervalEXT)
    {
        //wglSwapIntervalEXT(1);
    }
    
    //-
    gl_error_check();
    assert(result);
    return result;
}

