#include "fischl.h"
#include "rawdisk.h"

int main(){
    fischl *F = new fischl;
    F->init();
    uchar *d = strdup("/dev/vdc");
    RawDisk *H = new RawDisk(d);
    return 0;
}