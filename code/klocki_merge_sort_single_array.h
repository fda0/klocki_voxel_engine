
#define MergeSort_SkipUndef
#include "klocki_merge_sort_double_array.h"

static_function void
MergeSort_NameDecorate(merge_sort_array)(MergeSort_Type *source_array,
                                         s64 size
                                         MergeSort_AddArgComma MergeSort_AddArgType MergeSort_AddArgName)
{
    profile_function();
    
    if (size > 0)
    {
        Scratch scratch(0);
        
        MergeSort_Type *temp_array = push_array(scratch, MergeSort_Type, size);
        MergeSort_Type *result =
            MergeSort_NameDecorate(merge_sort_double_array)(source_array,
                                                            temp_array,
                                                            size
                                                            MergeSort_AddArgComma MergeSort_AddArgName);
        
        if (result == temp_array)
        {
#if 0
            copy_array(result, temp_array, MergeSort_Type, size);
#else
            for_s64(index, size)
            {
                source_array[index] = temp_array[index];
            }
#endif
        }
    }
}


#undef MergeSort_SkipUndef
#undef MergeSort_Type
#undef MergeSort_Compare
#undef MergeSort_NameDecorate
#undef MergeSort_AddArgType
#undef MergeSort_AddArgName
#undef MergeSort_AddArgComma