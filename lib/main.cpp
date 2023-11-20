#include "fischl.h"
#include "rawdisk.h"

int main(){
    fischl();
    char *d = strdup("/dev/vdc");
    RawDisk *H = new RawDisk(d);
    return 0;
}