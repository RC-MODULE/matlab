#include <easynmc/easynmc.h>
#include <easynmc/aura-exportfile.h>




AURA_METHOD(aura_nmc_nmppsSum_s32_256, Sum_s32_256,  
	    BIN(1024), 
	    U32);

AURA_METHOD(aura_nmc_nmppsAbs_s32_256, Abs_s32_256, 
	    BIN(1024), 
	    BIN(1024));

AURA_METHOD(aura_nmc_nmppsAdd_s32_256, Add_s32_256, 
	    BIN(1024) BIN(1024), 
	    BIN(1024));


AURA_METHOD(aura_nmc_nmppsFwdFFT256, FwdFFT256, 
	    BIN(2048), 
	    BIN(2048));





struct aura_object g_aura_eof;
