#include "driver.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, event_ctx_cleanup)
#endif

NTSTATUS DriverEntry( PDRIVER_OBJECT object, PUNICODE_STRING reg_path ) {

    dbg_print( "driver_entry\n" );

    WDF_DRIVER_CONFIG     conf;
    WDF_DRIVER_CONFIG_INIT( &conf, event_device_add );

    WDF_OBJECT_ATTRIBUTES attr;
    WDF_OBJECT_ATTRIBUTES_INIT( &attr );

    attr.EvtCleanupCallback = event_ctx_cleanup;

    auto status = WdfDriverCreate( object, reg_path, &attr, &conf, WDF_NO_HANDLE );

    if ( !NT_SUCCESS( status ) ) {
        dbg_print( "driver_entry > WdfDriverCreate() failed 0x%x", status );
    }

    return status;
}

VOID event_ctx_cleanup( WDFOBJECT driver ) {

    _IRQL_limited_to_( PASSIVE_LEVEL );

    UNREFERENCED_PARAMETER( driver );

    PAGED_CODE();

    dbg_print( "event_ctx_cleanup\n" );
}