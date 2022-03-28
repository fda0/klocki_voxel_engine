#ifndef MergeSort_Type
#error "MergeSort_Type undefined! Example value: String"
#endif

#ifndef MergeSort_Compare
#error "MergeSort_Compare undefined! Example value: a->size < b->size"
#endif


// NOTE: Optional
#ifndef MergeSort_NameDecorate
#define MergeSort_NameDecorate(Name) Name
#endif

#if defined(MergeSort_AddArgType) || defined(MergeSort_AddArgName)
#  define MergeSort_AddArgComma ,
#else
#  define MergeSort_AddArgComma
#  define MergeSort_AddArgType
#  define MergeSort_AddArgName
#endif



static_function MergeSort_Type *
MergeSort_NameDecorate(merge_sort_double_array)(MergeSort_Type *source_array,
                                                MergeSort_Type *secondary_array,
                                                s64 size
                                                MergeSort_AddArgComma MergeSort_AddArgType MergeSort_AddArgName)
{
    profile_function();
    assert(size >= 0);
    
    if (size <= 1) {
        return source_array;
    }
    
    MergeSort_Type *src = source_array;
    MergeSort_Type *dest = secondary_array;
    
    for (s64 bucket_size = 1;
         ;
         bucket_size *= 2)
    {
        s64 dest_index = 0;
        
        while (dest_index < size) // iterate over dest buckets
        {
            s64 left_bucket_size = pick_smaller(bucket_size, size - dest_index);
            s64 right_bucket_size = pick_smaller(bucket_size, size - dest_index - left_bucket_size);
            assert(left_bucket_size >= 0);
            assert(right_bucket_size >= 0);
            
            s64 left = dest_index;
            s64 right = dest_index + left_bucket_size;
            
            s64 left_end = right;
            s64 right_end = right + right_bucket_size;
            
            
            assert(left < size);
            assert(left_end <= size);
            assert(right < size || right == right_end);
            assert(right_end <= size || right == right_end);
            
            
            if (right >= right_end) {
                goto fill_from_left_label;
            }
            
            
            
            for (;;) // fill single dest bucket
            {
                assert(left < size);
                assert(right < size);
                assert(dest_index < size);
                
                //- Compare
                MergeSort_Type& l = src[left];
                MergeSort_Type& r = src[right];
                b32 left_go_first = (MergeSort_Compare);
                
                
                //-
                if (left_go_first)
                {
                    assert(left < size);
                    assert(dest_index < size);
                    dest[dest_index] = src[left];
                    dest_index += 1;
                    left += 1;
                    
                    if (left >= left_end)
                    {
                        while (right < right_end)
                        {
                            assert(right < size);
                            assert(dest_index < size);
                            dest[dest_index] = src[right];
                            dest_index += 1;
                            right += 1;
                        }
                        
                        break;
                    }
                }
                else
                {
                    assert(right < size);
                    assert(dest_index < size);
                    dest[dest_index] = src[right];
                    dest_index += 1;
                    right += 1;
                    
                    
                    if (right >= right_end)
                    {
                        while (left < left_end)
                        {
                            fill_from_left_label:;
                            
                            assert(left < size);
                            assert(dest_index < size);
                            dest[dest_index] = src[left];
                            dest_index += 1;
                            left += 1;
                        }
                        
                        break;
                    }
                }
            }
            
            // +1 new output bucket filled
            
        }
        
        
        //-
        if (bucket_size*2 >= size)
        {
            break;
        }
        else
        {
            MergeSort_Type *temp = src;
            src = dest;
            dest = temp;
        }
    }
    
    
    return dest;
}

#if !defined(MergeSort_SkipUndef)
#  undef MergeSort_Type
#  undef MergeSort_Compare
#  undef MergeSort_NameDecorate
#  undef MergeSort_AddArgType
#  undef MergeSort_AddArgName
#  undef MergeSort_AddArgComma
#endif