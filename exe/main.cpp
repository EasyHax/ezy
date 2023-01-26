#include <windows.h>
#include <iostream>
#include <memory>
#include <setupapi.h>

#define IOCTL_LAST_MOUSE_DATA CTL_CODE(FILE_DEVICE_UNKNOWN, 0x840, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma comment(lib, "setupapi.lib")

const GUID GUID_DEVINTERFACE_EZY = { 0x573e8c73, 0xcb4, 0x4471, 0xa1, 0xbf, 0xfa, 0xb2, 0x6c, 0x31, 0xd3, 0x85 };

struct mouse_data {

    uint8_t left_click : 1;
    uint8_t right_click : 1;
    uint8_t scroll_wheel : 1;
    uint8_t page_up : 1;
    uint8_t page_dw : 1;
    uint8_t unused_1 : 3;

    uint8_t unknown[3];

    uint16_t deltaX;
    uint16_t deltaY;
};

HANDLE get_driver_handle( void )
{
    auto devinfo   = HDEVINFO{};
    auto devdata   = SP_DEVICE_INTERFACE_DATA{ sizeof( SP_DEVICE_INTERFACE_DATA ) };

    if ( (devinfo = SetupDiGetClassDevs( &GUID_DEVINTERFACE_EZY, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE ) ) == INVALID_HANDLE_VALUE ) {
        printf( "SetupDiGetClassDevs() failed 0x%x\n", GetLastError() );
        return INVALID_HANDLE_VALUE;
    }

    if ( !SetupDiEnumDeviceInterfaces( devinfo, NULL, &GUID_DEVINTERFACE_EZY, 0, &devdata ) ) {
        printf( "SetupDiEnumDeviceInterfaces failed 0x%x\n", GetLastError() );
        SetupDiDestroyDeviceInfoList( devinfo );
        return INVALID_HANDLE_VALUE;
    }

    DWORD req_size = 0;

    if ( !SetupDiGetDeviceInterfaceDetail( devinfo, &devdata, NULL, 0, &req_size, NULL ) ) {
        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
            printf( "SetupDiGetDeviceInterfaceDetail failed 0x%x\n", GetLastError() );
            SetupDiDestroyDeviceInfoList( devinfo );
            return INVALID_HANDLE_VALUE;
        }
    }

    auto deleter = []( SP_DEVICE_INTERFACE_DETAIL_DATA* p ) { delete[] reinterpret_cast<char*>( p ); };

    auto devdetail = std::unique_ptr<SP_DEVICE_INTERFACE_DETAIL_DATA, decltype( deleter )>( 
        reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>( new char[req_size] ), deleter 
    );

    devdetail->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );

    if ( !SetupDiGetDeviceInterfaceDetail( devinfo, &devdata, devdetail.get(), req_size, &req_size, nullptr) ) {
        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {
            printf( "SetupDiGetDeviceInterfaceDetail failed 0x%x\n", GetLastError() );
            SetupDiDestroyDeviceInfoList( devinfo );
            return INVALID_HANDLE_VALUE;
        }
    }

    SetupDiDestroyDeviceInfoList( devinfo );

    if ( !devdetail ) {
        printf( "No interface found\n" );
        return INVALID_HANDLE_VALUE;
    }

    printf( "found interface [%ls]\n", devdetail->DevicePath );

    if ( HANDLE handle = CreateFile( devdetail->DevicePath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr ) ) {
        return handle;
    }

    printf("CreateFile() failed 0x%x\n", GetLastError() );

    return INVALID_HANDLE_VALUE;
}

int main(void) {

	std::cout << "main()" << std::endl;

    auto handle = get_driver_handle();

	if ( handle == INVALID_HANDLE_VALUE ) {
		std::cout << "[!] error loading driver" << std::endl;
		system("pause");
		return -1;
	}

	std::cout << "[+] driver loaded" << std::endl;

    while ( !(GetAsyncKeyState('X') & 0x8000) ) {

        mouse_data mouse_data = { 0 };

        if ( !DeviceIoControl( handle, IOCTL_LAST_MOUSE_DATA, nullptr, NULL, (LPVOID)&mouse_data, sizeof( mouse_data ), nullptr, nullptr ) ) {
            printf( "DeviceIoControl() failed 0x%x", GetLastError() );
            continue;
        }

        std::cout << "X:" << mouse_data.deltaX << " Y: " << mouse_data.deltaY 
                << " LC: " << (uint16_t)mouse_data.left_click
                << " RC: " << (uint16_t)mouse_data.right_click
                << " PU: " << (uint16_t)mouse_data.page_dw
                << " PD: " << (uint16_t)mouse_data.page_up
                << " SW: " << (uint16_t)mouse_data.scroll_wheel
            << std::endl;


        Sleep( 100 );
    }

	if ( !CloseHandle(handle) ) {
		std::cout << "[!] error unloading driver" << std::endl;
		system("pause");
		return -1;
	}

	std::cout << "[+] driver unloaded" << std::endl;

	system("pause");
	return 0;
}