/** @file
*
*  Copyright (c) 2018, Linaro Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

// The total number of descriptors, including the final "end-of-table" descriptor.
#define MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS 74

// DDR attributes
#define DDR_ATTRIBUTES_CACHED           ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define DDR_ATTRIBUTES_UNCACHED         ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED
#define SDM665_PERIPH_BASE              0x00000000
#define SDM665_PERIPH_SZ                0x40000000

STATIC struct ginkgoReservedMemory {
  EFI_PHYSICAL_ADDRESS         Offset;
  EFI_PHYSICAL_ADDRESS         Size;
} ginkgoReservedMemoryBuffer [] = {
    // DDR
    { 0x40000000, 0x05700000 }, // "Kernel"
    { 0x45700000, 0x00600000 }, // "HYP"
    { 0x45D00000, 0x00020000 }, // "Boot Info"
    { 0x85F00000, 0x00040000 }, // "AOP CMD DB"
    { 0x46000000, 0x00200000 }, // "SMEM"
    { 0x4AB00000, 0x09400000 }, // "PIL Reserved"
    { 0x53F00000, 0x02800000 }, // "DXE Heap"
    { 0x56700000, 0x00A00000 }, // "DBI Dump"
    { 0x57100000, 0x00400000 }, // "Sched Heap"
    { 0x5C000000, 0x01000000 }, // "Display Reserved"
    { 0x5F800000, 0x00200000 }, // "FV Region"
    { 0x5FA00000, 0x00200000 }, // "ABOOT FV"
    { 0x5FC00000, 0x00300000 }, // "UEFI FD"
    { 0x5FF00000, 0x0008C000 }, // "SEC Heap"
    { 0x5FF8C000, 0x00001000 }, // "CPU Vectors"
    { 0x5FF8D000, 0x00003000 }, // "MMU PageTables"
    { 0x5FF90000, 0x00040000 }, // "UEFI Stack"
    { 0x5FFF7000, 0x00008000 }, // "Log Buffer"
    { 0x5FFFF000, 0x00001000 }, // "Info Blk"

    // Other
    { 0x045F0000, 0x00007000 }, // "RPM_SS_MSG_RAM"
    { 0x0C100000, 0x00026000 }, // "IMEM Base"
    { 0x0C125000, 0x00001000 }, // "IMEM Cookie Base"

    // Register
    { 0x003C0000, 0x00040000 }, // "TCSR_TCSR_REGS"
    { 0x00500000, 0x00300000 }, // "TLMM_WEST"
    { 0x00900000, 0x00300000 }, // "TLMM_SOUTH"
    { 0x00D00000, 0x00300000 }, // "TLMM_EAST"
    { 0x01400000, 0x00200000 }, // "GCC CLK CTL"
    { 0x01C00000, 0x02800000 }, // "PMIC ARB SPMI"
    { 0x01648000, 0x00008000 }, // "MMCX_CPR3"
    { 0x01B00000, 0x00040000 }, // "CRYPTO0 CRYPTO"
    { 0x01B40000, 0x00010000 }, // "SECURITY CONTROL"
    { 0x01B50000, 0x00010000 }, // "PRNG_CFG_PRNG"
    { 0x04403000, 0x00001000 }, // "SLP_CNTR"
    { 0x04410000, 0x00001000 }, // "TSENS0"
    { 0x04411000, 0x00001000 }, // "TSENS0_TM"
    { 0x0440B000, 0x00001000 }, // "PSHOLD"
    { 0x04A00000, 0x000D0000 }, // "QUPV3_0_GSI"
    { 0x04C00000, 0x000D0000 }, // "QUPV3_1_GSI"
    { 0x04800000, 0x00020000 }, // "UFS UFS REGS"
    { 0x04700000, 0x00200000 }, // "PERIPH_SS"
    { 0x04E00000, 0x00200000 }, // "USB30_PRIM"
    { 0x0597D000, 0x0000C000 }, // "GPU_GMU_CX_BLK"
    { 0x05990000, 0x00009000 }, // "GPU_CC"
    { 0x05B00000, 0x00020000 }, // "VIDEO_CC"
    { 0x05E00000, 0x00200000 }, // "MDSS"
    { 0x05F00000, 0x00020000 }, // "DISP_CC_DISP_CC"
    { 0x0C600000, 0x00080000 }, // "SMMU"
    { 0x0F017000, 0x00001000 }, // "APSS_WDT_TMR1"
    { 0x0F020000, 0x00110000 }, // "QTIMER"
    { 0x0F111000, 0x00001000 }, // "APCS_ALIAS0_GLB"
    { 0x0F200000, 0x00010000 }, // "APSS_GIC500_GICD"
    { 0x0F300000, 0x00020000 }, // "APSS_GIC500_GICR"
    { 0x0F520000, 0x00020000 }, // "OSM_RAIL"
    { 0x0F500000, 0x000B0000 }, // "APSS_ACTPM_WRAP"
    { 0x01610000, 0x00010000 }, // "USB2"
};

/**
  Return the Virtual Memory Map of your platform
  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.
  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry
**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_ATTRIBUTES  CacheAttributes;
  ARM_MEMORY_REGION_DESCRIPTOR  *VirtualMemoryTable;
  EFI_RESOURCE_ATTRIBUTE_TYPE   ResourceAttributes;
  UINTN                         Index = 0, Count, ReservedTop;
  EFI_PEI_HOB_POINTERS          NextHob;
  UINT64                        ResourceLength;
  EFI_PHYSICAL_ADDRESS          ResourceTop;

  ResourceAttributes = (
    EFI_RESOURCE_ATTRIBUTE_PRESENT |
    EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
    EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |
    EFI_RESOURCE_ATTRIBUTE_TESTED
  );

  // Create initial Base Hob for system memory.
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    ResourceAttributes,
    PcdGet64 (PcdSystemMemoryBase),
    PcdGet64 (PcdSystemMemorySize)
  );

  NextHob.Raw = GetHobList ();
  Count = sizeof (ginkgoReservedMemoryBuffer) / sizeof (struct ginkgoReservedMemory);
  while ((NextHob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, NextHob.Raw)) != NULL)
  {
    if (Index >= Count)
      break;
    if ((NextHob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) &&
        (ginkgoReservedMemoryBuffer[Index].Offset >= NextHob.ResourceDescriptor->PhysicalStart) &&
        ((ginkgoReservedMemoryBuffer[Index].Offset + ginkgoReservedMemoryBuffer[Index].Size) <=
         NextHob.ResourceDescriptor->PhysicalStart + NextHob.ResourceDescriptor->ResourceLength))
    {
      ResourceAttributes = NextHob.ResourceDescriptor->ResourceAttribute;
      ResourceLength = NextHob.ResourceDescriptor->ResourceLength;
      ResourceTop = NextHob.ResourceDescriptor->PhysicalStart + ResourceLength;
      ReservedTop = ginkgoReservedMemoryBuffer[Index].Offset + ginkgoReservedMemoryBuffer[Index].Size;

      // Create the System Memory HOB for the reserved buffer
      BuildResourceDescriptorHob (
        EFI_RESOURCE_MEMORY_RESERVED,
        EFI_RESOURCE_ATTRIBUTE_PRESENT,
        ginkgoReservedMemoryBuffer[Index].Offset,
        ginkgoReservedMemoryBuffer[Index].Size
      );
      // Update the HOB
      NextHob.ResourceDescriptor->ResourceLength = ginkgoReservedMemoryBuffer[Index].Offset -
                                                   NextHob.ResourceDescriptor->PhysicalStart;

      // If there is some memory available on the top of the reserved memory then create a HOB
      if (ReservedTop < ResourceTop)
      {
        BuildResourceDescriptorHob (EFI_RESOURCE_SYSTEM_MEMORY,
                                    ResourceAttributes,
                                    ReservedTop,
                                    ResourceTop - ReservedTop);
      }
      Index++;
    }
    NextHob.Raw = GET_NEXT_HOB (NextHob);
  }

  ASSERT (VirtualMemoryMap != NULL);

  VirtualMemoryTable = (ARM_MEMORY_REGION_DESCRIPTOR*)AllocatePages (
                                                        EFI_SIZE_TO_PAGES (sizeof(ARM_MEMORY_REGION_DESCRIPTOR) * MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS)
                                                        );
  if (VirtualMemoryTable == NULL) {
    return;
  }

  CacheAttributes = DDR_ATTRIBUTES_CACHED;

  Index = 0;

  // DDR - 4.0GB section
  VirtualMemoryTable[Index].PhysicalBase    = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].VirtualBase     = PcdGet64 (PcdSystemMemoryBase);
  VirtualMemoryTable[Index].Length          = PcdGet64 (PcdSystemMemorySize);
  VirtualMemoryTable[Index].Attributes      = CacheAttributes;

  // SDM665 SOC peripherals
  VirtualMemoryTable[++Index].PhysicalBase  = SDM665_PERIPH_BASE;
  VirtualMemoryTable[Index].VirtualBase     = SDM665_PERIPH_BASE;
  VirtualMemoryTable[Index].Length          = SDM665_PERIPH_SZ;
  VirtualMemoryTable[Index].Attributes      = ARM_MEMORY_REGION_ATTRIBUTE_DEVICE;

  // End of Table
  VirtualMemoryTable[++Index].PhysicalBase  = 0;
  VirtualMemoryTable[Index].VirtualBase     = 0;
  VirtualMemoryTable[Index].Length          = 0;
  VirtualMemoryTable[Index].Attributes      = (ARM_MEMORY_REGION_ATTRIBUTES)0;

  ASSERT((Index + 1) <= MAX_VIRTUAL_MEMORY_MAP_DESCRIPTORS);

  *VirtualMemoryMap = VirtualMemoryTable;
}
