#include <easynmc/easynmc.h>
#include <easynmc/aura-exportfile.h>


AURA_METHOD(fft_u32, fft32, 
	    U32, 
	    U32);

AURA_METHOD(fft_u64, fft64, 
	    U64, 
	    U64);

AURA_METHOD(fft_bin, fftbin, 
	    BIN(64), 
	    BIN(64));

AURA_METHOD(aura_nmc_nmppsSum_s32_256, Sum_s32_256,  
	    BIN(1024), 
	    U32);

AURA_METHOD(aura_nmc_nmppsAbs_s32_256, Abs_s32_256, 
	    BIN(1024), 
	    BIN(1024));

AURA_METHOD(aura_nmc_nmppsAdd_s32_256, Add_s32_256, 
	    BIN(1024) BIN(1024), 
	    BIN(1024));


AURA_METHOD(fft_bin2, fftbin2, 
	    BIN(128), 
	    U32);

AURA_METHOD(fft_bin222, fftbin222, 
	    BIN(128), 
	    U32);

AURA_METHOD(fft_bin222_bullshit, qqq_fft, 
	    BIN(128), 
	    U32);


//AURA_METHOD(fft_bin2_bullshit, qqq, 
//	    BIN(128), 
//	    U32);




AURA_METHOD(fft_u32u32, fftu32u32, 
	    U32 U32, 
	    U32 U32);

AURA_METHOD(sum_binu32, sum_binu32, 
	    BIN(128), 
	    BIN(128));



struct aura_object g_aura_eof;
