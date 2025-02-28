/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/testcore/kernel/misctest.H $                          */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2019                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef __TESTCORE_KERNEL_MISC_H
#define __TESTCORE_KERNEL_MISC_H

#include <sys/misc.h>
#include <kernel/cpumgr.H>
#include <targeting/common/targetservice.H>

/** @file misctest.H
 *
 *  Testcases for the system calls in sys/misc.h
 */

class MiscTest : public CxxTest::TestSuite
{
    public:

        /** Tests for cpu_spr_value() */
        void testCpuSprValue()
        {
            if (WAKEUP_MSR_VALUE != cpu_spr_value(CPU_SPR_MSR))
            {
                TS_FAIL("MSR value is not as expected.");
            }

            if (WAKEUP_LPCR_VALUE != cpu_spr_value(CPU_SPR_LPCR))
            {
                TS_FAIL("LPCR value is not as expected.");
            }
        }

        /** Tests for mm_virt_to_phys() */
        void testVirtToPhys()
        {
            uint64_t phys = 0;
            uint64_t hrmor = cpu_spr_value(CPU_SPR_HRMOR);

            // Verify a regular heap address
            uint8_t* heap = (uint8_t*)malloc(1);
            *heap = 0xAB; //to make sure it gets paged in
            phys = mm_virt_to_phys( heap );
            if( phys != (reinterpret_cast<uint64_t>(heap)|hrmor) )
            {
                TS_TRACE( "heap> virt=%p, phys=%lX", (void*)heap, phys );
                TS_FAIL("Unexpected Physical Address for Heap.");
            }
            free(heap);

            // Verify a regular stack address
            phys = mm_virt_to_phys( (void*)&phys );
            if( phys < hrmor )
            {
                TS_FAIL("Unexpected Physical Address for Stack.");
                TS_TRACE( "stack> virt=%p, phys=%lX", &phys, phys );
            }

            // Verify a MMIO (XSCOM)
            TARGETING::Target * l_masterProc =   nullptr;
            TARGETING::Target * l_masterNode =   nullptr;
            bool l_onlyFunctional = true; // Make sure masterproc is functional
            errlHndl_t l_err(nullptr);
            l_err = TARGETING::targetService().queryMasterProcChipTargetHandle(
                                                 l_masterProc,
                                                 l_masterNode,
                                                 l_onlyFunctional);

            //Validate we found a master proc and
            //  didn't encounter any error finding it
            if(l_masterProc != nullptr && !l_err)
            {
                uint64_t xscom =
                  l_masterProc->getAttr<TARGETING::ATTR_XSCOM_VIRTUAL_ADDR>();
                phys = mm_virt_to_phys( (void*)xscom );
                  if( (phys != (1020*TERABYTE+32*GIGABYTE))
                    && (xscom != 0) ) //never got set
                {
                    TS_FAIL("Unexpected Physical Address for MMIO.");
                    TS_TRACE( "mmio1> virt=%lX, phys=%lX\n", xscom, phys );
                }
            }


            /* Hardcoded interrupt presenter address for testing on 1-proc model
            uint64_t intr = 0x20800000000;
            phys = mm_virt_to_phys( (void*)intr );
            TS_TRACE( "mmio2> virt=%lX, phys=%lX\n", intr, phys );
            */
        }
};

#endif
