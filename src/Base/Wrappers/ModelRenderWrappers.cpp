#include "ModelRenderWrappers.h"

int IVModelRenderWrapperSDK::GetOffset(EOffsets Offset)
{
    switch (Offset)
    {
    case Off_DrawModelExecute: return 19;
    case Off_DrawModelExStaticProp: return 17;
    case Off_DrawStaticPropArrayFast: return 21;
    default: return -1;
    }
}
