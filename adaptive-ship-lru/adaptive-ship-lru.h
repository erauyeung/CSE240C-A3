#define NUM_CORE 1
#define LLC_SETS NUM_CORE*2048
#define LLC_WAYS 16

//#define DOUBLE_BUDGET

/*
For experiments of DIP-SD in which 64 sets are dedicated to each
policy, we use a 11-bit PSEL counter.
Choose 32*2->64 dedicated sets, 11-bit counter
0 & every 65th set to policy1
Set 63 & every 63rd set to policy2
*/
#define POLICY_FOLLOWER 0
#define POLICY_SHIP 1
#define POLICY_LRU 2
#define DUEL_OFFSET 64
#define SHIP_OFFSET (DUEL_OFFSET+1)
#define LRU_OFFSET (DUEL_OFFSET-1)

// From SHiP++
#define SAT_INC(x,max)  (x<max)?x+1:x
#define SAT_DEC(x)      (x>0)?x-1:x
#define TRUE 1
#define FALSE 0

uint32_t GetPolicyForSet(uint32_t set);
bool IsSetShip(uint32_t set);
bool IsSetLRU(uint32_t set);
