#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int ret, fd;
    unsigned int random;
    printf("Starting device test code example...\n");
    printf("...\n...");

    fd = open("/dev/randomDevice", O_RDWR); 
    //fd = open("/dev/randomDevice", O_RDWR);  // Open the device with read/write access
    if (fd < 0){
          printf("Failed to open the device!");
          return -1;
     }
       

        printf("Reading from the device...\n");
        ret = read(fd, &random, sizeof(unsigned int));
        //ret = read(fd, random, sizeof(unsigned int));        // Read the response from the LKM
        if (ret < 0){
            close(fd);
            printf("Failed to read the message from the device!");
            return -1;
        }

        printf("Gia tri random thu duoc la: %u\n", random);
        close(fd);
    

    printf("End of the program\n");
    return 0;
}

