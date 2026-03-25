#include <windows.h>
#include <cstdint>
#include <cstdio>

typedef struct {
    uint8_t *pReserveStart; // a1[0]
    uint8_t *pUnknown;      // a1[1]
    uint8_t *pCommitEnd;    // a1[2]
    uint8_t *pCurrent;      // a1[3]
    size_t   pageSize;      // a1[4]
    size_t   reserveSize;   // a1[5]
} Mihoyo_Mem_Block;

void* __fastcall Mihoyo_Alloc(Mihoyo_Mem_Block *memBlock, const uint64_t count, const uint64_t sz) {
    const size_t alignment = count * sz + 7 & 0xFFFFFFFFFFFFFFF8uLL;
    uint8_t *pCommitEnd = memBlock->pCommitEnd;
    uint8_t *pCurrent = memBlock->pCurrent;
    const size_t pageSize = memBlock->pageSize;
    const size_t available = pCommitEnd - pCurrent - pageSize;

    uint8_t *pCurrentTemp = pCurrent;
    if (available < alignment) {
        const size_t neededCommitSize = -pageSize & alignment + pageSize + ~available;
        size_t totalCommitSize = pageSize * 4;
        if (totalCommitSize <= neededCommitSize) {
            totalCommitSize = neededCommitSize;
        }
        if (reinterpret_cast<uint64_t>(&pCommitEnd[totalCommitSize]) > *reinterpret_cast<uint64_t *>(memBlock) + memBlock->reserveSize) {
            abort();
        }

        uint8_t *pNewEndPointer = nullptr;
        if (VirtualAlloc(pCommitEnd, totalCommitSize, MEM_COMMIT, PAGE_READWRITE) == pCommitEnd) {
            pNewEndPointer = memBlock->pCommitEnd + totalCommitSize;
        }
        else {
            printf(
                "MemoryPool::InternalCommit failed: address %llX, size %zu, start %p, current %p, page size 0x%zx",
                *reinterpret_cast<uint64_t*>(memBlock),
                totalCommitSize,
                pCommitEnd,
                pCurrent,
                pageSize
            );
        }
        memBlock->pCommitEnd = pNewEndPointer;
        pCurrentTemp = memBlock->pCurrent;
    }
    memBlock->pCurrent = &pCurrentTemp[alignment];
    return memset(pCurrent,0, alignment);
}

bool InitMemoryBlock(Mihoyo_Mem_Block* block, const size_t reserveSize) {
    SYSTEM_INFO si{};
    GetSystemInfo(&si);

    block->pageSize = si.dwPageSize;
    block->reserveSize = reserveSize;

    block->pReserveStart = static_cast<uint8_t*>(VirtualAlloc(
        nullptr,
        reserveSize,
        MEM_RESERVE,
        PAGE_READWRITE
    ));

    if (!block->pReserveStart)
        return false;

    const size_t initialCommit = block->pageSize * 4;

    if (!VirtualAlloc(block->pReserveStart, initialCommit, MEM_COMMIT, PAGE_READWRITE))
        return false;

    block->pCommitEnd = block->pReserveStart + initialCommit;
    block->pCurrent   = block->pReserveStart;
    block->pUnknown   = nullptr;

    return true;
}

int main() {
    Mihoyo_Mem_Block pool{};

    if (!InitMemoryBlock(&pool, 1024 * 1024)) {
        printf("Init failed\n");
        return 1;
    }

    int* const arr = static_cast<int*>(Mihoyo_Alloc(&pool, 100, sizeof(int)));

    for (int i = 0; i < 100; i++)
        arr[i] = i;

    struct Vec3 { float x, y, z; };

    Vec3* const vec = static_cast<Vec3*>(Mihoyo_Alloc(&pool, 1024, sizeof(Vec3)));

    for (auto i = 0; i < 1024; i++) {
        vec[i].x = 1.0f;
        vec[i].y = 2.0f;
        vec[i].z = 3.0f;
        printf("vec = (%f, %f, %f)\n",  vec[i].x, vec[i].y, vec[i].z);
    }

    printf("arr[10] = %d\n", arr[10]);
}