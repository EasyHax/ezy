#pragma once

#include "driver.h"

volatile WDFMEMORY   g_buffer;
volatile WDFSPINLOCK g_spinlk;

NTSTATUS init_global_buffer( VOID );

VOID write_global_buffer( WDFMEMORY buffer, size_t length );

VOID read_global_buffer( PVOID buffer, size_t length );

VOID free_global_buffer( VOID );