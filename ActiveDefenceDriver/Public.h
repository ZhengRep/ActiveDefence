/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_ActiveDefenceDriver,
    0x3df7753c,0x3c91,0x41a1,0x81,0x3b,0x0e,0xb5,0x54,0x38,0x43,0xe2);
// {3df7753c-3c91-41a1-813b-0eb5543843e2}
