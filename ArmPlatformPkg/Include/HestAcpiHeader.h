/** @file
  HEST table helper macros.

  Macro definitions to initialize the HEST ACPI table specific structures.

  Copyright (c) 2021, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Specification Reference:
    - ACPI Reference Specification 6.3
    - UEFI Reference Specification 2.8
**/

#ifndef HEST_ACPI_HEADER_
#define HEST_ACPI_HEADER_

#include <IndustryStandard/Acpi.h>

//
// HEST table GHESv2 type related structures.
//
// Helper Macro to initialize the HEST GHESv2 Notification Structure.
// Refer Table 18-394 in ACPI Specification, Version 6.3.
#define EFI_ACPI_6_3_HARDWARE_ERROR_NOTIFICATION_STRUCTURE_INIT(Type,         \
  PollInterval, EventId)                                                      \
  {                                                                           \
    Type,                                                                     \
    sizeof (EFI_ACPI_6_3_HARDWARE_ERROR_NOTIFICATION_STRUCTURE),              \
    {0, 0, 0, 0, 0, 0, 0}, /* ConfigurationWriteEnable */                     \
    PollInterval,                                                             \
    EventId,                                                                  \
    0,                    /* Poll Interval Threshold Value  */                \
    0,                    /* Poll Interval Threshold Window */                \
    0,                    /* Error Threshold Value          */                \
    0                     /* Error Threshold Window         */                \
  }

// Helper Macro to initialize the HEST GHESv2 Error Status Structure.
// Refer Section 5.2.3.2 in ACPI Specification, Version 6.3.
#define EFI_ACPI_6_3_GENERIC_ERROR_STATUS_STRUCTURE_INIT(Address)     \
  {                                                                   \
    0,        /* UINT8 Address Space ID */                            \
    64,       /* Register Bit Width     */                            \
    0,        /* Register Bit Offset    */                            \
    4,        /* Access Size            */                            \
    Address   /* CPER/Read Ack Addr     */                            \
  }

#endif /* HEST_ACPI_HEADER_ */
