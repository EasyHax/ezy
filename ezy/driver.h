#pragma once

#include <initguid.h>
#include <ntddk.h>
#include "usbdi.h"
#include "usbdlib.h"
#include "driverspecs.h"
#include <wdf.h>
#include <wdfusb.h>

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

DECLARE_CONST_UNICODE_STRING( interface_name, L"ezy_driver_interface" );

DEFINE_GUID( GUID_DEVINTERFACE_EZY, 0x573e8c73, 0xcb4, 0x4471, 0xa1, 0xbf, 0xfa, 0xb2, 0x6c, 0x31, 0xd3, 0x85 );

#define dbg_print(...) DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL, __VA_ARGS__);

typedef struct _DEVICE_CONTEXT {

    WDFUSBDEVICE    usb_device;
    WDFUSBINTERFACE usb_interface;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

typedef struct _MOUSE_DATA {

    CHAR buttons;
    CHAR unknown[3];

    USHORT deltaX;
    USHORT deltaY;

} MOUSE_DATA, *PMOUSE_DATA;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_OBJECT_CONTEXT_CLEANUP  event_ctx_cleanup;
EVT_WDF_DRIVER_DEVICE_ADD       event_device_add;
EVT_WDF_DEVICE_PREPARE_HARDWARE event_prepare_hardware;

EVT_WDF_REQUEST_COMPLETION_ROUTINE event_request_complete;

EVT_WDF_USB_READER_COMPLETION_ROUTINE event_usb_isr_read_complete;
EVT_WDF_USB_READERS_FAILED            event_usb_isr_read_error;

EVT_WDF_DEVICE_D0_ENTRY event_d0_entry;
EVT_WDF_DEVICE_D0_EXIT  event_d0_exit;

_IRQL_requires_(PASSIVE_LEVEL) NTSTATUS init_usb_isr_reader(PDEVICE_CONTEXT context);

__drv_dispatchType( IRP_MJ_CREATE )
__drv_dispatchType( IRP_MJ_CLOSE )
__drv_dispatchType( IRP_MJ_DEVICE_CONTROL )
DRIVER_DISPATCH driver_dispatch;

EXTERN_C_END