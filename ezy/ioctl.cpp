#include "ioctl.h"
//#include "memory.h"

extern VOID read_global_buffer( PVOID buffer, size_t length );

VOID io_device_control( WDFQUEUE queue, WDFREQUEST request, size_t out_buffer_length, size_t in_buffer_length, ULONG ioctl_code ) {

    UNREFERENCED_PARAMETER(  out_buffer_length  );
    UNREFERENCED_PARAMETER(   in_buffer_length  );
    UNREFERENCED_PARAMETER(        queue        );

    dbg_print( "io_device_control " );

    if ( ioctl_code == IOCTL_LAST_MOUSE_DATA ) {

        dbg_print( "code: IOCTL_LAST_MOUSE_DATA\n" );

        auto buffer = PCHAR{};
        auto buffer_size = size_t{};

        auto status = WdfRequestRetrieveOutputBuffer( request, sizeof( MOUSE_DATA ), (PVOID*)&buffer, &buffer_size);

        if ( !NT_SUCCESS( status ) ) {
            dbg_print( "io_device_control > WdfRequestRetrieveOutputBuffer() failed 0x%x\n", status );
        }

        read_global_buffer( buffer, buffer_size );

        dbg_print( "wrote to buffer: " );
        for ( BYTE i = 0; i < buffer_size; ++i ) {
            dbg_print( "%d ", buffer[i] );
        } 
        dbg_print( "\n" );

        WdfRequestCompleteWithInformation( request, status, buffer_size );
    }
}

NTSTATUS init_ioctl_queue( WDFDEVICE device ) {

    auto conf  = WDF_IO_QUEUE_CONFIG{};
    auto queue = WDFQUEUE{};

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE( &conf, WdfIoQueueDispatchParallel );

    conf.EvtIoDeviceControl = io_device_control;

    auto status = WdfIoQueueCreate( device, &conf, WDF_NO_OBJECT_ATTRIBUTES, &queue );

    if ( !NT_SUCCESS( status ) ) {
        dbg_print( "init_ioctl_queue > WdfIoQueueCreate() failed 0x%x\n", status );
    }

    return status;
}