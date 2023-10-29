#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/fs.h> 

class RawDisk{

    char* dir;
    off_t numSectors;
    off_t diskSize;

public:
    RawDisk(char *directory){
        dir = directory;
        /*dir = strdup("/dev/vdc");
        numSectors = 62914560;
        diskSize = 32212254720;*/
        
        int fd;

        // Open the block device (replace /dev/sdX with the actual device)
        fd = open(dir, O_RDONLY);
        if (fd == -1) {
            perror("Error opening device");
            exit(1);
        }

        // Use ioctl with BLKGETSIZE to get the number of sectors
        if (ioctl(fd, BLKGETSIZE64, &numSectors) == -1) {
            perror("Error getting disk size");
            close(fd);
            exit(1);
        }

        // Calculate the size in bytes
        diskSize = numSectors * 512; // Assuming a sector size of 512 bytes

        printf("====Initializing RawDisk====\n");
        printf("Number of sectors: %llu\n", numSectors);
        printf("Disk size (in bytes): %llu\n", diskSize);

        close(fd);
    }

    int rawdisk_read(off_t blockNumber, char *buffer){
        int fd;

        fd = open(dir, O_RDONLY);
        if (fd == -1) {
            perror("Error opening device");
            return -1;
        }

        // Calculate the offset in bytes
        off_t offset = blockNumber * 512;

        // Move the file pointer to the desired block
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("Error seeking to block");
            close(fd);
            return -1;
        }

        // Read a block of data
        ssize_t bytesRead = read(fd, buffer, 512);
        if (bytesRead == -1) {
            perror("Error reading from device");
            close(fd);
            return -1;
        }

        // Process the data (e.g., data recovery or analysis)
        close(fd);
        
    }

    int rawdisk_write(off_t blockNumber, char *buffer){
        int fd;

        fd = open(dir, O_WRONLY);
        if (fd == -1) {
            perror("Error opening device");
            return -1;
        }

        // Calculate the offset in bytes
        off_t offset = blockNumber * 512;

        // Move the file pointer to the desired block
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("Error seeking to block");
            close(fd);
            return -1;
        }

        // Write a block of data
        ssize_t bytesWrite = write(fd, buffer, 512);
        if (bytesWrite == -1) {
            perror("Error writing from device");
            close(fd);
            return -1;
        }

        // Process the data (e.g., data recovery or analysis)
        close(fd);
        return 0;
    }

};