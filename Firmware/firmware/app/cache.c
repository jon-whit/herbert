/////////////////////////////////////////////////////////////
//
//  cache.c
//
//  Processor cache interface
//

#include <cache.h>

#include <xparameters.h>
#include <mb_interface.h>

void cacheInit()
{
    microblaze_disable_icache();
    microblaze_init_icache_range(XPAR_MICROBLAZE_ICACHE_BASEADDR, XPAR_MICROBLAZE_0_CACHE_BYTE_SIZE);
    microblaze_enable_icache();

    microblaze_disable_dcache();
    microblaze_init_dcache_range(XPAR_MICROBLAZE_DCACHE_BASEADDR, XPAR_MICROBLAZE_0_DCACHE_BYTE_SIZE);
    microblaze_enable_dcache();
}

// EOF
