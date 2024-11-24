#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include "MemoryAccess.h"
#include <iostream>


#include <sys/mman.h>
#include <unistd.h>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <iostream>

MemoryAccess::MemoryAccess() {
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        perror("open");
        throw std::runtime_error("Failed to open /dev/mem");
    }
}

MemoryAccess::~MemoryAccess() {
    if (mem_fd >= 0) {
        close(mem_fd);
    }
}

uint32_t MemoryAccess::readMemory(off_t address, size_t busSize) const {
    void* mapped = mapAddress(address, busSize);
    uint32_t value = *((uint32_t*)mapped);
    unmapAddress(mapped, busSize);
    return value;
}

void MemoryAccess::writeMemory(off_t address, size_t busSize, uint32_t value) const {
    void* mapped = mapAddress(address, busSize);
    *((uint32_t*)mapped) = value;
    unmapAddress(mapped, busSize);
}


void* MemoryAccess::mapAddress(off_t address, size_t busSize) const {
    size_t pageSize = getpagesize();
    off_t pageAddress = address & ~(pageSize - 1);
    size_t mapSize = ((address + busSize) - pageAddress + pageSize - 1) & ~(pageSize - 1);

   /* std::cout << "Attempting to map memory:" << std::endl;
    std::cout << "Address: " << address << ", Bus Size: " << busSize << ", Page Address: " << pageAddress << ", Map Size: " << mapSize << std::endl;
*/

    if (mapSize > pageSize * 1000) { // Arbitrary limit, adjust as needed
        std::cerr << "Map size is too large, aborting mmap" << std::endl;
        return nullptr;
    }

    void* mapped_base = mmap(0, mapSize, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, pageAddress);
    if (mapped_base == MAP_FAILED) {

    	std::cout << "Attempting to map memory:" << std::endl;
    	    std::cout << "Address: " << address << ", Bus Size: " << busSize << ", Page Address: " << pageAddress << ", Map Size: " << mapSize << std::endl;

        std::cerr << "mmap failed: " << std::strerror(errno) << std::endl;
        throw std::runtime_error("Failed to mmap");
    }
    return (unsigned char*)mapped_base + (address - pageAddress);
}



void MemoryAccess::unmapAddress(void* adjustedAddress, size_t busSize) const {
    size_t pageSize = getpagesize();
    off_t pageOffset = (off_t)adjustedAddress % pageSize;
    void* originalAddress = (unsigned char*)adjustedAddress - pageOffset; // Adjust back to original address

    size_t alignedSize = (busSize + (pageSize - 1)) & ~(pageSize - 1); // Align size to page size

    if (munmap(originalAddress, alignedSize) == -1) {
        perror("munmap");
    }
}

char* MemoryAccess::read_physical_mem(off_t target, size_t length) {

    char *mapped_base;
    off_t dev_base = target;
    size_t map_size = length;
    size_t map_mask = map_size - 1;

    mapped_base = (char *) mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, dev_base & ~map_mask);
    //mapped_base = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, dev_base & ~map_mask);
    //mapped_base = mmap((void*)dev_base, map_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (mapped_base == (void *) -1) {
        perror("Error mapping memory");
        close(mem_fd);

    }
    //printf("Send chunk data from: %h size: %d \n", dev_base , map_size);
    //printf("Memory mapped at address %p.\n", mapped_base);
    // Print all data in the memory chunk
    //unsigned char *ptr = (unsigned char *)mapped_base;
    /*for (size_t i = 0; i < map_size; i++) {
        printf("%02x ", ptr[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }*/
	        /*for (size_t i = 0; i < map_size; i++) {
	            ptr[i] = 0x55;
	        }*/

    // Unmap memory and close file descriptor
   /* if (munmap(mapped_base, map_size) == -1) {
        perror("Error un-mmapping the file");
    }
    close(fd);*/

    return mapped_base;
}

void MemoryAccess::safe_memcpy(uint8_t *dest, const uint8_t *src, size_t length) {
    for (size_t i = 0; i < length; i++) {
        dest[i] = src[i];
    }
}
/*
std::list<uint32_t> MemoryAccess::readBlockMemory(off_t address, size_t busSize ,uint32_t samples) const {
	std::list<uint32_t> samplesList; // Create a list of integers
	for (int i = 0 ; i < samples ; i++)
    {
    	void* mapped = mapAddress(address + (i*4), busSize);
    	samplesList.push_back(*((uint32_t*)mapped));
    	unmapAddress(mapped, busSize);
    }
	return samplesList;

}*/
