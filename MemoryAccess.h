#ifndef MEMORYACCESS_H
#define MEMORYACCESS_H

#include <cstdint>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <list>
class MemoryAccess {
public:
	MemoryAccess();
    ~MemoryAccess();

    uint32_t readMemory(off_t address, size_t busSize) const;
    char *read_physical_mem(off_t target, size_t length);
    //std::list<uint32_t> readBlockMemory(off_t address, size_t busSize ,uint32_t samples ) const;
    void writeMemory(off_t address, size_t busSize, uint32_t value) const;

private:
    int mem_fd;
    void safe_memcpy(uint8_t *dest, const uint8_t *src, size_t length) ;

    void* mapAddress(off_t address, size_t busSize) const;
    void unmapAddress(void* mappedAddress, size_t busSize) const;
};

#endif // MEMORYACCESS_H
