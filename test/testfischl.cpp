#include "fischl.h"
#include <assert.h>

void testFischlInit(){
    fischl *F = new fischl;
    assert(F->init()==3);
    delete F;
}

int main(){
    testFischlInit(); 
    return 0;
}