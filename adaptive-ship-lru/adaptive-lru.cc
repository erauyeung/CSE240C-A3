////////////////////////////////////////////
//                                        //
//        LRU replacement policy          //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"
#include "adaptive-ship-lru.h"

#ifdef DOUBLE_BUDGET
#define DOUBLE_BUDGET_LRU
#endif

// Less history available, to halve the budget
// from 16KB to 8KB
#ifdef DOUBLE_BUDGET_LRU
#define LRU_THRESHOLD LLC_WAYS
#else
#define LRU_THRESHOLD (LLC_WAYS/4)
#endif

uint32_t lru[LLC_SETS][LLC_WAYS];

// initialize replacement state
void InitReplacementStateLRU()
{
    cout << "Initialize LRU replacement state" << endl;

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            lru[i][j] = j;
        }
    }
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSetLRU (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    for (int i=0; i<LLC_WAYS; i++)
        //if (lru[set][i] == (LLC_WAYS-1))
        if (lru[set][i] == (LRU_THRESHOLD-1))
            return i;

    return 0;
}

// called on every cache hit and cache fill
void UpdateReplacementStateLRU (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    // update lru replacement state
    for (uint32_t i=0; i<LLC_WAYS; i++) {
        if (lru[set][i] < lru[set][way]) {
            lru[set][i]++;

            //if (lru[set][i] == LLC_WAYS)
            if (lru[set][i] == LRU_THRESHOLD)
                assert(0);
        }
    }
    lru[set][way] = 0; // promote to the MRU position
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_HeartbeatLRU()
{

}

// use this function to print out your own stats at the end of simulation
void PrintStatsLRU()
{

}
