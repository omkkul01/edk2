/** @file
  MM protocol to get the secure error source descriptor information.

  MM Drivers must implement this protocol in order to publish secure side
  error source descriptor information to OSPM through the HEST ACPI table.

  Copyright (c) 2020 - 2021, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef MM_HEST_ERROR_SOURCE_DESC_
#define MM_HEST_ERROR_SOURCE_DESC_

#define MM_HEST_ERROR_SOURCE_DESC_PROTOCOL_GUID \
  { \
    0x560bf236, 0xa4a8, 0x4d69, { 0xbc, 0xf6, 0xc2, 0x97, 0x24, 0x10, 0x9d, 0x91 } \
  }

typedef struct MM_HEST_ERROR_SOURCE_DESC_PROTOCOL_
                 MM_HEST_ERROR_SOURCE_DESC_PROTOCOL;

/**
  Get HEST Secure Error Source Descriptors.

  The MM drivers implementing this protocol must convey the total count and
  total length of the error sources the driver has along with the actual error
  source descriptor(s).

  Passing NULL as Buffer parameter shall return EFI_INVALID_PARAMETR with the
  total length and count of the error source descriptor(s) it supports.

  @param[in]   This                MM_HEST_ERROR_SOURCE_DESC_PROTOCOL instance.
  @param[out]  Buffer              Buffer to be appended with the error
                                   source descriptors information.
  @param[out]  ErrorSourcesLength  Total length of all the error source
                                   descriptors.
  @param[out]  ErrorSourceCount    Count of total error source descriptors
                                   supported by the driver.

  retval  EFI_SUCCESS           If the Buffer is valid and is filled with valid
                                Error Source descriptor data.
  retval  EFI_INVALID_PARAMTER  Buffer is NULL.
  retval  Other                 If no error source descriptor information is
                                available.
**/
typedef
EFI_STATUS
(EFIAPI *MM_HEST_GET_ERROR_SOURCE_DESCRIPTORS) (
  IN  MM_HEST_ERROR_SOURCE_DESC_PROTOCOL *This,
  OUT VOID                               **Buffer,
  OUT UINTN                              *ErrorSourcesLength,
  OUT UINTN                              *ErrorSourcesCount
  );

//
// Protocol declaration
//
struct MM_HEST_ERROR_SOURCE_DESC_PROTOCOL_ {
  MM_HEST_GET_ERROR_SOURCE_DESCRIPTORS GetHestErrorSourceDescriptors;
};

extern EFI_GUID gMmHestErrorSourceDescProtocolGuid;

#endif // MM_HEST_ERROR_SOURCE_DESC_
