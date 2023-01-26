#include "memory.h"

NTSTATUS init_global_buffer( VOID ) {

    auto buffer = WDFMEMORY{};
    auto status = WdfMemoryCreate( WDF_NO_OBJECT_ATTRIBUTES, NonPagedPoolNx, 0, sizeof( MOUSE_DATA ), &buffer, NULL ); 

    RtlZeroMemory( WdfMemoryGetBuffer(buffer, NULL), sizeof(MOUSE_DATA));

    if ( !NT_SUCCESS( status ) ) {
        dbg_print( "init_global_buffer > WdfMemoryCreate() failed 0x%x\n", status );
        return status;
    }

    auto spinlock = WDFSPINLOCK{};

    status = WdfSpinLockCreate( WDF_NO_OBJECT_ATTRIBUTES, &spinlock );

    if ( !NT_SUCCESS( status ) ) {
        dbg_print( "init_global_buffer > WdfSpinLockCreate() failed 0x%x\n", status );
        return status;
    }

    g_buffer   = buffer;
    g_spinlk = spinlock;

    return status;
}

VOID write_global_buffer( WDFMEMORY buffer, size_t length ) {

    WdfSpinLockAcquire( g_spinlk );

    WdfMemoryCopyToBuffer( buffer, 0, WdfMemoryGetBuffer( g_buffer, NULL ), length );

    WdfSpinLockRelease( g_spinlk );
}

VOID read_global_buffer( PVOID buffer, size_t length ) {

    WdfSpinLockAcquire( g_spinlk );

    WdfMemoryCopyToBuffer( g_buffer, 0, buffer, length );

    WdfSpinLockRelease( g_spinlk );
}

VOID free_global_buffer( VOID ) {

    WdfSpinLockRelease( g_spinlk );

    WdfObjectDelete( g_buffer );
}