#include "Utility/MaxEmptyRectFinder.h"

#if defined(_MSC_VER) && !defined(NDEBUG)
    #pragma optimize("", on)
#endif

namespace aby::util {
    
    template class MaxEmptyRectFinder<EGridType::FLAT_VECTOR>;
    template class MaxEmptyRectFinder<EGridType::VECTOR_2D>;
    template class MaxEmptyRectFinder<EGridType::RAW_FLAT_ARRAY>;
    template class MaxEmptyRectFinder<EGridType::RAW_2D_ARRAY>;


}

#if defined(_MSC_VER) && !defined(NDEBUG)
#   pragma optimize( "", off)
#endif