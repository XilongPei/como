#include "mimalloc_utils.h"
#include <mimalloc.h>



namespace como {
int MimallocUtils::setupFscpMemAreas(void* MemAreasInfo, int numAreas, COMO_MALLOC& mimalloc, FREE_MEM_FUNCTION& mifree) {
    int ret = setupFscpMemAreas(MemAreasInfo, numAreas, mimalloc, mifree);
    return ret;
}
void* MimallocUtils::area_malloc(short iMemArea, size_t size) {
    return mi_area_malloc(iMemArea, size);
}

void MimallocUtils::area_free(short iMemArea, const void* ptr) {
    mi_area_free(iMemArea, ptr);
}

}  // namespace como