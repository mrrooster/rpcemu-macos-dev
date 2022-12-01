#ifndef CODEGEN_ARM64_H
#define CODEGEN_ARM64_H

#include <stdint.h>

#define BLOCKS 1024
#define BLOCK_COUNT 1792
#define isblockvalid(l) (dcache)

#define BLOCK_ALLOC_SIZE (BLOCK_COUNT)
extern uint8_t *rcodeblock;
extern uint32_t codeblockpc[0x8000];
extern int codeblocknum[0x8000];

extern uint8_t flaglookup[16][16];

#define BLOCKSTART 36

#define SIGN_CONTRACT(val,bits) (((val & (1<<31)) >> (31-bits)) | val)

#define MASK_2(val) (val & 3)
#define MASK_5(val) (val & 0x1f)
#define MASK_6(val) (val & 0x3f)
#define MASK_7(val) (val & 0x7f)
#define MASK_9(val) (val & 0x1ff)
#define MASK_12(val) (val & 0xfff)
#define MASK_13(val) (val & 0x1fff)
#define MASK_14(val) (val & 0x3fff)
#define MASK_16(val) (val & 0xffff)
#define MASK_17(val) (val & 0x1FFFF)
#define MASK_18(val) (val & 0x3FFFF)
#define MASK_19(val) (val & 0x7FFFF)
#define MASK_26(val) (val & 0x3FFFFFF)

#define HASH(l) (((l) >> 2) & 0x7FFF)

#define REG_X0 0
#define REG_X1 1
#define REG_X2 2
#define REG_X3 3
#define REG_X4 4
#define REG_X5 5
#define REG_X6 6
#define REG_X7 7
#define REG_X8 8
#define REG_X9 9
#define REG_X10 10
#define REG_X11 11
#define REG_X12 12
#define REG_X13 13
#define REG_X14 14
#define REG_X15 15
#define REG_X16 16
#define REG_X17 17
#define REG_X18 18
#define REG_X19 19
#define REG_X20 20
#define REG_X21 21
#define REG_X22 22
#define REG_X23 23
#define REG_X24 24
#define REG_X25 25
#define REG_X26 26
#define REG_X27 27
#define REG_X28 28
#define REG_X29 29
#define REG_X30 30
#define REG_W0 0
#define REG_W1 1
#define REG_W2 2
#define REG_W3 3
#define REG_W4 4
#define REG_W5 5
#define REG_W6 6
#define REG_W7 7
#define REG_W8 8
#define REG_W9 9
#define REG_W10 10
#define REG_W11 11
#define REG_W12 12
#define REG_W13 13
#define REG_W14 14
#define REG_W15 15
#define REG_W16 16
#define REG_W17 17
#define REG_W18 18
#define REG_W19 19
#define REG_W20 20
#define REG_W21 21
#define REG_W22 22
#define REG_W23 23
#define REG_W24 24
#define REG_W25 25
#define REG_W26 26
#define REG_W27 27
#define REG_W28 28
#define REG_W29 29
#define REG_W30 30

#define REG_SP 31
#define REG_XZR 31
#define REG_IP0 REG_X16
#define REG_IP1 REG_X17
#define REG_LR REG_X30

// Temp registers used in code block
#define REG_ARM           REG_X19
#define REG_INSCOUNT      REG_X20
#define REG_LINECYC       REG_X21
#define REG_SCRATCH       REG_X22
#define REG_PCMASK        REG_X23
#define REG_PCPSR         REG_X24
#define REG_CG1           REG_X25
#define REG_CG2           REG_X26
#define REG_CG3           REG_X27
#define REG_CG4           REG_X28

#define REG_CODEBLOCKNUM  REG_X26 // Used in endblock only
#define REG_CODEBLOCKADDR REG_X27 // Used in endblock only
#define REG_CODEBLOCKPC   REG_X28 // Used in endblock only

#define ARM_REG_OFF(regno) ((void*)(&(arm.reg[regno]))-(void*)(&arm))

#define MRSSTATUS(reg) addlong( (0x354<<22) | MASK_5(reg) | MASK_17(0x1DA10) << 5 )
#define MSRSTATUS(reg) addlong( (0x354<<22) | MASK_5(reg) | MASK_17(0xDA10) << 5 )

//#define DP_ADR(imm, rd) addlong((OP1_DPIMM | (MASK_2(imm) << 29) | (MASK_18(imm >> 2) << 5) | (MASK_5(rd))))
//#define DP_ADRP(imm, rd) addlong((OP0_SET | DP_ADR(imm, rd)))

//#define DP_ADD_SUB_IMM addlong((OP1_DPIMM | (2 << 23)))
//#define ADD32(imm, rn, rd) addlong((DP_ADD_SUB_IMM | (MASK_12(imm) << 10) | (MASK_5(rn) << 5) | MASK_5(rd)))
#define ADD64(r1,r2,imm) addlong( (0x244<<22) | (MASK_12(imm)<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define ADD64SHL12(r1,r2,imm) addlong( (0x245<<22) | (MASK_12(imm)<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define ADD64SHIFT(r1,r2,r3,type,amt) addlong( (0x22c<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define ADD64REG(r1,r2,r3) ADD64SHIFT(r1,r2,r3,0,0) //addlong( (0x22c<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define ADD32SHIFT(r1,r2,r3,type,amt) addlong( (0x2c<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define ADD32REG(r1,r2,r3) ADD32SHIFT(r1,r2,r3,0,0) //addlong( (0x2c<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define ADDS32SHIFT(r1,r2,r3,type,amt) addlong( (0xac<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define ADDS32REG(r1,r2,r3) ADD32SHIFT(r1,r2,r3,0,0) // addlong( (0xac<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define ADD32(r1,r2,imm) addlong( (0x44<<22) | (MASK_12(imm)<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define SUB64(r1,r2,imm) addlong( (0x344<<22) | (MASK_12(imm)<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define SUBS64(r1,r2,imm) addlong( (0x3c4<<22) | (MASK_12(imm)<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define SUB32(r1,r2,imm) addlong( (0x144<<22) | (MASK_12(imm)<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define SUBS32(r1,r2,imm) addlong( (0x1c4<<22) | (MASK_12(imm)<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define SUB32SHIFT(r1,r2,r3,type,amt) addlong( (0x12c<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define SUB32REG(r1,r2,r3) SUB32SHIFT(r1,r2,r3,0,0) //addlong( (0x12c<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define SUBS32SHIFT(r1,r2,r3,type,amt) addlong( (0x1ac<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define SUBS32REG(r1,r2,r3) SUBS32SHIFT(r1,r2,r3,0,0) //addlong( (0x1ac<<22) | (MASK_5(r3)<<16) | (MASK_5(r2)<<5) | MASK_5(r1) )

#define LSL64(r1,r2,r3) addlong( (0x26b<<22) | (MASK_5(r3)<<16) | (1<<13) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LSL32(r1,r2,r3) addlong( (0x6b<<22) | (MASK_5(r3)<<16) | (1<<13) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LSR64(r1,r2,r3) addlong( (0x26b<<22) | (MASK_5(r3)<<16) | (0x9<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LSR32(r1,r2,r3) addlong( (0x6b<<22) | (MASK_5(r3)<<16) | (0x9<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LSL64IMM(r1,r2,imm) addlong( (0x34d<<22) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_6( (-imm)%64  )<<16) | (MASK_6(63-imm)<<10) )
#define LSL32IMM(r1,r2,imm) addlong( (0x14c<<22) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_6( 32-imm )<<16) | (MASK_6(31-imm)<<10) )
#define LSR64IMM(r1,r2,imm) addlong( (0x34d<<22) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_6( imm  )<<16) | (MASK_6(0x3f)<<10) )
#define LSR32IMM(r1,r2,imm) addlong( (0x14c<<22) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_6( imm  )<<16) | (MASK_6(0x1f)<<10) )

#define ASR64(r1,r2,r3) addlong( (0x26b<<22) | (MASK_5(r3)<<16) | (0xa<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define ASR32(r1,r2,r3) addlong( (0x6b<<22) | (MASK_5(r3)<<16) | (0xa<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define ASR32IMM(r1,r2,imm) addlong( (0x4c << 22) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_6( imm  )<<16) | (MASK_6(0x1f)<<10) )
#define ROR32IMM(r1,r2,imm) addlong( (0x4e << 22) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_6( imm  )<<10) | (MASK_5(r2)<<16) )
#define ROR32(r1,r2,r3) addlong( (0x6b << 22) | (MASK_5(r2)<<5) | MASK_5(r1) | (MASK_5( r3  )<<16) | (MASK_6(0xb)<<10) )

#define LDRP64OFF(r1,r2,r3,amt) addlong( (0x2a5<<22) | (MASK_7(SIGN_CONTRACT((amt/8),7)) << 15) | (MASK_5(r2) << 10) | (MASK_5(r3) << 5) | MASK_5(r1) ) // LDRP X0,X1,[X3],#amt
#define LDRP64POST(r1,r2,r3,amt) addlong( (0x2a3<<22) | (MASK_7(SIGN_CONTRACT((amt/8),7)) << 15) | (MASK_5(r2) << 10) | (MASK_5(r3) << 5) | MASK_5(r1) ) // LDRP X0,X1,[X3],#amt
#define LDR64POST(r1,r2,amt) addlong( (0x3e1<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_9(SIGN_CONTRACT(amt,9))<<12) | (1<<10) ) // LDR r1,[r2],#amt
#define LDR64IMMPOST(r1,r2,off) addlong( (0x3e1<<22) | (MASK_9(off)<<12) | (1<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDR32IMMPOST(r1,r2,off) addlong( (0x2e1<<22) | (MASK_9(off)<<12) | (1<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDR64(r1,r2,off) addlong( (0x3e5<<22) | (MASK_12((off/8))<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDR32(r1,r2,off) addlong( (0x2e5<<22) | (MASK_12((off/4))<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDRR64(r1,r2,r3) addlong( (0x3e1<<22) | (MASK_5(r3)<<16) | (1<<21) | (1<<11) | (3<<13) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDRR32(r1,r2,r3) addlong( (0x2e1<<22) | (MASK_5(r3)<<16) | (1<<21) | (1<<11) | (3<<13) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDRR8(r1,r2,r3) addlong( (0xe1<<22) | (1<<21) | (MASK_5(r3)<<16) | (1<<11) | (2<<13) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDRR8S(r1,r2,r3) addlong( (0xe3<<22) | (1<<21) | (MASK_5(r3)<<16) | (1<<11) | (2<<13) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define LDR8(r1,r2,off) addlong ( (0xe5<<22) | (MASK_12(off)<<10) | (MASK_5(r2)<<5) | MASK_5(r1)  )

#define STRP64OFF(r1,r2,r3,amt) addlong( (0x2a4<<22) | (MASK_7(SIGN_CONTRACT( (amt/8) ,7)) << 15) | (MASK_5(r2) << 10) | (MASK_5(r3) << 5) | MASK_5(r1) ) // STRP X0,X1,[X3,#amt]!
#define STRP64PRE(r1,r2,r3,amt) addlong( (0x2a6<<22) | (MASK_7(SIGN_CONTRACT( (amt/8) ,7)) << 15) | (MASK_5(r2) << 10) | (MASK_5(r3) << 5) | MASK_5(r1) ) // STRP X0,X1,[X3,#amt]!
#define STR64PRE(r1,r2,amt) addlong( (0x3e0<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_9(SIGN_CONTRACT(amt,9))<<12) | (3<<10) ) // STR r1,[r2,#amt]!
#define STR64IMMPOST(r1,r2,off) addlong( (0x3e0<<22) | (MASK_9(off)<<12) | (1<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define STR32IMMPOST(r1,r2,off) addlong( (0x2e0<<22) | (MASK_9(off)<<12) | (1<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define STR64(r1,r2,off) addlong( (0x3e4<<22) | (MASK_12((off/8))<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )
#define STR32(r1,r2,off) addlong( (0x2e4<<22) | (MASK_12((off/4))<<10) | (MASK_5(r2)<<5) | MASK_5(r1) )

#define AND64(r1,r2,r3) addlong( (0x228<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )
#define AND32SHIFT(r1,r2,r3,type,amt) addlong( (0x28<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define AND32(r1,r2,r3) AND32SHIFT(r1,r2,r3,0,0)  //addlong( (0x28<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )
#define ANDS64(r1,r2,r3) addlong( (0x3a8<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )
#define ANDS32SHIFT(r1,r2,r3,type,amt) addlong( (0x1a8<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define ANDS32(r1,r2,r3) ANDS32SHIFT(r1,r2,r3,0,0) //addlong( (0x1a8<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )
#define ORR64(r1,r2,imm) addlong( (0x2c8<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_14(imm)<<10) )
#define ORR32(r1,r2,imm) addlong( (0xc8<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_13(imm)<<10) )
#define ORRR64(r1,r2,r3) addlong( (0x2a8<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )
#define ORRR32(r1,r2,r3) addlong( (0xa8<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )
#define EOR64(r1,r2,r3) addlong( (0x328<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )
#define EOR32SHIFT(r1,r2,r3,type,amt) addlong( (0x128<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) | (MASK_2(type)<<22) | (MASK_6(amt)<<10) )
#define EOR32(r1,r2,r3) EOR32SHIFT(r1,r2,r3,type,amt)  //addlong( (0x128<<22) | MASK_5(r1) | (MASK_5(r2)<<5) | (MASK_5(r3)<<16) )

//#define MOV64(r1,imm) ORR64(r1,REG_XZR,imm)
//#define MOV32(r1,imm) ORR32(r1,REG_XZR,imm)
#define MVN32(r1,r2) addlong( (0xa8<<22) |  MASK_5(r1) | (MASK_5(r2)<<16) | (MASK_5(0x1f)<<5) | (1<<21))
#define MOVZ32(r1,imm) addlong( (0x14a<<22) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVZ32SHL16(r1,imm) addlong( (0x14a<<22) | (1<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVZ64(r1,imm) addlong( (0x34a<<22) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVZ64SHL32(r1,imm) addlong( (0x34a<<22) | (2<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVZ64SHL48(r1,imm) addlong( (0x34a<<22) | (3<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVK64(r1,imm) addlong( (0x3ca<<22) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVK64SHL16(r1,imm) addlong( (0x3ca<<22) | (1<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVK64SHL32(r1,imm) addlong( (0x3ca<<22) | (2<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVK64SHL48(r1,imm) addlong( (0x3ca<<22) | (3<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVZK64(keep,r1,imm) addlong( (0x34a<<22) | (keep<<29) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVZK64SHL16(keep,r1,imm) addlong( (0x34a<<22) | (keep<<29) | (1<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )
#define MOVZK64SHL32(keep,r1,imm) addlong( (0x34a<<22) | (keep<<29) | (2<<21) | MASK_5(r1) | (MASK_16(imm)<<5) )

#define CMPR64(r1,r2) addlong( (0x3ac<<22) | MASK_5(REG_XZR) | (MASK_5(r1)<<5) | (MASK_5(r2)<<16)  );
#define CMP64(r1,val) addlong( (0x3c4<<22) | (MASK_12(val)<<10) | (MASK_5(r1)<<5) | MASK_5(REG_XZR) )
#define CMPR32(r1,r2) addlong( (0x1ac<<22) | MASK_5(REG_XZR) | (MASK_5(r1)<<5) | (MASK_5(r2)<<16)  );
#define CMP32(r1,val) addlong( (0x1c4<<22) | (MASK_12(val)<<10) | (MASK_5(r1)<<5) | MASK_5(REG_XZR) )
// See here for 'val' https://gist.github.com/dinfuehr/51a01ac58c0b23e4de9aac313ed6a06a
#define TST64(r1,n,immr,imms) addlong( (0x3c8<<22) | (MASK_13( MASK_6(imms) | (MASK_6(immr)<<6) | ((n&1)<<12)  )<<10) | (MASK_5(r1)<<5) | MASK_5(REG_XZR) )
#define TST32(r1,n,immr,imms) addlong( (0x1c8<<22) | (MASK_12( MASK_6(imms) | (MASK_6(immr)<<6))<<10) | (MASK_5(r1)<<5) | MASK_5(REG_XZR) )

#define B(off) addlong( (0x50<<22) | MASK_26(SIGN_CONTRACT((off/4),26)) )
#define BL(off) addlong( (0x250<<22) | MASK_26(SIGN_CONTRACT((off/4),26)) )
#define BRL(r1) addlong( (0x358<<22) | (0x3f << 16) | (MASK_5(r1)<<5) )
#define BR(r1) addlong( (0x358<<22) | (0x1f << 16) | (MASK_5(r1)<<5) )

#define COND_EQ 0x0
#define COND_NE 0x1
#define COND_CS 0x2
#define COND_CC 0x3
#define COND_MI 0x4
#define COND_PL 0x5
#define COND_VS 0x6
#define COND_VC 0x7
#define COND_HI 0x8
#define COND_LS 0x9
#define COND_GE 0xa
#define COND_LT 0xb
#define COND_GT 0xc
#define COND_LE 0xd
#define COND_AL 0xe
#define COND_ALWAYS COND_AL

#define BCOND(off,cond) addlong( (0x150<<22) | (MASK_19(SIGN_CONTRACT((off/4),19))<<5) | (cond) )
#define BEQ(off) BCOND(off,COND_EQ)
#define BNE(off) BCOND(off,COND_NE)
#define BCS(off) BCOND(off,COND_CS)
#define BCC(off) BCOND(off,COND_CC)
#define BMI(off) BCOND(off,COND_MI)
#define BPL(off) BCOND(off,COND_PL)
#define BVS(off) BCOND(off,COND_VS)
#define BVC(off) BCOND(off,COND_VC)
#define BHI(off) BCOND(off,COND_HI)
#define BLS(off) BCOND(off,COND_LS)
#define BGE(off) BCOND(off,COND_GE)
#define BLT(off) BCOND(off,COND_LT)
#define BGT(off) BCOND(off,COND_GT)
#define BLE(off) BCOND(off,COND_LE)
#define BAL(off) BCOND(off,COND_AL)


#define RET addlong( 0xd65f0000 | (MASK_5(30) << 5) )

#define stack_x0_x18() STRP64PRE(REG_X0,REG_X1,REG_SP,-16);STRP64PRE(REG_X2,REG_X3,REG_SP,-16);STRP64PRE(REG_X4,REG_X5,REG_SP,-16);STRP64PRE(REG_X6,REG_X7,REG_SP,-16);STRP64PRE(REG_X8,REG_X9,REG_SP,-16);STRP64PRE(REG_X10,REG_X11,REG_SP,-16);STRP64PRE(REG_X12,REG_X13,REG_SP,-16);STRP64PRE(REG_X14,REG_X15,REG_SP,-16);STRP64PRE(REG_X16,REG_X17,REG_SP,-16);STR64PRE(REG_X18,REG_SP,-16);
#define pop_x0_x18() LDR64POST(REG_X18,REG_SP,16);LDRP64POST(REG_X16,REG_X17,REG_SP,16);LDRP64POST(REG_X14,REG_X15,REG_SP,16);LDRP64POST(REG_X12,REG_X13,REG_SP,16);LDRP64POST(REG_X10,REG_X11,REG_SP,16);LDRP64POST(REG_X8,REG_X9,REG_SP,16);LDRP64POST(REG_X6,REG_X7,REG_SP,16);LDRP64POST(REG_X4,REG_X5,REG_SP,16);LDRP64POST(REG_X2,REG_X3,REG_SP,16);LDRP64POST(REG_X0,REG_X1,REG_SP,16);


#endif // CODEGEN_ARM64_H
