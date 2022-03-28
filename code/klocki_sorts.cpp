//~
#define MergeSort_NameDecorate(Name) Name##_by_distance_from_p
#define MergeSort_Type Vec3_S64
#define MergeSort_Compare (get_length_sq(l - vec3_point) > get_length_sq(r - vec3_point))
#define MergeSort_AddArgType Vec3_S64
#define MergeSort_AddArgName vec3_point
#include "klocki_merge_sort_single_array.h"


//~
#define MergeSort_Type Chunk *
#define MergeSort_Compare (l->distance_from_camera > r->distance_from_camera)
#include "klocki_merge_sort_single_array.h"


//~
#define MergeSort_Type Sort_Mesh_Face
#define MergeSort_Compare (l.dist_sq > r.dist_sq)
#include "klocki_merge_sort_double_array.h"
