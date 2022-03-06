////////////////////////////////////////////
//                                        //
//     SRRIP [Jaleel et al. ISCA' 10]     //
//     Jinchun Kim, cienlux@tamu.edu      //
//                                        //
////////////////////////////////////////////
//
#include "../inc/champsim_crc2.h"
#include "adaptive-ship-lru.h"

//#define MAX_LLC_SETS 8192
#define RRIP_OVERRIDE_PERC   0

#ifdef DOUBLE_BUDGET
#define DOUBLE_BUDGET_SHIP
#endif

// The base policy is SRRIP. SHIP needs the following on a per-line basis
// TODO reduce RRPV to save 4KB by shaving off 1 bit * sets * ways
#ifdef DOUBLE_BUDGET_SHIP
#define maxRRPV 3
#else
#define maxRRPV 1
#endif
uint32_t line_rrpv[LLC_SETS][LLC_WAYS];
uint32_t is_prefetch[LLC_SETS][LLC_WAYS];
uint32_t fill_core[LLC_SETS][LLC_WAYS];

// These two are only for sampled sets (we use 64 sets)
// Adaptive: The original used 64 sampled sets for LLC_SETS=2048
// This time, let the sampled sets be exactly those that aren't
// followers for adapative
// (i.e. for SHiP++, our leader sets are 0 & every 65th)
//#define NUM_LEADER_SETS   64

// TODO no longer need ship_sample, we know exactly what sets
// Removes LLC_SETS amount of space
//uint32_t ship_sample[LLC_SETS];
uint32_t line_reuse[LLC_SETS][LLC_WAYS];
uint64_t line_sig[LLC_SETS][LLC_WAYS];
	
// SHCT. Signature History Counter Table
// per-core 16K entry. 14-bit signature = 16k entry. 3-bit per entry
// TODO set up 12-bit sig instead to halve size
// 12-bit signature = 4k entry, 3-bit per entry
#define maxSHCTR 7

#ifdef DOUBLE_BUDGET_SHIP
#define SHCT_SIGSIZE 14
#else
#define SHCT_SIGSIZE 12
#endif

#define SHCT_SIZE (1<<SHCT_SIGSIZE)
uint32_t SHCT[NUM_CORE][SHCT_SIZE];


// Statistics
uint64_t insertion_distrib[NUM_TYPES][maxRRPV+1];
uint64_t total_prefetch_downgrades;

// initialize replacement state
//void InitReplacementState()
void InitReplacementStateShip()
{
    //int LLC_SETS = (get_config_number() <= 2) ? 2048 : LLC_SETS;

    cout << "Initialize SRRIP state" << endl;
    cout << "Config #: " << get_config_number() << endl; 

    for (int i=0; i<LLC_SETS; i++) {
        for (int j=0; j<LLC_WAYS; j++) {
            line_rrpv[i][j] = maxRRPV; // 2 bits [0,3]
            line_reuse[i][j] = FALSE;  // 1 bit
            is_prefetch[i][j] = FALSE; // 1 big
            line_sig[i][j] = 0;
        }
    }

    for (int i=0; i<NUM_CORE; i++) {
        for (int j=0; j<SHCT_SIZE; j++) {
            SHCT[i][j] = 1; // Assume weakly re-use start
        }
    }

    // TODO no longer need ship_sample, we know exactly what sets
    // Consider leaders to be basically everything assigned to us and not a follower
    // 0, 65, 130, 195, etc
    /*
    for (int i = 0; i < numSets; i+=SHIP_OFFSET) {
        ship_sample[i] = 1;
    }
    */

    // Don't randomly select leaders anymore!
    /*
    int leaders=0;
    while(leaders<NUM_LEADER_SETS){
        int randval = rand()%numSets;
        
        if(ship_sample[randval]==0){
            ship_sample[randval]=1;
            leaders++;
        }
    }
    */
}

// find replacement victim
// return value should be 0 ~ 15 or 16 (bypass)
uint32_t GetVictimInSetShip (uint32_t cpu, uint32_t set, const BLOCK *current_set, uint64_t PC, uint64_t paddr, uint32_t type)
{
    // look for the maxRRPV line
    while (TRUE)
    {
        for (int i=0; i<LLC_WAYS; i++)
            if (line_rrpv[set][i] == maxRRPV) { // found victim
                return i;
            }

        for (int i=0; i<LLC_WAYS; i++)
            line_rrpv[set][i]++;
    }

    // WE SHOULD NOT REACH HERE
    assert(0);
    return 0;
}

// called on every cache hit and cache fill
void UpdateReplacementStateShip (uint32_t cpu, uint32_t set, uint32_t way, uint64_t paddr, uint64_t PC, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
// TODO
  assert(cpu == 0);
  uint32_t sig   = line_sig[set][way];

    if (hit) { // update to REREF on hit
        if( type != WRITEBACK ) 
        {

            if( (type == PREFETCH) && is_prefetch[set][way] )
            {
//                line_rrpv[set][way] = 0;
               
	        // TODO no longer need ship_sample, we know exactly what sets
                //if( (ship_sample[set] == 1) && ((rand()%100 <5) || (get_config_number()==4))) 
                if( IsSetShip(set) && ((rand()%100 <5) || (get_config_number()==4))) 
                {
                    uint32_t fill_cpu = fill_core[set][way];

                    SHCT[fill_cpu][sig] = SAT_INC(SHCT[fill_cpu][sig], maxSHCTR);
                    line_reuse[set][way] = TRUE;
                }
            }
            else 
            {
                line_rrpv[set][way] = 0;

                if( is_prefetch[set][way] )
                {
                    line_rrpv[set][way] = maxRRPV;
                    is_prefetch[set][way] = FALSE;
                    total_prefetch_downgrades++;
                }

	        // TODO no longer need ship_sample, we know exactly what sets
                //if( (ship_sample[set] == 1) && (line_reuse[set][way]==0) ) 
                if( IsSetShip(set) && (line_reuse[set][way]==0) ) 
                {
                    uint32_t fill_cpu = fill_core[set][way];

                    SHCT[fill_cpu][sig] = SAT_INC(SHCT[fill_cpu][sig], maxSHCTR);
                    line_reuse[set][way] = TRUE;
                }
            }
        }
        
	return;
    }
    
    //--- All of the below is done only on misses -------
    // remember signature of what is being inserted
    uint64_t use_PC = (type == PREFETCH ) ? ((PC << 1) + 1) : (PC<<1);
    uint32_t new_sig = use_PC%SHCT_SIZE;
    
    // TODO no longer need ship_sample, we know exactly what sets
    //if( ship_sample[set] == 1 ) 
    if( IsSetShip(set) ) 
    {
        uint32_t fill_cpu = fill_core[set][way];
        
        // update signature based on what is getting evicted
        if (line_reuse[set][way] == FALSE) { 
            SHCT[fill_cpu][sig] = SAT_DEC(SHCT[fill_cpu][sig]);
        }
        else 
        {
            SHCT[fill_cpu][sig] = SAT_INC(SHCT[fill_cpu][sig], maxSHCTR);
        }

        line_reuse[set][way] = FALSE;
        line_sig[set][way]   = new_sig;  
        fill_core[set][way]  = cpu;
    }



    is_prefetch[set][way] = (type == PREFETCH);

    // Now determine the insertion prediciton

    uint32_t priority_RRPV = maxRRPV-1 ; // default SHIP

    if( type == WRITEBACK )
    {
        line_rrpv[set][way] = maxRRPV;
    }
    else if (SHCT[cpu][new_sig] == 0) {
      line_rrpv[set][way] = (rand()%100>=RRIP_OVERRIDE_PERC)?  maxRRPV: priority_RRPV; //LowPriorityInstallMostly
    }
    else if (SHCT[cpu][new_sig] == 7) {
        line_rrpv[set][way] = (type == PREFETCH) ? 1 : 0; // HighPriority Install
    }
    else {
        line_rrpv[set][way] = priority_RRPV; // HighPriority Install 
    }

    // Stat tracking for what insertion it was at
    insertion_distrib[type][line_rrpv[set][way]]++;

}

// use this function to print out your own stats on every heartbeat 
void PrintStats_HeartbeatShip()
{
}

string names[] = 
{
    "LOAD", "RFO", "PREF", "WRITEBACK" 
};

// use this function to print out your own stats at the end of simulation
void PrintStatsShip()
{
    cout<<"Insertion Distribution: "<<endl;
    for(uint32_t i=0; i<NUM_TYPES; i++) 
    {
        cout<<"\t"<<names[i]<<" ";
        for(uint32_t v=0; v<maxRRPV+1; v++) 
        {
            cout<<insertion_distrib[i][v]<<" ";
        }
        cout<<endl;
    }

    cout<<"Total Prefetch Downgrades: "<<total_prefetch_downgrades<<endl;
    
}

