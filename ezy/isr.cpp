#include "driver.h"
//#include "memory.h"

extern VOID write_global_buffer( WDFMEMORY buffer, size_t length );

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS init_usb_isr_reader(PDEVICE_CONTEXT context)
{
    dbg_print("init_usb_isr_reader\n");

    auto pipe = WDFUSBPIPE{};

    for ( BYTE i = 0; i < WdfUsbInterfaceGetNumConfiguredPipes(context->usb_interface); ++i ) {

        auto info = WDF_USB_PIPE_INFORMATION{};
        WDF_USB_PIPE_INFORMATION_INIT(&info);
        
        pipe = WdfUsbInterfaceGetConfiguredPipe(context->usb_interface, i, &info);

        dbg_print("init_usb_isr_reader > pipe handle: %p, endpoint addr: %x, type %d\n", pipe, (int)info.EndpointAddress, (int)info.PipeType);

        if ( info.PipeType == WdfUsbPipeTypeInterrupt ) { break; }
        
        pipe = nullptr;
    }

    if ( !pipe ) {
        dbg_print("init_usb_isr_reader > no interrupt pipe found\n");
        return STATUS_UNSUCCESSFUL;
    }

    WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(pipe);

    auto conf = WDF_USB_CONTINUOUS_READER_CONFIG{};
    WDF_USB_CONTINUOUS_READER_CONFIG_INIT(&conf, event_usb_isr_read_complete, context, 1024);
    conf.EvtUsbTargetPipeReadersFailed = event_usb_isr_read_error;

    auto status = WdfUsbTargetPipeConfigContinuousReader(pipe, &conf);

    if ( !NT_SUCCESS(status) ) {
        dbg_print("init_usb_isr_reader > WdfUsbTargetPipeConfigContinuousReader() failed 0x%x\n", status);
    }

    return status;
}

VOID event_usb_isr_read_complete(WDFUSBPIPE pipe, WDFMEMORY buffer, size_t length, WDFCONTEXT context)
{
    UNREFERENCED_PARAMETER(  pipe   );
    UNREFERENCED_PARAMETER( context );

    dbg_print("event_usb_isr_read_complete\n");

    write_global_buffer( buffer, length );
}

BOOLEAN event_usb_isr_read_error(WDFUSBPIPE pipe, NTSTATUS status, USBD_STATUS usbd_status) {

    UNREFERENCED_PARAMETER(pipe);

    dbg_print("event_usb_isr_read_error - status 0x%x, usbd_status 0x%x\n", status, usbd_status);

    return TRUE;
}


