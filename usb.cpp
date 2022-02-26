// usb.cpp

#ifndef _WIN32
#include <libusb-1.0/libusb.h>
#include <cstring>
#endif

#include "profiler.h"
#include "rpc_error.h"
#include "usb.h"


const char* CUSB::GetErrorMsg(int error)
{
	switch (error)
	{
    case FT_OK:                          return "ok";
    case FT_INVALID_HANDLE:              return "invalid handle";
    case FT_DEVICE_NOT_FOUND:            return "device not found";
    case FT_DEVICE_NOT_OPENED:           return "device not opened";
    case FT_IO_ERROR:                    return "io error";
    case FT_INSUFFICIENT_RESOURCES:      return "insufficient resource";
    case FT_INVALID_PARAMETER:           return "invalid parameter";
    case FT_INVALID_BAUD_RATE:           return "invalid baud rate";
    case FT_DEVICE_NOT_OPENED_FOR_ERASE: return "device not opened for erase";
    case FT_DEVICE_NOT_OPENED_FOR_WRITE: return "device not opened for write";
    case FT_FAILED_TO_WRITE_DEVICE:      return "failed to write device";
    case FT_EEPROM_READ_FAILED:          return "eeprom read failed";
    case FT_EEPROM_WRITE_FAILED:         return "eeprom write failed";
    case FT_EEPROM_ERASE_FAILED:         return "eeprom erase failed";
	case FT_EEPROM_NOT_PRESENT:          return "eeprom not present";
	case FT_EEPROM_NOT_PROGRAMMED:       return "eeprom not programmed";
	case FT_INVALID_ARGS:                return "invalid args";
	case FT_NOT_SUPPORTED:               return "not supported";
	case FT_OTHER_ERROR:                 return "other error";
	}
	return "unknown error";
}


bool CUSB::EnumFirst(unsigned int &nDevices)
{
	ftStatus = FT_ListDevices(&enumCount,
		NULL,FT_LIST_NUMBER_ONLY|FT_OPEN_BY_SERIAL_NUMBER);
	if (ftStatus != FT_OK)
	{
		nDevices = enumCount = enumPos = 0;
		return false;
	}

	nDevices = enumCount;
	enumPos = 0;
	return true;
}


bool CUSB::EnumNext(char name[])
{
	if (enumPos >= enumCount) return false;
	ftStatus = FT_ListDevices((PVOID)enumPos, name, FT_LIST_BY_INDEX);
	if (ftStatus != FT_OK)
	{
		enumCount = enumPos = 0;
		return false;
	}

	enumPos++;
	return true;
}

bool CUSB::Enum(char name[], unsigned int pos)
{
	enumPos=pos;
	if (enumPos >= enumCount) return false;
	ftStatus = FT_ListDevices((PVOID)enumPos, name, FT_LIST_BY_INDEX);
	if (ftStatus != FT_OK)
	{
		enumCount = enumPos = 0;
		return false;
	}

	return true;
}


bool CUSB::Open(char serialNumber[])
{
	if (isUSB_open) { ftStatus = FT_DEVICE_NOT_OPENED; return false; }

	m_posR = m_sizeR = m_posW = 0;

	ftStatus = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);

	if (ftStatus != FT_OK)
#ifdef _WIN32
		return false;
#else
	{
        /* maybe the ftdi_sio and usbserial kernel modules are attached to the device */
        /* try to detach them using the libusb library directly */

        /* prepare libusb structures */
        libusb_device ** list;
        libusb_device_handle *handle;
        struct libusb_device_descriptor descriptor;

        /* initialise libusb and get device list*/
        libusb_init(NULL);
        ssize_t ndevices = libusb_get_device_list(NULL, &list);
        if( ndevices < 0)
          return false;

        char serial [20];

        bool found = false;

        /* loop over all USB devices */
        for( int dev = 0; dev < ndevices; dev++) {
            /* get the device descriptor */
            int ok = libusb_get_device_descriptor(list[dev], &descriptor);
            if( ok != 0)
                continue;
            /* we're only interested in devices with one vendor and two possible product ID */
            if( descriptor.idVendor != 0x0403 && (descriptor.idProduct != 0x6001 || descriptor.idProduct != 0x6014))
                continue;

            /* open the device */
            ok = libusb_open(list[dev], &handle);
            if( ok != 0)
                    continue;
            /* Read the serial number from the device */
            ok = libusb_get_string_descriptor_ascii(handle, descriptor.iSerialNumber, (unsigned char *) serial, 20);
            if( ok < 0)
                continue;

            /* Check the device serial number */
            if( strcmp(serialNumber, serial) == 0) {
                /* that's our device */
                found = true;

                /* Detach the kernel module from the device */
                ok = libusb_detach_kernel_driver(handle, 0);
                if( ok == 0)
                    printf("Detached kernel driver from selected testboard.\n");
                else
                    printf("Unable to detach kernel driver from selected testboard.\n");
                break;
            }

            libusb_close(handle);
        }

        libusb_free_device_list(list, 1);

        /* if the device was not found in the previous loop, don't try again */
        if( !found)
            return false;

        /* try to re-open with the detached device */
        ftStatus = FT_OpenEx(serialNumber, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
        if( ftStatus != FT_OK)
            return false;
    }
#endif


	ftStatus = FT_SetBitMode(ftHandle, 0xFF, 0x40);
	if (ftStatus != FT_OK) return false;

//	FT_SetUSBParameters(ftHandle, 8192, 8192);
//	FT_SetBaudRate(ftHandle, 9600);
	
	FT_SetTimeouts(ftHandle,4000,1000);
	isUSB_open = true;
	return true;
}


void CUSB::Close()
{
	if (!isUSB_open) return;
	FT_Close(ftHandle);
	isUSB_open = 0;
}


void CUSB::Write(const void *buffer, unsigned int bytesToWrite)
{ PROFILING
	if (!isUSB_open) throw CRpcError(CRpcError::WRITE_ERROR);

	DWORD k=0;
	for (k=0; k < bytesToWrite; k++)
	{
		if (m_posW >= USBWRITEBUFFERSIZE) Flush();
		m_bufferW[m_posW++] = ((unsigned char*)buffer)[k];
	}
}


void CUSB::Flush()
{ PROFILING
	DWORD bytesWritten;
	DWORD bytesToWrite = m_posW;
	m_posW = 0;

	if (!isUSB_open) throw CRpcError(CRpcError::WRITE_ERROR);

	if (!bytesToWrite) return;

	ftStatus = FT_Write(ftHandle, m_bufferW, bytesToWrite, &bytesWritten);

	if (ftStatus != FT_OK) throw CRpcError(CRpcError::WRITE_ERROR);
	if (bytesWritten != bytesToWrite) { ftStatus = FT_IO_ERROR; throw CRpcError(CRpcError::WRITE_ERROR); }
}


bool CUSB::FillBuffer(DWORD minBytesToRead)
{ PROFILING
	if (!isUSB_open) return false;

	DWORD bytesAvailable, bytesToRead;

	ftStatus = FT_GetQueueStatus(ftHandle, &bytesAvailable);
	if (ftStatus != FT_OK) return false;

	if (m_posR<m_sizeR) return false;

	bytesToRead = (bytesAvailable>minBytesToRead)? bytesAvailable : minBytesToRead;
	if (bytesToRead>USBREADBUFFERSIZE) bytesToRead = USBREADBUFFERSIZE;

	ftStatus = FT_Read(ftHandle, m_bufferR, bytesToRead, &m_sizeR);
	m_posR = 0;
	if (ftStatus != FT_OK)
	{
		m_sizeR = 0;
		return false;
	}
	return true;
}


void CUSB::Read(void *buffer,  unsigned int bytesToRead)
{ PROFILING
	if (!isUSB_open) throw CRpcError(CRpcError::READ_ERROR);

	DWORD i;

	for (i=0; i<bytesToRead; i++)
	{
		if (m_posR<m_sizeR)
			((unsigned char*)buffer)[i] = m_bufferR[m_posR++];

		else
		{
			DWORD n = bytesToRead-i;
			if (n>USBREADBUFFERSIZE) n = USBREADBUFFERSIZE;

			if (!FillBuffer(n)) throw CRpcError(CRpcError::READ_ERROR);
			if (m_sizeR < n) throw CRpcError(CRpcError::READ_ERROR);

			if (m_posR<m_sizeR)
				((unsigned char*)buffer)[i] = m_bufferR[m_posR++];
			else
			{   // timeout (bytesRead < bytesToRead)
				throw CRpcError(CRpcError::READ_TIMEOUT);
			}
		}
	}
}


void CUSB::Clear()
{ PROFILING
	if (!isUSB_open) return;

	ftStatus = FT_Purge(ftHandle, FT_PURGE_RX|FT_PURGE_TX);
	m_posR = m_sizeR = 0;
	m_posW = 0;
}


/*
CUsbLog::CUsbLog()
{
	f = fopen("usb_log.txt", "wt");
	state = IDLE;
}


CUsbLog::~CUsbLog()
{
	fclose(f);
}


void CUsbLog::Add(unsigned int x)
{
	return;
	switch (state)
	{
	case IDLE:
		if (x == 0xc0) state = CMD_1;
		else if (x == 0xc2)	state = DAT_CNT1;
		else fprintf(f, "WRONG HEADER: %02X\n", x);
		break;
	case CMD_1:
		cmd = x;
		state = CMD_2;
		break;
	case CMD_2:
		cmd += x << 8;
		state = CMD_CNT;
		break;
	case CMD_CNT:
		count = x;
		fprintf(f, "CMD(%u, %u)\n", cmd, count);
		state = count ? DATA : IDLE;
		break;
	case DAT_CNT1:
		count = x;
		state = DAT_CNT2;
		return;
	case DAT_CNT2:
		count += x << 8;
		state = DAT_CNT3;
		return;
	case DAT_CNT3:
		count += x << 16;
		fprintf(f, "DAT(%u)\n", count);
		state = count ? DATA : IDLE;
		return;
	case DATA:
		fprintf(f, " %02X", x);
		count--;
		if (count == 0)
		{
			fprintf(f, "\n");
			state = IDLE;
		}
	}
}
*/
