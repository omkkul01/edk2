/** @file
  MM HEST error source gateway driver.

  This MM driver installs a handler which can be used to retrieve the error
  source descriptors from the all MM drivers implementing the HEST error source
  descriptor protocol.

  The MM driver acts as a single point of contact to collect secure hardware
  error sources from the MM drivers. It loops over all the MM drivers that
  implement HEST error source descriptor protocol and collects error source
  descriptor information along with the error source count and length.

  Copyright (c) 2020 - 2021, ARM Limited. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Protocol/HestErrorSourceInfo.h>

#include "HestMmErrorSourceCommon.h"

STATIC EFI_MM_SYSTEM_TABLE *mMmst = NULL;

/**
  Returns an array of handles that implement the HEST error source descriptor
  protocol.

  Passing HandleBuffer as NULL will return the actual size of the buffer
  required to hold the array of handles implementing the protocol.

  @param[in, out]  HandleBufferSize  The size of the HandleBuffer.
  @param[out]      HandleBuffer      A pointer to the buffer containing the list
                                    of handles.

  @retval  EFI_SUCCESS    The array of handles returned in HandleBuffer.
  @retval  EFI_NOT_FOUND  No implementation present for the protocol.
  @retval  Other          For any other error.
**/
STATIC
EFI_STATUS
GetHestErrorSourceProtocolHandles (
  IN OUT UINTN      *HandleBufferSize,
  OUT    EFI_HANDLE **HandleBuffer
  )
{
  EFI_STATUS Status;

  Status = mMmst->MmLocateHandle (
                    ByProtocol,
                    &gMmHestErrorSourceDescProtocolGuid,
                    NULL,
                    HandleBufferSize,
                    *HandleBuffer
                    );
  if ((EFI_ERROR (Status)) &&
      (Status != EFI_BUFFER_TOO_SMALL))
  {
    DEBUG ((
      DEBUG_ERROR,
      "%a: No implementation of MmHestErrorSourceDescProtocol found, \
       Status:%r\n",
      __FUNCTION__,
      Status
      ));
    return EFI_NOT_FOUND;
  }

  return Status;
}

/**
  Mmi handler to retrieve HEST error source descriptor information.

  Handler for Mmi service that returns the supported HEST error source
  descriptors in MM. This handler populates the CommBuffer with the
  list of all error source descriptors, prepended with the length and
  the number of descriptors populated into CommBuffer.

  @param[in]       DispatchHandle  The unique handle assigned to this handler by
                                   MmiHandlerRegister().
  @param[in]       Context         Points to an optional handler context that
                                   is specified when the handler was registered.
  @param[in, out]  CommBuffer      Buffer used for communication of HEST error
                                   source descriptors.
  @param[in, out]  CommBufferSize  The size of the CommBuffer.

  @retval  EFI_SUCCESS            CommBuffer has valid data.
  @retval  EFI_BAD_BUFFER_SIZE    CommBufferSize not adequate.
  @retval  EFI_OUT_OF_RESOURCES   System out of memory resources.
  @retval  EFI_INVALID_PARAMETER  Invalid CommBufferSize recieved.
  @retval  Other                  For any other error.
**/
STATIC
EFI_STATUS
EFIAPI
HestErrorSourcesInfoMmiHandler (
  IN     EFI_HANDLE DispatchHandle,
  IN     CONST VOID *Context,       OPTIONAL
  IN OUT VOID       *CommBuffer,    OPTIONAL
  IN     UINTN      *CommBufferSize OPTIONAL
  )
{
  MM_HEST_ERROR_SOURCE_DESC_PROTOCOL *HestErrSourceDescProtocolHandle;
  HEST_ERROR_SOURCE_DESC_INFO        *ErrorSourceInfoList;
  EFI_HANDLE                         *HandleBuffer;
  EFI_STATUS                         Status;
  UINTN                              HandleCount;
  UINTN                              HandleBufferSize;
  UINTN                              Index;
  UINTN                              SourceCount = 0;
  UINTN                              SourceLength = 0;
  VOID                               *ErrorSourcePtr;
  UINTN                              TotalSourceLength = 0;
  UINTN                              TotalSourceCount = 0;

  if (*CommBufferSize < HEST_ERROR_SOURCE_DESC_INFO_SIZE) {
    //
    // Ensures that the communication buffer has enough space to atleast hold
    // the ErrSourceDescCount and ErrSourceDescSize elements of the
    // HEST_ERROR_SOURCE_DESC_INFO structure.
    //
    DEBUG ((
      DEBUG_ERROR,
      "%a: Invalid CommBufferSize parameter\n",
      __FUNCTION__
      ));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get all handles that implement the HEST error source descriptor protocol.
  // Get the buffer size required to store list of handles for the protocol.
  //
  HandleBuffer = NULL;
  HandleBufferSize = 0;
  Status = GetHestErrorSourceProtocolHandles (&HandleBufferSize, &HandleBuffer);
  if ((Status == EFI_NOT_FOUND) ||
      (HandleBufferSize == 0))
  {
    return Status;
  }

  // Allocate memory for HandleBuffer of size HandleBufferSize.
  HandleBuffer = AllocateZeroPool (HandleBufferSize);
  if (HandleBuffer == NULL) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Failed to allocate memory for HandleBuffer\n",
      __FUNCTION__
      ));
    return EFI_OUT_OF_RESOURCES;
  }

  // Get the list of handles.
  Status = GetHestErrorSourceProtocolHandles (&HandleBufferSize, &HandleBuffer);
  if ((EFI_ERROR (Status)) ||
      (HandleBuffer == NULL))
  {
    FreePool (HandleBuffer);
    return Status;
  }

  // Count of handles for the protocol.
  HandleCount = HandleBufferSize / sizeof (EFI_HANDLE);

  //
  // Loop to get the count and length of the error source descriptors.
  //
  // This loop collects and adds the length of error source descriptors and
  // its count from all the the MM drivers implementing HEST error source.
  // descriptor protocol. The total length and count values retrieved help
  // to determine weather the CommBuffer is big enough to hold the descriptor
  // information.
  // As mentioned in the HEST error source descriptor protocol definition,
  // Buffer parameter set to NULL ensures only length and the count values
  // are returned from the driver and no error source information is copied to
  // Buffer.
  //
  for (Index = 0; Index < HandleCount; ++Index) {
    Status = mMmst->MmHandleProtocol (
                      HandleBuffer[Index],
                      &gMmHestErrorSourceDescProtocolGuid,
                      (VOID **)&HestErrSourceDescProtocolHandle
                      );
    if (EFI_ERROR (Status)) {
      continue;
    }

    //
    // Protocol called with Buffer parameter passed as NULL, must return
    // error source length and error count for that driver.
    //
    Status = HestErrSourceDescProtocolHandle->GetHestErrorSourceDescriptors (
                                                HestErrSourceDescProtocolHandle,
                                                NULL,
                                                &SourceLength,
                                                &SourceCount
                                                );
    if (Status == EFI_INVALID_PARAMETER) {
      TotalSourceLength += SourceLength;
      TotalSourceCount += SourceCount;
    }
  }

  // Set the count and length in the error source descriptor.
  ErrorSourceInfoList = (HEST_ERROR_SOURCE_DESC_INFO *)(CommBuffer);
  ErrorSourceInfoList->ErrSourceDescCount = TotalSourceCount;
  ErrorSourceInfoList->ErrSourceDescSize = TotalSourceLength;

  //
  // Check the size of CommBuffer, it should atleast be of size
  // TotalSourceLength + HEST_ERROR_SOURCE_DESC_INFO_SIZE.
  //
  TotalSourceLength = TotalSourceLength + HEST_ERROR_SOURCE_DESC_INFO_SIZE;
  if ((*CommBufferSize) < TotalSourceLength) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Invalid CommBufferSize parameter\n",
      __FUNCTION__
      ));
    FreePool (HandleBuffer);
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // CommBuffer size is adequate to return all the error source descriptors.
  // So go ahead and populate it with the error source descriptor information.
  //

  // Buffer pointer to append the Error Descriptors data.
  ErrorSourcePtr =  ErrorSourceInfoList->ErrSourceDescList;

  //
  // Loop to retrieve error source descriptors information.
  //
  // Calls into each MM driver that implement the HEST error source descriptor
  // protocol. Here the Buffer parameter passed to the protocol service is
  // valid. So the MM driver when called copies the descriptor information.
  //
  for (Index = 0; Index < HandleCount; ++Index) {
    Status = mMmst->MmHandleProtocol (
                      HandleBuffer[Index],
                      &gMmHestErrorSourceDescProtocolGuid,
                      (VOID **)&HestErrSourceDescProtocolHandle
                      );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = HestErrSourceDescProtocolHandle->GetHestErrorSourceDescriptors (
                                                HestErrSourceDescProtocolHandle,
                                                (VOID **)&ErrorSourcePtr,
                                                &SourceLength,
                                                &SourceCount
                                                );
    if (Status == EFI_SUCCESS) {
      ErrorSourcePtr += SourceLength;
    }
  }

  // Free the buffer holding all the protocol handles.
  FreePool (HandleBuffer);

  return Status;
}

/**
  Entry point for this Stanalone MM driver.

  Registers an Mmi handler that retrieves the error source descriptors from all
  the MM drivers implementing the MM_HEST_ERROR_SOURCE_DESC_PROTOCOL.

  @param[in]  ImageHandle  The firmware allocated handle for the EFI image.
  @param[in]  SystemTable  A pointer to the EFI System Table.

  @retval  EFI_SUCCESS  The entry point registered handler successfully.
  @retval  Other        Some error occurred when executing this entry point.
**/
EFI_STATUS
EFIAPI
StandaloneMmHestErrorSourceInitialize (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_MM_SYSTEM_TABLE *SystemTable
  )
{
  EFI_HANDLE DispatchHandle;
  EFI_STATUS Status;

  ASSERT (SystemTable != NULL);
  mMmst = SystemTable;

  Status = mMmst->MmiHandlerRegister (
                    HestErrorSourcesInfoMmiHandler,
                    &gMmHestGetErrorSourceInfoGuid,
                    &DispatchHandle
                    );
  if (EFI_ERROR(Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "%a: Mmi handler registration failed with status : %r\n",
      __FUNCTION__,
      Status
      ));
    return Status;
  }

  return Status;
}
