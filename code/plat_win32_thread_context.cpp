#pragma once

//
// @todo use thread local storage provided by the compiler instead of TlsAlloc() etc?
// I think there is __thread keyword in clang but I'm not sure. Look into it.
//

static_global Platform_Data
platform_setup_data()
{
    Platform_Data result = {};
    result.tls_index = TlsAlloc();
    return result;
}

static_global void
platform_set_thread_context(Thread_Context *thread)
{
    assert(plat->platform_data.tls_index);
    TlsSetValue(plat->platform_data.tls_index, thread);
}

static_global Thread_Context *
platform_get_thread_context()
{
    assert(plat->platform_data.tls_index);
    Thread_Context *result = (Thread_Context *)TlsGetValue(plat->platform_data.tls_index);
    assert(result);
    return result;
}
