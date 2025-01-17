/*
 * This file is part of the Aaru Remote Server.
 * Copyright (c) 2019-2025 Natalia Portillo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <windows.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "win32.h"
#include "../aaruremote.h"

void* DeviceOpen(const char* device_path)
{
    DeviceContext* ctx;

    ctx = malloc(sizeof(DeviceContext));

    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(DeviceContext));

    ctx->handle = CreateFile(device_path,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

    if(ctx->handle == INVALID_HANDLE_VALUE)
    {
        free(ctx);
        return NULL;
    }

    strncpy(ctx->device_path, device_path, 4096);

    return ctx;
}

void DeviceClose(void* device_ctx)
{
    DeviceContext* ctx = device_ctx;

    if(!ctx) return;

    CloseHandle(ctx->handle);

    free(ctx);
}

int32_t GetDeviceType(void* device_ctx)
{
    DeviceContext*             ctx = device_ctx;
    STORAGE_PROPERTY_QUERY     query;
    DWORD                      error = 0;
    BOOL                       ret;
    DWORD                      returned;
    PSTORAGE_DEVICE_DESCRIPTOR descriptor;
    char*                      buf;

    if(!ctx) return -1;

    buf = malloc(1000);

    if(!buf) return AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    query.PropertyId = StorageDeviceProperty;
    query.QueryType  = PropertyStandardQuery;

    memset(buf, 0, 1000);

    ret = DeviceIoControl(
        ctx->handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(STORAGE_PROPERTY_QUERY), buf, 1000, &returned, NULL);

    if(!ret) error = GetLastError();

    if(!ret && error != 0)
    {
        free(buf);
        return AARUREMOTE_DEVICE_TYPE_UNKNOWN;
    }

    descriptor = (PSTORAGE_DEVICE_DESCRIPTOR)buf;

    switch(descriptor->BusType)
    {
        case 1: returned = AARUREMOTE_DEVICE_TYPE_SCSI; break;
        case 2: returned = AARUREMOTE_DEVICE_TYPE_ATAPI; break;
        case 3: returned = AARUREMOTE_DEVICE_TYPE_ATA; break;
        case 4: returned = AARUREMOTE_DEVICE_TYPE_SCSI; break;
        case 5: returned = AARUREMOTE_DEVICE_TYPE_SCSI; break;
        case 6: returned = AARUREMOTE_DEVICE_TYPE_SCSI; break;
        case 7: returned = AARUREMOTE_DEVICE_TYPE_SCSI; break;
        case 9: returned = AARUREMOTE_DEVICE_TYPE_SCSI; break;
        case 0xA: returned = AARUREMOTE_DEVICE_TYPE_SCSI; break;
        case 0xB: returned = AARUREMOTE_DEVICE_TYPE_ATA; break;
        case 0xC: returned = AARUREMOTE_DEVICE_TYPE_SECURE_DIGITAL; break;
        case 0xD: returned = AARUREMOTE_DEVICE_TYPE_MMC; break;
        case 0x11: returned = AARUREMOTE_DEVICE_TYPE_NVME; break;
        default: returned = AARUREMOTE_DEVICE_TYPE_UNKNOWN; break;
    }

    free(buf);

    return returned;
}

int32_t ReOpen(void* device_ctx, uint32_t* closeFailed)
{
    DeviceContext* ctx = device_ctx;
    *closeFailed       = 0;

    if(!ctx) return -1;

    CloseHandle(ctx->handle);

    ctx->handle = CreateFile(ctx->device_path,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

    return ctx->handle == INVALID_HANDLE_VALUE ? GetLastError() : 0;
}

int32_t OsRead(void* device_ctx, char* buffer, uint64_t offset, uint32_t length, uint32_t* duration)
{
    DeviceContext* ctx = device_ctx;
    BOOL           ret;
    LARGE_INTEGER  liDistanceToMove;
    DWORD          nNumberOfBytesRead;
    *duration                 = 0;
    liDistanceToMove.QuadPart = offset;

    if(!ctx) return -1;

    // TODO: Timing
    ret = SetFilePointerEx(ctx->handle, liDistanceToMove, NULL, FILE_BEGIN);

    if(!ret) return GetLastError();

    // TODO: Timing
    ret = ReadFile(ctx->handle, buffer, length, &nNumberOfBytesRead, NULL);

    return !ret ? GetLastError() : 0;
}