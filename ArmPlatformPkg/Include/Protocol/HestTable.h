/** @file
  Builds and installs the HEST ACPI table.

  Define the protocol interface that allows HEST ACPI table to be created,
  populated with error record descriptions and installation of the HEST ACPI
  table.

  Copyright (c) 2020 - 2021, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef HEST_TABLE_H_
#define HEST_TABLE_H_

#define HEST_TABLE_PROTOCOL_GUID \
  { \
    0x705bdcd9, 0x8c47, 0x457e, \
    { 0xad, 0x0d, 0xf7, 0x86, 0xf3, 0x4a, 0x0d, 0x63 } \
  }

/**
  Append HEST error source descriptor protocol service.

  Protocol service used to append newly collected error source descriptors to
  to an already created HEST table.

  @param[in]  ErrorSourceDescriptorList      List of Error Source Descriptors.
  @param[in]  ErrorSourceDescriptorListSize  Total Size of Error Source
                                             Descriptors.
  @param[in]  ErrorSourceDescriptorCount     Total count of error source
                                             descriptors.

  @retval  EFI_SUCCESS            Appending the error source descriptors
                                  successful.
  @retval  EFI_OUT_OF_RESOURCES   Buffer reallocation failed for the Hest
                                  table.
  @retval  EFI_INVALID_PARAMETER  Null ErrorSourceDescriptorList param or
**/
typedef
EFI_STATUS
(EFIAPI *APPEND_ERROR_SOURCE_DESCRIPTOR) (
  IN CONST VOID *ErrorSourceDescriptorList,
  IN UINTN      ErrorSourceDescriptorListSize,
  IN UINTN      ErrorSourceDescriptorCount
  );

/**
  Install HEST table protocol service.

  Installs the HEST table that has been appended with the error source
  descriptors. The checksum of this table is calculated and updated in
  the table header. The HEST table is then installed.

  @retval  EFI_SUCCESS  HEST table is installed successfully.
  @retval  EFI_ABORTED  HEST table is NULL.
  @retval  Other        Install service call failed.
**/
typedef
EFI_STATUS
(EFIAPI *INSTALL_HEST_TABLE) (VOID);

//
// HEST_TABLE_PROTOCOL enables creation and installation of HEST table
//
typedef struct {
  APPEND_ERROR_SOURCE_DESCRIPTOR AppendErrorSourceDescriptors;
  INSTALL_HEST_TABLE             InstallHestTable;
} HEST_TABLE_PROTOCOL;

extern EFI_GUID gHestTableProtocolGuid;
#endif  // HEST_TABLE_H_
