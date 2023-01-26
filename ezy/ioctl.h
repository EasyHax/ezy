#pragma once

#include "driver.h"

#define IOCTL_LAST_MOUSE_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x840, METHOD_BUFFERED, FILE_ANY_ACCESS)

_IRQL_requires_same_
_IRQL_requires_max_( DISPATCH_LEVEL )
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL io_device_control;

NTSTATUS init_ioctl_queue( WDFDEVICE device );