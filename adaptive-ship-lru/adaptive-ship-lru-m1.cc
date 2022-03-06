////////////////////////////////////////////
//                                        //
//        LRU replacement policy          //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////

#include "../inc/champsim_crc2.h"
#include "adaptive-ship-lru.h"
#include "adaptive-ship++.cc"
#include "adaptive-lru.cc"

#define PSEL_SIZE 11
#define PSEL_MAX (1<<PSEL_SIZE)-1
#define PSEL_THRESHOLD (1<<(PSEL_SIZE-1))
#define PSEL_INIT PSEL_THRESHOLD

uint32_t psel;

int shipcount;
int lrucount;

uint32_t adaptive[LLC_SETS][LLC_WAYS];

#define NUM_LEADERS_EACH 32
// Get policy for our set:
// 0 = follower
// 1 = ship++
// 2 = LRU
uint32_t GetPolicyForSet(uint32_t set)
{
    if (IsSetShip(set)) {
        return POLICY_SHIP;
    }
    else if (IsSetLRU(set)) {
        return POLICY_LRU;
    } else {
        return POLICY_FOLLOWER;
    }
}

// TODO: Naively choose leaders
bool IsSetShip(uint32_t set) {
    return set < NUM_LEADERS_EACH;
}
bool IsSetLRU(uint32_t set) {
    return set >= NUM_LEADERS_EACH && set < (2 * NUM_LEADERS_EACH);
}

// initialize replacement state
void InitReplacementState()
{
    cout << "Initialize Adaptive replacement state" << endl;

    InitReplacementStateShip();
    InitReplacementStateLRU();

    // Set up counter
    psel = PSEL_INIT;
    cout << "Max psel is " << PSEL_MAX << "; threshold is " << PSEL_THRESHOLD << endl;
    cout << "Initialize psel to " << psel << endl;
    shipcount = 0;
    lrucount = 0;
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSet (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    int victim = 0;
    uint32_t policy = GetPolicyForSet(set);
    if (policy == POLICY_SHIP) {
        // Use SHiP++
        victim = GetVictimInSetShip(cpu, set, current_set, PC, paddr, type);
    } else if (policy == POLICY_LRU) {
        // Use LRU
        victim = GetVictimInSetLRU(cpu, set, current_set, PC, paddr, type);
    } else {
        // Based on PSEL value, do *not* update PSEL
        if (psel >= PSEL_THRESHOLD) {
            // Use SHiP++
            victim = GetVictimInSetShip(cpu, set, current_set, PC, paddr, type);
        } else {
            // Use MPPPB
            victim = GetVictimInSetLRU(cpu, set, current_set, PC, paddr, type);
        }
    }
    return victim;
}

// called on every cache hit and cache fill
void UpdateReplacementState (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    uint32_t policy = GetPolicyForSet(set);
    if (policy == POLICY_SHIP) {
        // Used SHiP++ and update PSEL
        UpdateReplacementStateShip(cpu, set, way, paddr, PC, victim_addr, type, hit);
        if (!hit) {
            psel = SAT_INC(psel, PSEL_MAX);
	    shipcount++;
        }
    } else if (policy == POLICY_LRU) {
        // Used LRU and update PSEL
        UpdateReplacementStateLRU(cpu, set, way, paddr, PC, victim_addr, type, hit);
        if (!hit) {
            psel = SAT_DEC(psel);
	    lrucount++;
        }
    }
    // Do not update replacement state when using a follower set.
    // The follower set applies the policy, but does not train it.
}

// use this function to print out your own stats on every heartbeat 
void PrintStats_Heartbeat()
{
    cout << "PSEL: " << psel << endl;
    PrintStats_HeartbeatShip();
    PrintStats_HeartbeatLRU();
}

// use this function to print out your own stats at the end of simulation
void PrintStats()
{
    cout << "Final PSEL: " << psel << "\tSHiP usage: " << shipcount << "\tLRU usage: " << lrucount << endl;
    PrintStatsShip();
    PrintStatsLRU();
}
