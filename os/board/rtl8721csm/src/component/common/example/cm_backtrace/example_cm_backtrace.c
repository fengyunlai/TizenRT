/*
 * This file is part of the CmBacktrace Library.
 *
 * Copyright (c) 2016-2017, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Initialize function and other general function.
 * Created on: 2016-12-15
 */

#include "example_cm_backtrace.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "platform_stdlib.h"

#if __STDC_VERSION__ < 199901L
    #error "must be C99 or higher. try to add '-std=c99' to compile parameters"
#endif

#if defined(__CC_ARM)
    #define SECTION_START(_name_)                _name_##$$Base
    #define SECTION_END(_name_)                  _name_##$$Limit
    #define IMAGE_SECTION_START(_name_)          Image$$##_name_##$$Base
    #define IMAGE_SECTION_END(_name_)            Image$$##_name_##$$Limit
    #define CSTACK_BLOCK_START(_name_)           SECTION_START(_name_)
    #define CSTACK_BLOCK_END(_name_)             SECTION_END(_name_)
    #define CODE_SECTION_START(_name_)           IMAGE_SECTION_START(_name_)
    #define CODE_SECTION_END(_name_)             IMAGE_SECTION_END(_name_)

    extern const int CSTACK_BLOCK_START(CMB_CSTACK_BLOCK_NAME);
    extern const int CSTACK_BLOCK_END(CMB_CSTACK_BLOCK_NAME);
    extern const int CODE_SECTION_START(CMB_CODE_SECTION_NAME);
    extern const int CODE_SECTION_END(CMB_CODE_SECTION_NAME);
#elif defined(__ICCARM__)
    #pragma section=CMB_CSTACK_BLOCK_NAME
    #pragma section=CMB_CODE_SECTION_NAME
#if defined(CONFIG_PLATFORM_8711B)	
    #pragma section=".text"
#elif defined(CONFIG_PLATFORM_8195A)		
    #pragma section=".heap.stdlib"
    #pragma section="CPP_INIT"
    #pragma section="CODE"	
    #pragma section=".sdram.text"
#elif defined(CONFIG_PLATFORM_8721D)		
    #pragma section=".text"
    #pragma section=".image2.ram.text"
    #pragma section=".image2.net.ram.text"	
    #pragma section=".psram.text"
#endif

#elif defined(__GNUC__)
    extern const int CMB_CSTACK_BLOCK_START;
    extern const int CMB_CSTACK_BLOCK_END;
    extern const int CMB_CODE_SECTION_START;
    extern const int CMB_CODE_SECTION_END;
#else
    #error "not supported compiler"
#endif

enum {
    PRINT_FIRMWARE_INFO,
    PRINT_ASSERT_ON_THREAD,
    PRINT_ASSERT_ON_HANDLER,
    PRINT_THREAD_STACK_INFO,
    PRINT_MAIN_STACK_INFO,
    PRINT_THREAD_STACK_OVERFLOW,
    PRINT_MAIN_STACK_OVERFLOW,
    PRINT_CALL_STACK_INFO,
    PRINT_CALL_STACK_ERR,
    PRINT_FAULT_ON_THREAD,
    PRINT_FAULT_ON_HANDLER,
    PRINT_REGS_TITLE,
    PRINT_HFSR_VECTBL,
    PRINT_MFSR_IACCVIOL,
    PRINT_MFSR_DACCVIOL,
    PRINT_MFSR_MUNSTKERR,
    PRINT_MFSR_MSTKERR,
    PRINT_MFSR_MLSPERR,
    PRINT_BFSR_IBUSERR,
    PRINT_BFSR_PRECISERR,
    PRINT_BFSR_IMPREISERR,
    PRINT_BFSR_UNSTKERR,
    PRINT_BFSR_STKERR,
    PRINT_BFSR_LSPERR,
    PRINT_UFSR_UNDEFINSTR,
    PRINT_UFSR_INVSTATE,
    PRINT_UFSR_INVPC,
    PRINT_UFSR_NOCP,
    PRINT_UFSR_UNALIGNED,
    PRINT_UFSR_DIVBYZERO,
    PRINT_DFSR_HALTED,
    PRINT_DFSR_BKPT,
    PRINT_DFSR_DWTTRAP,
    PRINT_DFSR_VCATCH,
    PRINT_DFSR_EXTERNAL,
    PRINT_MMAR,
    PRINT_BFAR,
    #if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
    PRINT_SFSR_LSERR,
    PRINT_SFSR_LSPERR,
    PRINT_SFSR_INVTRAN,
    PRINT_SFSR_AUVIOL,
    PRINT_SFSR_INVER,
    PRINT_SFSR_INVIS,
    PRINT_SFSR_INVEP,
    #endif
};

static const char *print_info[] = {
#if (CMB_PRINT_LANGUAGE == CMB_PRINT_LANGUAGE_ENGLISH)
        [PRINT_FIRMWARE_INFO]         = "Firmware name: %s, hardware version: %s, software version: %s",
        [PRINT_ASSERT_ON_THREAD]      = "Assert on thread %s",
        [PRINT_ASSERT_ON_HANDLER]     = "Assert on interrupt or bare metal(no OS) environment",
        [PRINT_THREAD_STACK_INFO]     = "===== Thread stack information =====",
        [PRINT_MAIN_STACK_INFO]       = "====== Main stack information ======",
        [PRINT_THREAD_STACK_OVERFLOW] = "Error: Thread stack(%08x) was overflow",
        [PRINT_MAIN_STACK_OVERFLOW]   = "Error: Main stack(%08x) was overflow",
        //[PRINT_CALL_STACK_INFO]       = "Show more call stack info by run: addr2line -e %s%s -a -f %s",
        [PRINT_CALL_STACK_INFO]       = "Show more call stack info by run: addr2line -e %s%s -f %s",        
        [PRINT_CALL_STACK_ERR]        = "Dump call stack has an error",
        [PRINT_FAULT_ON_THREAD]       = "Fault on thread %s",
        [PRINT_FAULT_ON_HANDLER]      = "Fault on interrupt or bare metal(no OS) environment",
        [PRINT_REGS_TITLE]            = "=================== Registers information ====================",
        [PRINT_HFSR_VECTBL]           = "Hard fault is caused by failed vector fetch",
        [PRINT_MFSR_IACCVIOL]         = "Memory management fault is caused by instruction access violation",
        [PRINT_MFSR_DACCVIOL]         = "Memory management fault is caused by data access violation",
        [PRINT_MFSR_MUNSTKERR]        = "Memory management fault is caused by unstacking error",
        [PRINT_MFSR_MSTKERR]          = "Memory management fault is caused by stacking error",
        [PRINT_MFSR_MLSPERR]          = "Memory management fault is caused by floating-point lazy state preservation",
        [PRINT_BFSR_IBUSERR]          = "Bus fault is caused by instruction access violation",
        [PRINT_BFSR_PRECISERR]        = "Bus fault is caused by precise data access violation",
        [PRINT_BFSR_IMPREISERR]       = "Bus fault is caused by imprecise data access violation",
        [PRINT_BFSR_UNSTKERR]         = "Bus fault is caused by unstacking error",
        [PRINT_BFSR_STKERR]           = "Bus fault is caused by stacking error",
        [PRINT_BFSR_LSPERR]           = "Bus fault is caused by floating-point lazy state preservation",
        [PRINT_UFSR_UNDEFINSTR]       = "Usage fault is caused by attempts to execute an undefined instruction",
        [PRINT_UFSR_INVSTATE]         = "Usage fault is caused by attempts to switch to an invalid state (e.g., ARM)",
        [PRINT_UFSR_INVPC]            = "Usage fault is caused by attempts to do an exception with a bad value in the EXC_RETURN number",
        [PRINT_UFSR_NOCP]             = "Usage fault is caused by attempts to execute a coprocessor instruction",
        [PRINT_UFSR_UNALIGNED]        = "Usage fault is caused by indicates that an unaligned access fault has taken place",
        [PRINT_UFSR_DIVBYZERO]        = "Usage fault is caused by Indicates a divide by zero has taken place (can be set only if DIV_0_TRP is set)",
        [PRINT_DFSR_HALTED]           = "Debug fault is caused by halt requested in NVIC",
        [PRINT_DFSR_BKPT]             = "Debug fault is caused by BKPT instruction executed",
        [PRINT_DFSR_DWTTRAP]          = "Debug fault is caused by DWT match occurred",
        [PRINT_DFSR_VCATCH]           = "Debug fault is caused by Vector fetch occurred",
        [PRINT_DFSR_EXTERNAL]         = "Debug fault is caused by EDBGRQ signal asserted",
        [PRINT_MMAR]                  = "The memory management fault occurred address is %08x",
        [PRINT_BFAR]                  = "The bus fault occurred address is %08x",
         #if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
        [PRINT_SFSR_LSERR]            = "Secure fault is caused during laze state",
        [PRINT_SFSR_LSPERR]           = "Secure fault is caused during laze preservation of floating-point state",
        [PRINT_SFSR_INVTRAN]          = "Secure fault is caused by transition error",
        [PRINT_SFSR_AUVIOL]           = "Secure fault is caused by attribution unit violation",
        [PRINT_SFSR_INVER]            = "Secure fault is caused by invalid exception return",
        [PRINT_SFSR_INVIS]            = "Secure fault is caused by invalid integrity signature",
        [PRINT_SFSR_INVEP]            = "Secure falut is caused by invalid entry point",
        #endif
#elif (CMB_PRINT_LANGUAGE == CMB_PRINT_LANUUAGE_CHINESE)
        [PRINT_FIRMWARE_INFO]         = "\B9̼\FE\C3\FB\B3ƣ\BA%s\A3\ACӲ\BC\FE\B0汾\BAţ\BA%s\A3\AC\C8\ED\BC\FE\B0汾\BAţ\BA%s",
        [PRINT_ASSERT_ON_THREAD]      = "\D4\DA\CF߳\CC(%s)\D6з\A2\C9\FA\B6\CF\D1\D4",
        [PRINT_ASSERT_ON_HANDLER]     = "\D4\DA\D6жϻ\F2\C2\E3\BB\FA\BB\B7\BE\B3\CF·\A2\C9\FA\B6\CF\D1\D4",
        [PRINT_THREAD_STACK_INFO]     = "=========== \CF̶߳\D1ջ\D0\C5Ϣ ===========",
        [PRINT_MAIN_STACK_INFO]       = "============ \D6\F7\B6\D1ջ\D0\C5Ϣ ============",
        [PRINT_THREAD_STACK_OVERFLOW] = "\B4\ED\CE\F3\A3\BA\CF߳\CCջ(%08x)\B7\A2\C9\FA\D2\E7\B3\F6",
        [PRINT_MAIN_STACK_OVERFLOW]   = "\B4\ED\CE\F3\A3\BA\D6\F7ջ(%08x)\B7\A2\C9\FA\D2\E7\B3\F6",
        [PRINT_CALL_STACK_INFO]       = "\B2鿴\B8\FC\B6ຯ\CA\FD\B5\F7\D3\C3ջ\D0\C5Ϣ\A3\AC\C7\EB\D4\CB\D0У\BAaddr2line -e %s%s -a -f %.*s",
        [PRINT_CALL_STACK_ERR]        = "\BB\F1ȡ\BA\AF\CA\FD\B5\F7\D3\C3ջʧ\B0\DC",
        [PRINT_FAULT_ON_THREAD]       =  "\D4\DA\CF߳\CC(%s)\D6з\A2\C9\FA\B4\ED\CE\F3\D2쳣",
        [PRINT_FAULT_ON_HANDLER]      = "\D4\DA\D6жϻ\F2\C2\E3\BB\FA\BB\B7\BE\B3\CF·\A2\C9\FA\B4\ED\CE\F3\D2쳣",
        [PRINT_REGS_TITLE]            = "========================= \BCĴ\E6\C6\F7\D0\C5Ϣ =========================",
        [PRINT_HFSR_VECTBL]           = "\B7\A2\C9\FAӲ\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BAȡ\D6ж\CF\CF\F2\C1\BFʱ\B3\F6\B4\ED",
        [PRINT_MFSR_IACCVIOL]         = "\B7\A2\C9\FA\B4洢\C6\F7\B9\DC\C0\ED\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C6\F3ͼ\B4Ӳ\BB\D4\CA\D0\ED\B7\C3\CEʵ\C4\C7\F8\D3\F2ȡָ\C1\EE",
        [PRINT_MFSR_DACCVIOL]         = "\B7\A2\C9\FA\B4洢\C6\F7\B9\DC\C0\ED\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C6\F3ͼ\B4Ӳ\BB\D4\CA\D0\ED\B7\C3\CEʵ\C4\C7\F8\D3\F2\B6\C1\A1\A2д\CA\FD\BE\DD",
        [PRINT_MFSR_MUNSTKERR]        = "\B7\A2\C9\FA\B4洢\C6\F7\B9\DC\C0\ED\B4\ED\CE\F3\A3\ACԭ\D2򣺳\F6ջʱ\C6\F3ͼ\B7\C3\CEʲ\BB\B1\BB\D4\CA\D0\ED\B5\C4\C7\F8\D3\F2",
        [PRINT_MFSR_MSTKERR]          = "\B7\A2\C9\FA\B4洢\C6\F7\B9\DC\C0\ED\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C8\EBջʱ\C6\F3ͼ\B7\C3\CEʲ\BB\B1\BB\D4\CA\D0\ED\B5\C4\C7\F8\D3\F2",
        [PRINT_MFSR_MLSPERR]          = "\B7\A2\C9\FA\B4洢\C6\F7\B9\DC\C0\ED\B4\ED\CE\F3\A3\ACԭ\D2򣺶\E8\D0Ա\A3\B4渡\B5\E3״̬ʱ\B7\A2\C9\FA\B4\ED\CE\F3",
        [PRINT_BFSR_IBUSERR]          = "\B7\A2\C9\FA\D7\DC\CFߴ\ED\CE\F3\A3\ACԭ\D2\F2\A3\BAָ\C1\EE\D7\DC\CFߴ\ED\CE\F3",
        [PRINT_BFSR_PRECISERR]        = "\B7\A2\C9\FA\D7\DC\CFߴ\ED\CE\F3\A3\ACԭ\D2򣺾\ABȷ\B5\C4\CA\FD\BE\DD\D7\DC\CFߴ\ED\CE\F3",
        [PRINT_BFSR_IMPREISERR]       = "\B7\A2\C9\FA\D7\DC\CFߴ\ED\CE\F3\A3\ACԭ\D2򣺲\BB\BE\ABȷ\B5\C4\CA\FD\BE\DD\D7\DC\CFߴ\ED\CE\F3",
        [PRINT_BFSR_UNSTKERR]         = "\B7\A2\C9\FA\D7\DC\CFߴ\ED\CE\F3\A3\ACԭ\D2򣺳\F6ջʱ\B7\A2\C9\FA\B4\ED\CE\F3",
        [PRINT_BFSR_STKERR]           = "\B7\A2\C9\FA\D7\DC\CFߴ\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C8\EBջʱ\B7\A2\C9\FA\B4\ED\CE\F3",
        [PRINT_BFSR_LSPERR]           = "\B7\A2\C9\FA\D7\DC\CFߴ\ED\CE\F3\A3\ACԭ\D2򣺶\E8\D0Ա\A3\B4渡\B5\E3״̬ʱ\B7\A2\C9\FA\B4\ED\CE\F3",
        [PRINT_UFSR_UNDEFINSTR]       = "\B7\A2\C9\FA\D3÷\A8\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C6\F3ͼִ\D0\D0δ\B6\A8\D2\E5ָ\C1\EE",
        [PRINT_UFSR_INVSTATE]         = "\B7\A2\C9\FA\D3÷\A8\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\CA\D4ͼ\C7л\BB\B5\BD ARM ״̬",
        [PRINT_UFSR_INVPC]            = "\B7\A2\C9\FA\D3÷\A8\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\CE\DEЧ\B5\C4\D2쳣\B7\B5\BB\D8\C2\EB",
        [PRINT_UFSR_NOCP]             = "\B7\A2\C9\FA\D3÷\A8\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C6\F3ͼִ\D0\D0Э\B4\A6\C0\ED\C6\F7ָ\C1\EE",
        [PRINT_UFSR_UNALIGNED]        = "\B7\A2\C9\FA\D3÷\A8\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C6\F3ͼִ\D0зǶ\D4\C6\EB\B7\C3\CE\CA",
        [PRINT_UFSR_DIVBYZERO]        = "\B7\A2\C9\FA\D3÷\A8\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\C6\F3ͼִ\D0г\FD 0 \B2\D9\D7\F7",
        [PRINT_DFSR_HALTED]           = "\B7\A2\C9\FA\B5\F7\CAԴ\ED\CE\F3\A3\ACԭ\D2\F2\A3\BANVIC ͣ\BB\FA\C7\EB\C7\F3",
        [PRINT_DFSR_BKPT]             = "\B7\A2\C9\FA\B5\F7\CAԴ\ED\CE\F3\A3\ACԭ\D2\F2\A3\BAִ\D0\D0 BKPT ָ\C1\EE",
        [PRINT_DFSR_DWTTRAP]          = "\B7\A2\C9\FA\B5\F7\CAԴ\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\CA\FD\BEݼ\E0\B2\E2\B5\E3ƥ\C5\E4",
        [PRINT_DFSR_VCATCH]           = "\B7\A2\C9\FA\B5\F7\CAԴ\ED\CE\F3\A3\ACԭ\D2򣺷\A2\C9\FA\CF\F2\C1\BF\B2\B6\BB\F1",
        [PRINT_DFSR_EXTERNAL]         = "\B7\A2\C9\FA\B5\F7\CAԴ\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\CDⲿ\B5\F7\CA\D4\C7\EB\C7\F3",
        [PRINT_MMAR]                  = "\B7\A2\C9\FA\B4洢\C6\F7\B9\DC\C0\ED\B4\ED\CE\F3\B5ĵ\D8ַ\A3\BA%08x",
        #if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
        [PRINT_BFAR]                  = "\B7\A2\C9\FA\D7\DC\CFߴ\ED\CE\F3\B5ĵ\D8ַ\A3\BA%08x",
        [PRINT_SFSR_LSERR]            = "\B7\A2\C9\FA\B0\B2ȫ\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BAlazy״̬\B7\A2\C9\FA\B4\ED\CE\F3",
        [PRINT_SFSR_LSPERR]           = "\B7\A2\C9\FA\B0\B2ȫ\B4\ED\CE\F3\A3\ACԭ\D2򣺱\A3\C1\F4lazyʱΥ\B1\B3SAU/IDAU\C9\E8\D6\C3",
        [PRINT_SFSR_INVTRAN]          = "\B7\A2\C9\FA\B0\B2ȫ\B4\ED\CE\F3\A3\ACԭ\D2򣺷\D6֧δ\B1\EA\BC\C7\D3\F2",
        [PRINT_SFSR_AUVIOL]           = "\B7\A2\C9\FA\B0\B2ȫ\B4\ED\CE\F3\A3\BAԭ\D2򣺷ǰ\B2ȫ\D3\F2\B7\C3\CEʰ\B2ȫ\BFռ\E4",
        [PRINT_SFSR_INVER]            = "\B7\A2\C9\FA\B0\B2ȫ\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\CE\DEЧ\B5\C4\D2쳣\B7\B5\BB\D8",
        [PRINT_SFSR_INVIS]            = "\B7\A2\C9\FA\B0\B2ȫ\B4\ED\CE\F3\A3\ACԭ\D2򣺳\F6ջʱ\CE\DEЧ\B5\C4ǩ\C3\FB",
        [PRINT_SFSR_INVEP]            = "\B7\A2\C9\FA\B0\B2ȫ\B4\ED\CE\F3\A3\ACԭ\D2\F2\A3\BA\CE\DEЧ\B5\C4\C8\EB\BF\DA",
        #endif
#else
    #error "CMB_PRINT_LANGUAGE defined error in 'cmb_cfg.h'"
#endif
};

static char fw_name[CMB_NAME_MAX] = {0};
static char hw_ver[CMB_NAME_MAX] = {0};
static char sw_ver[CMB_NAME_MAX] = {0};
static uint32_t main_stack_start_addr = 0;
static size_t main_stack_size = 0;
static uint32_t code_start_addr = 0;
static size_t code_size = 0;
static uint32_t sdram_code_start_addr = 0;
static size_t sdram_code_size = 0;
static size_t sdram_code_enable = 0;
static uint32_t psram_code_start_addr = 0;
static size_t psram_code_size = 0;
static size_t psram_code_enable = 0;

static bool init_ok = false;
static char call_stack_info[CMB_CALL_STACK_MAX_DEPTH * (8 + 1)] = { 0 };
static bool on_fault = false;
static bool stack_is_overflow = false;
static struct cmb_hard_fault_regs regs;

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
static bool statck_has_fpu_regs = false;
#endif

static bool on_thread_before_fault = false;

#define CMB_MSP_END_NAME          ".heap.stdlib"

void fault_test_by_unalign(void) {
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    volatile int * p;
    volatile int value;

    *SCB_CCR |= (1 << 3); /* bit3: UNALIGN_TRP. */

    p = (int *) 0x00;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x04;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x03;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);
}

void fault_test_by_div0(void) {
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    int x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

    x = 10;
    y = 0;
    z = x / y;
    printf("z:%d\n", z);
}

/**
 * library initialize
 */
void cm_backtrace_init(const char *firmware_name, const char *hardware_ver, const char *software_ver) {
    strncpy(fw_name, firmware_name, CMB_NAME_MAX);
    strncpy(hw_ver, hardware_ver, CMB_NAME_MAX);
    strncpy(sw_ver, software_ver, CMB_NAME_MAX);

#if defined(__CC_ARM)
    main_stack_start_addr = (uint32_t)&CSTACK_BLOCK_START(CMB_CSTACK_BLOCK_NAME);
    main_stack_size = (uint32_t)&CSTACK_BLOCK_END(CMB_CSTACK_BLOCK_NAME) - main_stack_start_addr;
    code_start_addr = (uint32_t)&CODE_SECTION_START(CMB_CODE_SECTION_NAME);
    code_size = (uint32_t)&CODE_SECTION_END(CMB_CODE_SECTION_NAME) - code_start_addr;
#elif defined(__ICCARM__)
#if defined(CONFIG_PLATFORM_8711B)	
    code_start_addr = (uint32_t)__section_begin(".text");
    code_size = (uint32_t)__section_end(".text") - code_start_addr;

    main_stack_start_addr = 0x1003EFFC;
    main_stack_size = 0xFFC;   	
#elif defined(CONFIG_PLATFORM_8195A)	
    code_start_addr = (uint32_t)__section_begin("CPP_INIT");
    code_size = (uint32_t)__section_end("CODE") - code_start_addr;

    sdram_code_start_addr = (uint32_t)__section_begin(".sdram.text");
    sdram_code_size = (uint32_t)__section_end(".sdram.text") - sdram_code_start_addr;
    if(sdram_code_size != 0 || (sdram_code_start_addr&0x30000000) == 0x30000000)
    {
        sdram_code_enable = 1;
    }

    main_stack_start_addr = 0x1ffffffc;
    main_stack_size = 0x1ffffffc - (uint32_t)__section_end(".heap.stdlib");   

#elif defined(CONFIG_PLATFORM_8721D)	
    code_start_addr = (uint32_t)__section_begin(".text");
    code_size = (uint32_t)__section_end(".text") - code_start_addr;

    sdram_code_start_addr = (uint32_t)__section_begin(".image2.ram.text");
    sdram_code_size = (uint32_t)__section_end(".image2.net.ram.text") - sdram_code_start_addr;
    if(sdram_code_size != 0 )
    {
        sdram_code_enable = 1;
    }

    psram_code_start_addr = (uint32_t)__section_begin(".psram.text");
    psram_code_size = (uint32_t)__section_end(".psram.text") - psram_code_start_addr;
    if(psram_code_size != 0)
    {
        psram_code_enable = 1;
    }

    main_stack_start_addr = 0x10005000;
    main_stack_size = 0x10005000 - 0x10004000;   
#endif

    //printf("0x%x, %d,  code 0x%x, %d, sdram 0x%x, %d, enable sdram %d\r\n",main_stack_start_addr, main_stack_size, code_start_addr, code_size, sdram_code_start_addr, sdram_code_size, sdram_code_enable);
#elif defined(__GNUC__)
#if defined(CONFIG_PLATFORM_8721D)	
    extern u8 __flash_text_start__[];
    extern u8 __flash_text_end__[];
    extern u8 __ram_text_start__[];
    extern u8 __ram_text_end__[];
    extern u8 __ram_text_start__[];
    extern u8 __ram_text_end__[];
    extern u8 __psram_image2_text_start__[];
    extern u8 __psram_image2_text_end__[];

    code_start_addr = (uint32_t) __flash_text_start__;
    code_size = (uint32_t)(__flash_text_end__ - __flash_text_start__);

    sdram_code_start_addr = (uint32_t) __ram_text_start__;
    sdram_code_size = (uint32_t)(__ram_text_end__ - __ram_text_start__);
    if(sdram_code_size != 0)
    {
        sdram_code_enable = 1;
    }

     psram_code_start_addr = (uint32_t) __ram_text_start__;
    psram_code_size = (uint32_t)(__ram_text_end__ - __ram_text_start__);
    if(psram_code_size != 0)
    {
        psram_code_enable = 1;
    }

	
    main_stack_start_addr = 0x10005000;
    main_stack_size = 0x10005000 - 0x10004000;   

#else
    main_stack_start_addr = (uint32_t)(&CMB_CSTACK_BLOCK_START);
    main_stack_size = (uint32_t)(&CMB_CSTACK_BLOCK_END) - main_stack_start_addr;
    code_start_addr = (uint32_t)(&CMB_CODE_SECTION_START);
    code_size = (uint32_t)(&CMB_CODE_SECTION_END) - code_start_addr;
#endif

#else
    #error "not supported compiler"
#endif

    init_ok = true;
}

/**
 * print firmware information, such as: firmware name, hardware version, software version
 */
void cm_backtrace_firmware_info(void) {
    cmb_println(print_info[PRINT_FIRMWARE_INFO], fw_name, hw_ver, sw_ver);
}

#ifdef CMB_USING_OS_PLATFORM
/**
 * Get current thread stack information
 *
 * @param sp stack current pointer
 * @param start_addr stack start address
 * @param size stack size
 */
static void get_cur_thread_stack_info(uint32_t sp, uint32_t *start_addr, size_t *size) {
    CMB_ASSERT(start_addr);
    CMB_ASSERT(size);

#if (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_RTT)
    *start_addr = (uint32_t) rt_thread_self()->stack_addr;
    *size = rt_thread_self()->stack_size;
#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSII)
    extern OS_TCB *OSTCBCur;

    *start_addr = (uint32_t) OSTCBCur->OSTCBStkBottom;
    *size = OSTCBCur->OSTCBStkSize * sizeof(OS_STK);
#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSIII)
    #error "not implemented, I hope you can do this"
    //TODO \B4\FDʵ\CF\D6
#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_FREERTOS)   
    *start_addr = (uint32_t)vTaskStackAddr();
    *size = vTaskStackSize() * sizeof( StackType_t );
	printf("\r\nstact addr 0x%x, size 0x%x, top 0x%x\r\n", *start_addr, *size, (uint32_t)vTaskStackTOPAddr());
#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_TIZENRT)   
    *start_addr = (uint32_t)vTaskStackAddr();
    *size = vTaskStackSize();
	printf("\r\nstact addr 0x%x, size 0x%x\r\n", *start_addr, *size);
#endif
}

/**
 * Get current thread name
 */
static const char *get_cur_thread_name(void) {
#if (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_RTT)
    return rt_thread_self()->name;
#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSII)
    extern OS_TCB *OSTCBCur;

#if OS_TASK_NAME_SIZE > 0 || OS_TASK_NAME_EN > 0
        return (const char *)OSTCBCur->OSTCBTaskName;
#else
        return NULL;
#endif /* OS_TASK_NAME_SIZE > 0 || OS_TASK_NAME_EN > 0 */

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSIII)
    #error "not implemented, I hope you can do this"
    //TODO \B4\FDʵ\CF\D6
#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_FREERTOS)
    return vTaskName();
#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_TIZENRT)
    return vTaskName();
#endif
}

#endif /* CMB_USING_OS_PLATFORM */

#ifdef CMB_USING_DUMP_STACK_INFO
/**
 * dump current stack information
 */
static void dump_stack(uint32_t stack_start_addr, size_t stack_size, uint32_t *stack_pointer) {
    if (stack_is_overflow) {
        if (on_thread_before_fault) {
            cmb_println(print_info[PRINT_THREAD_STACK_OVERFLOW], stack_pointer);
        } else {
            cmb_println(print_info[PRINT_MAIN_STACK_OVERFLOW], stack_pointer);
        }
        if ((uint32_t) stack_pointer < stack_start_addr) {
            stack_pointer = (uint32_t *) stack_start_addr;
        } else if ((uint32_t) stack_pointer > stack_start_addr + stack_size) {
            stack_pointer = (uint32_t *) (stack_start_addr + stack_size);
        }
    }
#if (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_FREERTOS)
    /*add by raul for more stack infomation*/
    cmb_println("dump_stack sp point 0x%x, task sp 0x%x\r\n", stack_pointer, vTaskStackTOPAddr());
    if(on_thread_before_fault && stack_pointer >  (uint32_t*)vTaskStackTOPAddr())
        stack_pointer = (uint32_t*)vTaskStackTOPAddr();
#endif
    cmb_println(print_info[PRINT_THREAD_STACK_INFO]);
    for (; (uint32_t) stack_pointer < stack_start_addr + stack_size; stack_pointer++) {
        cmb_println("  addr: %x    data: %x", stack_pointer, *stack_pointer);
    }
    cmb_println("====================================");
}
#endif /* CMB_USING_DUMP_STACK_INFO */

/**
 * backtrace function call stack
 *
 * @param buffer call stack buffer
 * @param size buffer size
 * @param sp stack pointer
 *
 * @return depth
 */
size_t cm_backtrace_call_stack(uint32_t *buffer, size_t size, uint32_t sp) {
    uint32_t stack_start_addr = main_stack_start_addr-main_stack_size, pc;
    size_t depth = 0, stack_size = main_stack_size;
    bool regs_saved_lr_is_valid = false;

    if (on_fault) {
        if (!stack_is_overflow) {
            /* first depth is PC */
            buffer[depth++] = regs.saved.pc;
			
            /* second depth is from LR, so need decrease a word to PC */
            pc = regs.saved.lr - sizeof(size_t);
            if ((((pc >= code_start_addr) && (pc <= code_start_addr + code_size))
				||(sdram_code_enable && (pc >= sdram_code_start_addr) && (pc <= sdram_code_start_addr + sdram_code_size))
				||(psram_code_enable && (pc >= psram_code_start_addr) && (pc <= psram_code_start_addr + psram_code_size))) 
			&& (depth < CMB_CALL_STACK_MAX_DEPTH)
                    && (depth < size)) {
                buffer[depth++] = pc;
                regs_saved_lr_is_valid = true;
            }
        }

#ifdef CMB_USING_OS_PLATFORM
        /* program is running on thread before fault */
        if (on_thread_before_fault) {
            get_cur_thread_stack_info(sp, &stack_start_addr, &stack_size);
        }
    } else {
        /* OS environment */
        if (cmb_get_sp() == cmb_get_psp()) {
            get_cur_thread_stack_info(sp, &stack_start_addr, &stack_size);
        }
#endif /* CMB_USING_OS_PLATFORM */

    }

    if (stack_is_overflow) {
        if (sp < stack_start_addr) {
            sp = stack_start_addr;
        } else if (sp > stack_start_addr + stack_size) {
            sp = stack_start_addr + stack_size;
        }
    }

    /* copy called function address */
    for (; sp < stack_start_addr + stack_size; sp += sizeof(size_t)) {
        /* the *sp value may be LR, so need decrease a word to PC */
        pc = *((uint32_t *) sp) - sizeof(size_t);
            if ((((pc >= code_start_addr) && (pc <= code_start_addr + code_size))
				||(sdram_code_enable && (pc >= sdram_code_start_addr) && (pc <= sdram_code_start_addr + sdram_code_size))) 
			&& (depth < CMB_CALL_STACK_MAX_DEPTH)
			&& (depth < size)) {
            /* the second depth function may be already saved, so need ignore repeat */
            if ((depth == 2) && regs_saved_lr_is_valid && (pc == buffer[1])) {
                continue;
            }
            buffer[depth++] = pc;
        }
    }

    return depth;
}

/**
 * dump function call stack
 *
 * @param sp stack pointer
 */
static void print_call_stack(uint32_t sp) {
    size_t i, cur_depth = 0;
    uint32_t call_stack_buf[CMB_CALL_STACK_MAX_DEPTH] = {0};

    cur_depth = cm_backtrace_call_stack(call_stack_buf, CMB_CALL_STACK_MAX_DEPTH, sp);

    for (i = 0; i < cur_depth; i++) {
        DiagSPrintf(call_stack_info + i * (8 + 1), "%8x", call_stack_buf[i]);
        call_stack_info[i * (8 + 1) + 8] = ' ';
    }

    if (cur_depth) {
        cmb_println(print_info[PRINT_CALL_STACK_INFO], fw_name, CMB_ELF_FILE_EXTENSION_NAME,
                call_stack_info);
    } else {
        cmb_println(print_info[PRINT_CALL_STACK_ERR]);
    }
}

/**
 * backtrace for assert
 *
 * @param sp the stack pointer when on assert occurred
 */
void cm_backtrace_assert(uint32_t sp) {
    CMB_ASSERT(init_ok);

#ifdef CMB_USING_OS_PLATFORM
    uint32_t cur_stack_pointer = cmb_get_sp();
#endif

    cmb_println("");
    cm_backtrace_firmware_info();

#ifdef CMB_USING_OS_PLATFORM
    /* OS environment */
    if (cur_stack_pointer == cmb_get_msp()) {
        cmb_println(print_info[PRINT_ASSERT_ON_HANDLER]);

#ifdef CMB_USING_DUMP_STACK_INFO
        dump_stack(main_stack_start_addr, main_stack_size, (uint32_t *) sp);
#endif /* CMB_USING_DUMP_STACK_INFO */

    } else if (cur_stack_pointer == cmb_get_psp()) {
        cmb_println(print_info[PRINT_ASSERT_ON_THREAD], get_cur_thread_name());

#ifdef CMB_USING_DUMP_STACK_INFO
        uint32_t stack_start_addr;
        size_t stack_size;
        get_cur_thread_stack_info(sp, &stack_start_addr, &stack_size);
        dump_stack(stack_start_addr, stack_size, (uint32_t *) sp);
#endif /* CMB_USING_DUMP_STACK_INFO */

    }

#else

    /* bare metal(no OS) environment */
#ifdef CMB_USING_DUMP_STACK_INFO
    dump_stack(main_stack_start_addr, main_stack_size, (uint32_t *) sp);
#endif /* CMB_USING_DUMP_STACK_INFO */

#endif /* CMB_USING_OS_PLATFORM */

    print_call_stack(sp);
}

#if (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0) && (CMB_CPU_PLATFORM_TYPE != CMB_CPU_REALTEK_KM0)
/**
 * fault diagnosis then print cause of fault
 */
static void fault_diagnosis(void) {
    if (regs.hfsr.bits.VECTBL) {
        cmb_println(print_info[PRINT_HFSR_VECTBL]);
    }
    if (regs.hfsr.bits.FORCED) {
        /* Memory Management Fault */
        if (regs.mfsr.value) {
            if (regs.mfsr.bits.IACCVIOL) {
                cmb_println(print_info[PRINT_MFSR_IACCVIOL]);
            }
            if (regs.mfsr.bits.DACCVIOL) {
                cmb_println(print_info[PRINT_MFSR_DACCVIOL]);
            }
            if (regs.mfsr.bits.MUNSTKERR) {
                cmb_println(print_info[PRINT_MFSR_MUNSTKERR]);
            }
            if (regs.mfsr.bits.MSTKERR) {
                cmb_println(print_info[PRINT_MFSR_MSTKERR]);
            }

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7)
            if (regs.mfsr.bits.MLSPERR) {
                cmb_println(print_info[PRINT_MFSR_MLSPERR]);
            }
#endif

            if (regs.mfsr.bits.MMARVALID) {
                if (regs.mfsr.bits.IACCVIOL || regs.mfsr.bits.DACCVIOL) {
                    cmb_println(print_info[PRINT_MMAR], regs.mmar);
                }
            }
        }
        /* Bus Fault */
        if (regs.bfsr.value) {
            if (regs.bfsr.bits.IBUSERR) {
                cmb_println(print_info[PRINT_BFSR_IBUSERR]);
            }
            if (regs.bfsr.bits.PRECISERR) {
                cmb_println(print_info[PRINT_BFSR_PRECISERR]);
            }
            if (regs.bfsr.bits.IMPREISERR) {
                cmb_println(print_info[PRINT_BFSR_IMPREISERR]);
            }
            if (regs.bfsr.bits.UNSTKERR) {
                cmb_println(print_info[PRINT_BFSR_UNSTKERR]);
            }
            if (regs.bfsr.bits.STKERR) {
                cmb_println(print_info[PRINT_BFSR_STKERR]);
            }

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7)
            if (regs.bfsr.bits.LSPERR) {
                cmb_println(print_info[PRINT_BFSR_LSPERR]);
            }
#endif

            if (regs.bfsr.bits.BFARVALID) {
                if (regs.bfsr.bits.PRECISERR) {
                    cmb_println(print_info[PRINT_BFAR], regs.bfar);
                }
            }

        }
        /* Usage Fault */
        if (regs.ufsr.value) {
            if (regs.ufsr.bits.UNDEFINSTR) {
                cmb_println(print_info[PRINT_UFSR_UNDEFINSTR]);
            }
            if (regs.ufsr.bits.INVSTATE) {
                cmb_println(print_info[PRINT_UFSR_INVSTATE]);
            }
            if (regs.ufsr.bits.INVPC) {
                cmb_println(print_info[PRINT_UFSR_INVPC]);
            }
            if (regs.ufsr.bits.NOCP) {
                cmb_println(print_info[PRINT_UFSR_NOCP]);
            }
            if (regs.ufsr.bits.UNALIGNED) {
                cmb_println(print_info[PRINT_UFSR_UNALIGNED]);
            }
            if (regs.ufsr.bits.DIVBYZERO) {
                cmb_println(print_info[PRINT_UFSR_DIVBYZERO]);
            }
        }
    }
    /* Debug Fault */
    if (regs.hfsr.bits.DEBUGEVT) {
        if (regs.dfsr.value) {
            if (regs.dfsr.bits.HALTED) {
                cmb_println(print_info[PRINT_DFSR_HALTED]);
            }
            if (regs.dfsr.bits.BKPT) {
                cmb_println(print_info[PRINT_DFSR_BKPT]);
            }
            if (regs.dfsr.bits.DWTTRAP) {
                cmb_println(print_info[PRINT_DFSR_DWTTRAP]);
            }
            if (regs.dfsr.bits.VCATCH) {
                cmb_println(print_info[PRINT_DFSR_VCATCH]);
            }
            if (regs.dfsr.bits.EXTERNAL) {
                cmb_println(print_info[PRINT_DFSR_EXTERNAL]);
            }
        }
    }

	#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
	if (regs.sfsr.value) {
		if (regs.sfsr.bits.LSERR) {
			cmb_println(print_info[PRINT_SFSR_LSERR]);
		}
		if (regs.sfsr.bits.LSPERR) {
			cmb_println(print_info[PRINT_SFSR_LSPERR]);
		}
		if (regs.sfsr.bits.INVTRAN) {
			cmb_println(print_info[PRINT_SFSR_INVTRAN]);
		}
		if (regs.sfsr.bits.AUVIOL) {
			cmb_println(print_info[PRINT_SFSR_AUVIOL]);
		}
		if (regs.sfsr.bits.INVER) {
			cmb_println(print_info[PRINT_SFSR_INVER]);
		}
		if (regs.sfsr.bits.INVIS) {
			cmb_println(print_info[PRINT_SFSR_INVIS]);
		}
		if (regs.sfsr.bits.INVEP) {
			cmb_println(print_info[PRINT_SFSR_INVEP]);
		}
		if (regs.sfsr.bits.SFARVALID) {
			cmb_println("Secure voilation address is 0x%x", CMB_NVIC_SFAR);
		}
	}
	#endif
}
#endif /* (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0) */

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
static uint32_t statck_del_fpu_regs(uint32_t fault_handler_lr, uint32_t sp) {
    statck_has_fpu_regs = (fault_handler_lr & (1UL << 4)) == 0 ? true : false;

	if (statck_has_fpu_regs == true) {
		/* the stack has S0~S15 and FPSCR registers when statck_has_fpu_regs is true, double word align */
		sp += sizeof(size_t) * 18;
		#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
		/* the stack has S16-S31 registers when FPCCR_S.TS is 1 in secure state */
		if ((fault_handler_lr & (1UL << 6)) && (CMB_FPCCR_S & (1UL << 26)))
			sp += sizeof(size_t) * 16;
		#endif
		}
	return sp;
	}
#endif

/**
 * backtrace for fault
 * @note only call once
 *
 * @param fault_handler_lr the LR register value on fault handler
 * @param fault_handler_sp the stack pointer on fault handler
 */
 #if defined(CONFIG_PLATFORM_8711B)	||defined(CONFIG_PLATFORM_8721D)
void cm_backtrace_fault(uint32_t fault_handler_sp, uint32_t fault_handler_lr) {
#elif defined(CONFIG_PLATFORM_8195A)		
void cm_backtrace_fault(uint32_t fault_handler_sp) {
#endif
    cmb_println("Enter CM BackTrace, SP 0x%x\r\n", fault_handler_sp);
    uint32_t stack_pointer = fault_handler_sp, saved_regs_addr = stack_pointer;
    const char *regs_name[] = { "R0 ", "R1 ", "R2 ", "R3 ", "R12", "LR ", "PC ", "PSR" };

#ifdef CMB_USING_DUMP_STACK_INFO
    uint32_t stack_start_addr = main_stack_start_addr - main_stack_size;
    size_t stack_size = main_stack_size;
#endif

    CMB_ASSERT(init_ok);
    /* only call once */
    CMB_ASSERT(!on_fault);

    on_fault = true;

    cmb_println("");
    cm_backtrace_firmware_info();

#ifdef CMB_USING_OS_PLATFORM
    //on_thread_before_fault = fault_handler_lr & (1UL << 2);
    on_thread_before_fault = (fault_handler_sp == cmb_get_psp());
    /* check which stack was used before (MSP or PSP) */
    if (on_thread_before_fault) {
        cmb_println(print_info[PRINT_FAULT_ON_THREAD], get_cur_thread_name() != NULL ? get_cur_thread_name() : "NO_NAME");
        saved_regs_addr = stack_pointer = cmb_get_psp();

#ifdef CMB_USING_DUMP_STACK_INFO
        get_cur_thread_stack_info(stack_pointer, &stack_start_addr, &stack_size);
#endif /* CMB_USING_DUMP_STACK_INFO */

    } else {
        cmb_println(print_info[PRINT_FAULT_ON_HANDLER]);
    }
#else
    /* bare metal(no OS) environment */
    cmb_println(print_info[PRINT_FAULT_ON_HANDLER]);
#endif /* CMB_USING_OS_PLATFORM */

    /* delete saved R0~R3, R12, LR,PC,xPSR registers space */
    stack_pointer += sizeof(size_t) * 8;

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
#if defined(CONFIG_PLATFORM_8711B) || defined(CONFIG_PLATFORM_8721D)
	stack_pointer = statck_del_fpu_regs(fault_handler_lr, stack_pointer);
#endif
#endif /* (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7) */

#ifdef CMB_USING_DUMP_STACK_INFO
    /* check stack overflow */
    if (stack_pointer < stack_start_addr || stack_pointer > stack_start_addr + stack_size) {
        stack_is_overflow = true;
    }
    /* dump stack information */
    dump_stack(stack_start_addr, stack_size, (uint32_t *) stack_pointer);
#endif /* CMB_USING_DUMP_STACK_INFO */

    /* the stack frame may be get failed when it is overflow  */
    if (!stack_is_overflow) {
        /* dump register */
        cmb_println(print_info[PRINT_REGS_TITLE]);

        regs.saved.r0        = ((uint32_t *)saved_regs_addr)[0];  // Register R0
        regs.saved.r1        = ((uint32_t *)saved_regs_addr)[1];  // Register R1
        regs.saved.r2        = ((uint32_t *)saved_regs_addr)[2];  // Register R2
        regs.saved.r3        = ((uint32_t *)saved_regs_addr)[3];  // Register R3
        regs.saved.r12       = ((uint32_t *)saved_regs_addr)[4];  // Register R12
        regs.saved.lr        = ((uint32_t *)saved_regs_addr)[5];  // Link register LR
        regs.saved.pc        = ((uint32_t *)saved_regs_addr)[6];  // Program counter PC
        regs.saved.psr.value = ((uint32_t *)saved_regs_addr)[7];  // Program status word PSR

        cmb_println("  %s: %x\r\n  %s: %x\r\n  %s: %x\r\n  %s: %x\r\n", regs_name[0], regs.saved.r0,
                                                                regs_name[1], regs.saved.r1,
                                                                regs_name[2], regs.saved.r2,
                                                                regs_name[3], regs.saved.r3);
        cmb_println("  %s: %x\r\n  %s: %x\r\n  %s: %x\r\n  %s: %x\r\n", regs_name[4], regs.saved.r12,
                                                                regs_name[5], regs.saved.lr,
                                                                regs_name[6], regs.saved.pc,
                                                                regs_name[7], regs.saved.psr.value);
        cmb_println("==============================================================");
    }

    /* the Cortex-M0 is not support fault diagnosis */
#if (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0)  && (CMB_CPU_PLATFORM_TYPE != CMB_CPU_REALTEK_KM0)
    regs.syshndctrl.value = CMB_SYSHND_CTRL;  // System Handler Control and State Register
    regs.mfsr.value       = CMB_NVIC_MFSR;    // Memory Fault Status Register
    regs.mmar             = CMB_NVIC_MMAR;    // Memory Management Fault Address Register
    regs.bfsr.value       = CMB_NVIC_BFSR;    // Bus Fault Status Register
    regs.bfar             = CMB_NVIC_BFAR;    // Bus Fault Manage Address Register
    regs.ufsr.value       = CMB_NVIC_UFSR;    // Usage Fault Status Register
    regs.hfsr.value       = CMB_NVIC_HFSR;    // Hard Fault Status Register
    regs.dfsr.value       = CMB_NVIC_DFSR;    // Debug Fault Status Register
    regs.afsr             = CMB_NVIC_AFSR;    // Auxiliary Fault Status Register
    #if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_KM4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_REALTEK_TM9)
    regs.sfsr.value       = CMB_NVIC_SFSR;    // Secure Fault Status Register
    #endif

    fault_diagnosis();
#endif

    print_call_stack(stack_pointer);

    cmb_println("Exit CM BackTrace\r\n\r\n\r\n");

}
