#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "rpcemu.h"
#include "arm.h"
#include "arm_common.h"
#include "codegen_arm64.h"
#include "mem.h"
#ifdef __APPLE__
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <libkern/OSCacheControl.h>
#endif


///DEBUG
#include <unistd.h>
#include <fcntl.h>
static int haverecompiled = 0;
///DEBUG

#define isblockvalid(l) (dcache)
#ifdef __APPLE__
#ifdef __arm64__
//#define __REG_FILE__ // Disables saving registers every instruction.
uint8_t *rcodeblock;
#else
uint8_t rcodeblock[BLOCKS][BLOCK_COUNT] __attribute__ ((aligned (4096)));
#endif
#else
uint8_t rcodeblock[BLOCKS][BLOCK_COUNT] __attribute__ ((aligned (4096)));
#endif

int lastflagchange=0;

static const void *codeblockaddr[BLOCKS];
uint32_t codeblockpc[0x8000];
int codeblocknum[0x8000];
static uint8_t codeblockpresent[0x10000];
static int codeblockpos;
static int blockpoint, blockpoint2;
#ifdef __APPLE__
static int rcodeinit=0;
#endif
static uint32_t blocks[BLOCKS];
static uint32_t *lastpcpsr=0;
static int blocknum;
static int tempinscount;
static int codeblockstartjumppos=0;
static uint32_t currentblockpc;
static int block_enter;
static int lastrecompiled;
static int pcinc;

static const int canrecompile[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 10
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 30

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 50
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 70

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 80
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 90
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // a0
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // b0

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // c0
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // d0
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // e0
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // f0
};

static int generate_forward_jump(int condition);
static void generate_forward_jump_here(int jump_offset_pos);
static int generate_function_call(void *addr);
static void generate_load_addr_into_reg(short reg, void *addr);

static void generate_br(int block_offset,int with_link);
static void generate_bne(int block_offset);
static void generate_beq(int block_offset);
static void generate_blt(int block_offset);
static void generate_bmi(int block_offset);

static int recompile(uint32_t opcode, uint32_t *pcpsr);

#define gen_load_reg(arm64,arm) LDR32(arm64,REG_ARM,ARM_REG_OFF(arm))
#define gen_save_reg(arm64,arm) STR32(arm64,REG_ARM,ARM_REG_OFF(arm))
#define gen_load_reg_with_r15_mask(arm64,arm) gen_load_reg(arm64,arm);if (arm == 15) {AND32(arm64,arm64,REG_PCMASK);}

static inline void
addlong(uint32_t a)
{
#ifdef __APPLE__
    assert( ((BLOCK_ALLOC_SIZE*blockpoint2)+codeblockpos) < (BLOCK_ALLOC_SIZE * BLOCKS));
    memcpy(rcodeblock+(BLOCK_ALLOC_SIZE*blockpoint2)+codeblockpos,&a,sizeof(uint32_t));
#else
    memcpy(&rcodeblock[blockpoint2][codeblockpos], &a, sizeof(uint32_t));
#endif
//    printf("[0x%llx] - 0x%llx\n",(void*)rcodeblock+(BLOCK_ALLOC_SIZE*blockpoint2)+codeblockpos,a);
    codeblockpos += 4;
}

static void setflags_logical()
{
    MRSSTATUS(REG_IP1);
    MSRSTATUS(REG_SCRATCH);
    MOVZ32SHL16(REG_SCRATCH,0xf000);
    AND32(REG_IP1,REG_SCRATCH,REG_IP1);
    MVN32(REG_SCRATCH,REG_SCRATCH);
    LDR32(REG_IP0,REG_PCPSR,0);
    AND32(REG_IP0,REG_IP0,REG_SCRATCH);
    ORRR32(REG_IP1,REG_IP0,REG_IP1);
    STR32(REG_IP1,REG_PCPSR,0);
}

static void getflags()
{
    MRSSTATUS(REG_SCRATCH);
    LDR32(REG_IP0,REG_PCPSR,0);
    MSRSTATUS(REG_IP0);
}

void
initcodeblocks(void)
{
    int c;

#ifdef __APPLE__
    if (rcodeinit==0) {
        // Alloc memory with the correct permissions to allow JIT on M1
        rcodeblock=mmap(0,BLOCK_ALLOC_SIZE * BLOCKS,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANON|MAP_JIT,-1,0);
        rcodeinit=1;
    }
#endif
    // Clear all blocks
    memset(codeblockpc, 0xff, sizeof(codeblockpc));
    memset(blocks, 0xff, sizeof(blocks));

    for (c = 0; c < BLOCKS; c++) {
#ifdef __APPLE__
        codeblockaddr[c] = rcodeblock + (c*BLOCK_ALLOC_SIZE);
#else
        codeblockaddr[c] = &rcodeblock[c][0];
#endif
    }
}

void
resetcodeblocks(void)
{
    int c;

    blockpoint = 0;

    for (c = 0; c < BLOCKS; c++) {
        if (blocks[c] != 0xffffffff) {
            codeblockpc[blocks[c] & 0x7fff] = 0xffffffff;
            codeblocknum[blocks[c] & 0x7fff] = 0xffffffff;
            blocks[c] = 0xffffffff;
        }
    }
}

void
cacheclearpage(uint32_t a)
{
    int c, d;

    if (!codeblockpresent[a & 0xffff]) {
        return;
    }
    codeblockpresent[a & 0xffff] = 0;
    // a >>= 10;
    d = HASH(a << 12);
    for (c = 0; c < 0x400; c++) {
        if ((codeblockpc[c + d] >> 12) == a) {
            codeblockpc[c + d] = 0xffffffff;
        }
    }
}

void
initcodeblock(uint32_t l)
{
    lastpcpsr = 0;
    codeblockpresent[(l >> 12) & 0xffff] = 1;
    tempinscount = 0;
    // rpclog("Initcodeblock %08x\n", l);
    blockpoint++;
    blockpoint &= (BLOCKS - 1);
    if (blocks[blockpoint] != 0xffffffff) {
        // rpclog("Chucking out block %08x %d %03x\n", blocks[blockpoint], blocks[blockpoint] >> 24, blocks[blockpoint] & 0xfff);
        codeblockpc[blocks[blockpoint] & 0x7fff] = 0xffffffff;
        codeblocknum[blocks[blockpoint] & 0x7fff] = 0xffffffff;
    }
    blocknum = HASH(l);

    codeblockpos = 0;
    codeblockpc[blocknum] = l;
    codeblocknum[blocknum] = blockpoint;
    blocks[blockpoint] = blocknum;
    blockpoint2 = blockpoint;
#ifdef __APPLE__
    pthread_jit_write_protect_np(0);
#endif

    LDRP64OFF(REG_X19,REG_X20,REG_SP,0x10);
    LDRP64OFF(REG_X21,REG_X22,REG_SP,0x20);
    LDRP64OFF(REG_X23,REG_X24,REG_SP,0x30);
    LDRP64OFF(REG_X25,REG_X26,REG_SP,0x40);
    LDRP64OFF(REG_X27,REG_X28,REG_SP,0x50);
    LDRP64OFF(REG_X29,REG_LR,REG_SP,0x60);
    ADD64(REG_SP,REG_SP,0x70);
    RET;
    codeblockstartjumppos=generate_forward_jump(COND_ALWAYS);

    // Block Prologue
    assert(codeblockpos <= BLOCKSTART);
    codeblockpos = BLOCKSTART;
    SUB64(REG_SP,REG_SP,0x70); // we store R19-R30
    STRP64OFF(REG_X29,REG_LR,REG_SP,0x60);
    SUB64(REG_X29,REG_SP,0x60);
    STRP64OFF(REG_X27,REG_X28,REG_SP,0x50);
    STRP64OFF(REG_X25,REG_X26,REG_SP,0x40);
    STRP64OFF(REG_X23,REG_X24,REG_SP,0x30);
    STRP64OFF(REG_X21,REG_X22,REG_SP,0x20);
    STRP64OFF(REG_X19,REG_X20,REG_SP,0x10);

//    printf("Generated function starts at: 0x%llx\n",rcodeblock+(BLOCK_ALLOC_SIZE*blockpoint2)+codeblockpos);
    block_enter = codeblockpos;

    generate_load_addr_into_reg(REG_ARM          ,&arm);
    generate_load_addr_into_reg(REG_INSCOUNT     ,&inscount);
    generate_load_addr_into_reg(REG_LINECYC      ,&linecyc);
    generate_load_addr_into_reg(REG_PCMASK       ,(void*)arm.r15_mask);

     currentblockpc = arm.reg[15] & arm.r15_mask;
//	currentblockpc2 = PC;
}

void generatecall(OpFn addr, uint32_t opcode, uint32_t *pcpsr)
{
    if (pcpsr!=lastpcpsr) {
        generate_load_addr_into_reg(REG_PCPSR        ,pcpsr);
        lastpcpsr = pcpsr;
    }
    const int rewind_marker = codeblockpos;
    //    printf("Generate call: opc=0x%x, pcpsr=0x%x\n",opcode,*pcpsr);
    lastrecompiled = 0;
    if (canrecompile[(opcode >> 20) & 0xff]) {
        if (recompile(opcode, pcpsr)) {
            //printf("Recompile code end at 0x%llx\n",(rcodeblock+(BLOCK_ALLOC_SIZE*blockpoint2)+codeblockpos));
            return;
        }
    }
    codeblockpos=rewind_marker;
    generate_load_addr_into_reg(REG_X0,(void*)opcode);
    generate_function_call(addr);

    if (!flaglookup[opcode >> 28][(*pcpsr) >> 28] && (opcode & 0xe000000) == 0xa000000) {
        // rpclog("Carrying on - %d\n", pcinc);
        if (pcinc != 0) {
            gen_load_reg(REG_IP0,15);
            ADD32(REG_IP0,REG_IP0,pcinc);
            gen_save_reg(REG_IP0,15);
            //pcinc = 0;
        }
        generate_br(codeblockstartjumppos,0);
    }
    if (lastflagchange != 0) {
        generate_forward_jump_here(lastflagchange);
        lastflagchange=0;
    }
}

void
endblock(uint32_t opcode)
{
//	NOT_USED(opcode);
    generateupdatepc();
    generateupdateinscount();

    generate_forward_jump_here(codeblockstartjumppos);

//  REG_X19,&arm);
//  REG_X20,&inscount);
//  REG_X21,&linecyc);
//  REG_X22,codeblocknum);
//  REG_X23,codeblockaddr);
//  REG_X24,codeblockpc);
    generate_load_addr_into_reg(REG_CODEBLOCKNUM ,codeblocknum);
    generate_load_addr_into_reg(REG_CODEBLOCKADDR,codeblockaddr);
    generate_load_addr_into_reg(REG_CODEBLOCKPC  ,codeblockpc);

    // linecyc-=1;
    LDR32(REG_IP0,REG_LINECYC,0);
    SUB32(REG_IP0,REG_IP0,1);
    STR32(REG_IP0,REG_LINECYC,0);
    CMP32(REG_IP0,0);
    generate_blt(0);

    // if ((arm.event && 0xff)!=0) jump to end;
    AND64(REG_IP0,REG_IP0,REG_XZR);
    LDR32(REG_IP0,REG_ARM,offsetof(ARMState, event)); //arm.event
    TST32(REG_IP0,1,0,7); // #0xff
    generate_bne(0);

    // load PC into IP0
    gen_load_reg(REG_IP0,15);

    if (arm.r15_mask != 0xfffffffc) {
        AND32(REG_IP0,REG_IP0,REG_PCMASK);
    }

    if (((opcode >> 20) & 0xff) == 0xaf) {
        generate_load_addr_into_reg(REG_IP1,(void*)currentblockpc); // CMP $currentblockpc,%eax
        CMPR32(REG_IP0,REG_IP1);
        generate_beq(block_enter);
    }

    SUB32(REG_IP0,REG_IP0,8);

    generate_load_addr_into_reg(REG_SCRATCH,(void*)0x1fffc);
    AND64(REG_IP1,REG_IP0,REG_SCRATCH);

    LDRR32(REG_SCRATCH,REG_CODEBLOCKPC,REG_IP1);
    CMPR32(REG_SCRATCH,REG_IP0);
    generate_bne(0);

    LDRR32(REG_IP0,REG_CODEBLOCKNUM,REG_IP1);
    LSL64IMM(REG_IP0,REG_IP0,3);
    LDRR64(REG_IP0,REG_CODEBLOCKADDR,REG_IP0);
    ADD64(REG_IP0,REG_IP0,block_enter);
    BR(REG_IP0);


#ifdef __APPLE__
    pthread_jit_write_protect_np(1);
    sys_icache_invalidate(rcodeblock+(blockpoint2*BLOCK_ALLOC_SIZE),BLOCK_ALLOC_SIZE);
#endif

    if (haverecompiled>0) {
 //     ((void(*)(void))  (rcodeblock+(BLOCK_ALLOC_SIZE*blockpoint2)+BLOCKSTART)) ();
    }
}

void
generateflagtestandbranch(uint32_t opcode, uint32_t *pcpsr)
{
    if (pcpsr!=lastpcpsr) {
        generate_load_addr_into_reg(REG_PCPSR        ,pcpsr);
        lastpcpsr = pcpsr;
    }
    int cond;

    if ((opcode >> 28) == 0xe) {
        // No need if 'always' condition code
        return;
    }
    if (pcpsr!=lastpcpsr) {
        generate_load_addr_into_reg(REG_PCPSR        ,pcpsr);
        lastpcpsr = pcpsr;
    }
    generate_load_addr_into_reg(REG_X0,0); // Possibly branching to after a fn return, setup a possible return value.
    AND32(REG_IP1,REG_IP1,REG_XZR);
    LDR32(REG_IP1,REG_PCPSR,0);
    LSR32IMM(REG_IP1,REG_IP1,24);

    cond = ((opcode >> 28) & 1) ? COND_NE : COND_EQ;
    switch (opcode >> 28) {
    case 0: // EQ
    case 1: // NE
        TST32(REG_IP1,1,58,0); // 0x40
        break;
    case 2: // CS
    case 3: // CC
        TST32(REG_IP1,1,59,0); // 0x20
        break;
    case 4: // MI
    case 5: // PL
        TST32(REG_IP1,1,57,0); // 0x80
        break;
    case 6: // VS
    case 7: // VC
        TST32(REG_IP1,1,60,0); // 0x10
        break;
    default:
        generate_load_addr_into_reg(REG_SCRATCH,&flaglookup[opcode >> 28][0]);
        LSR32IMM(REG_IP1,REG_IP1,4);
        LDRR8(REG_IP1,REG_SCRATCH,REG_IP1);
        CMP32(REG_IP1,0);
        cond = COND_EQ;
        break;
    }
    //printf("Generating forward jump from flagtandb\n");
    lastflagchange = generate_forward_jump(cond);
}

void
generateirqtest(void)
{
    if (lastrecompiled) {
        return;
    }

    //ANDS32(REG_X0,REG_X0,REG_X0); // Return value of last called op fn, non zero on IRQ
    CMP32(REG_X0,0);
    generate_bne(0);
    if (lastflagchange!=0) {
        generate_forward_jump_here(lastflagchange);
    }
}

void
generateupdatepc(void)
{
    if (pcinc != 0) {
        gen_load_reg(REG_IP0,15);
        ADD32(REG_IP0,REG_IP0,pcinc);
        gen_save_reg(REG_IP0,15);
        pcinc = 0;
    }
}

void
generateupdateinscount(void)
{
    if (tempinscount!= 0) {
        LDR32(REG_IP0,REG_INSCOUNT,0);
        ADD32(REG_IP0,REG_IP0,tempinscount);
        STR32(REG_IP0,REG_INSCOUNT,0);
        tempinscount = 0;
    }
}

void
generatepcinc(void)
{
    tempinscount++;
    pcinc += 4;
    if (pcinc == 124) {
        generateupdatepc();
    }
    if (codeblockpos >= 500) {
        blockend = 1;
    }
}


static int  generate_forward_jump(int condition) {
    int jump_offset_pos;

    jump_offset_pos = codeblockpos;
    BCOND(0,condition); // Will be updated later by generate_forward_jump_here
 //   printf("Generated forward jump at offset: %d\n",jump_offset_pos);
    return jump_offset_pos;
}

static void generate_forward_jump_here(int jump_offset_pos) {
    int rel = codeblockpos - jump_offset_pos;

#ifdef __APPLE__
    uint32_t *addr = (void*)(rcodeblock+(BLOCK_ALLOC_SIZE*blockpoint2)+jump_offset_pos);
#else
    uint32_t *addr = (void*)&rcodeblock[blockpoint2][jump_offset_pos];
#endif
    *addr = (*addr) | (MASK_19(SIGN_CONTRACT((rel/4),19))<<5);
//printf("Completed jump for offset: %d\n",jump_offset_pos);
}

static int generate_function_call(void *addr) {
    generate_load_addr_into_reg(REG_X8,(void*)addr);
    BRL(REG_X8);
}

static void generate_load_addr_into_reg(short reg, void *addr) {
    uint64_t address = (uint64_t)addr;
    int cleared=0;

    if (addr==0) {
        AND64(reg,reg,REG_XZR);
    }
//    printf("Generating load into R%d = 0x%llx\n",reg,address);
    if ((address>>48)&0xffff) {
        MOVZ64SHL48(reg,((address>>48)&0xffff));
        cleared=1;
    }
    if ((address>>32)&0xffff) {
        MOVZK64SHL32(cleared,reg,((address>>32)&0xffff));
        cleared=1;
    }
    if ((address>>16)&0xffff) {
        MOVZK64SHL16(cleared,reg,((address>>16)&0xffff));
        cleared=1;
    }
    if (address&0xffff) {
        MOVZK64(cleared,reg,(address&0xffff));
    }
}

static void generate_br(int block_offset,int with_link) {
    if (with_link) {
        BL(((block_offset - codeblockpos)));
    } else {
        B(((block_offset - codeblockpos)));
    }
}

static void generate_bne(int block_offset) {
    BNE(((block_offset - codeblockpos)));
}

static void generate_bmi(int block_offset) {
    BMI(((block_offset - codeblockpos)));
}

static void generate_beq(int block_offset) {
    BEQ(((block_offset - codeblockpos)));
}

static void generate_blt(int block_offset) {
    BLT(((block_offset - codeblockpos)));
}

static int recompile(uint32_t opcode, uint32_t *pcpsr) {
    return 0;
}
