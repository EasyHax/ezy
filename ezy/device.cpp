#include "driver.h"
#include "ioctl.h"

extern NTSTATUS init_global_buffer( VOID );
extern VOID free_global_buffer( VOID );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, event_device_add)
#pragma alloc_text(PAGE, event_prepare_hardware)
#pragma alloc_text(PAGE, event_d0_entry)
#endif

NTSTATUS event_device_add(WDFDRIVER driver, PWDFDEVICE_INIT device_init) {

    UNREFERENCED_PARAMETER(driver);
    PAGED_CODE();

    dbg_print("event_device_add\n");

    auto pnp_callbacks = WDF_PNPPOWER_EVENT_CALLBACKS{};
    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnp_callbacks);
    pnp_callbacks.EvtDevicePrepareHardware = event_prepare_hardware;
    pnp_callbacks.EvtDeviceD0Entry         = event_d0_entry;
    pnp_callbacks.EvtDeviceD0Exit          = event_d0_exit;

    WdfDeviceInitSetPnpPowerEventCallbacks(device_init, &pnp_callbacks);

    auto io_conf = WDF_IO_TYPE_CONFIG{};
    WDF_IO_TYPE_CONFIG_INIT( &io_conf );
    io_conf.DeviceControlIoType = WdfDeviceIoDirect;

    WdfDeviceInitSetIoTypeEx( device_init, &io_conf );

    auto attr   = WDF_OBJECT_ATTRIBUTES{};
    auto device = WDFDEVICE{};
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attr, DEVICE_CONTEXT);

    auto status = WdfDeviceCreate(&device_init, &attr, &device);

    if ( !NT_SUCCESS(status) ) {
        dbg_print("event_device_add > WdfDeviceCreate() failed 0x%x\n", status);
        return status;
    }

    status = WdfDeviceCreateDeviceInterface( device, (LPGUID)&GUID_DEVINTERFACE_EZY, &interface_name );

    if ( !NT_SUCCESS( status ) ) {
        dbg_print( "event_device_add > WdfDeviceCreateDeviceInterface() failed 0x%x\n", status );
    }

    return init_ioctl_queue( device );
}

NTSTATUS event_prepare_hardware(WDFDEVICE device, WDFCMRESLIST resource_list, WDFCMRESLIST resource_list_translated) {

    UNREFERENCED_PARAMETER(resource_list);
    UNREFERENCED_PARAMETER(resource_list_translated);
    PAGED_CODE();

    dbg_print("event_prepare_hardware\n");


    auto status  = STATUS_UNSUCCESSFUL;
    auto context = GetDeviceContext(device);

    if ( context->usb_device == NULL ) {

        auto conf = WDF_USB_DEVICE_CREATE_CONFIG{};
        WDF_USB_DEVICE_CREATE_CONFIG_INIT(&conf, USBD_CLIENT_CONTRACT_VERSION_602);

        status = WdfUsbTargetDeviceCreateWithParameters(device, &conf, WDF_NO_OBJECT_ATTRIBUTES, &context->usb_device);

        if ( !NT_SUCCESS(status) ) {
            dbg_print("event_prepare_hardware > WdfUsbTargetDeviceCreateWithParameters() failed 0x%x\n", status);
            return status;
        }
    }

    auto conf = WDF_USB_DEVICE_SELECT_CONFIG_PARAMS{};

    WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_SINGLE_INTERFACE(&conf);

    status = WdfUsbTargetDeviceSelectConfig(context->usb_device, WDF_NO_OBJECT_ATTRIBUTES, &conf);

    if ( !NT_SUCCESS(status) ) {
        dbg_print("WdfUsbTargetDeviceSelectConfig() failed 0x%x\n", status);
        return status;
    }

    context->usb_interface = conf.Types.SingleInterface.ConfiguredUsbInterface;

    status = init_usb_isr_reader(context);

    return status;
}


NTSTATUS event_d0_entry(WDFDEVICE device, WDF_POWER_DEVICE_STATE old_state) {

    dbg_print("event_d0_entry\n");

    UNREFERENCED_PARAMETER(old_state);

    auto status = init_global_buffer();

    if ( !NT_SUCCESS( status ) ) {
        dbg_print( "event_d0_entry > init_global_buffer() failed 0x%x\n", status )
    }

    auto context = GetDeviceContext(device);

    status  = WdfIoTargetStart(WdfUsbTargetDeviceGetIoTarget(context->usb_device));

    if ( !NT_SUCCESS(status) ) {
        dbg_print("event_d0_entry > WdfIoTargetStart() failed 0x%x\n", status)
    }

    return status;
}


NTSTATUS event_d0_exit(WDFDEVICE device, WDF_POWER_DEVICE_STATE next_state) {

    UNREFERENCED_PARAMETER(next_state);

    PAGED_CODE();

    dbg_print("event_d0_exit\n");

    auto context = GetDeviceContext(device);

    WdfIoTargetStop(WdfUsbTargetDeviceGetIoTarget(context->usb_device), WdfIoTargetCancelSentIo);

    free_global_buffer();

    return STATUS_SUCCESS;
}

