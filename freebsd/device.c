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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../aaruremote.h"
#include "freebsd.h"

void *DeviceOpen(const char *device_path)
{
    DeviceContext *ctx;

    ctx = malloc(sizeof(DeviceContext));

    if(!ctx) return NULL;

    memset(ctx, 0, sizeof(DeviceContext));

    strncpy(ctx->device_path, device_path, 4096);

    ctx->device = cam_open_device(ctx->device_path, O_RDWR);

    if(!ctx->device)
    {
        free(ctx);
        return NULL;
    }

    return ctx;
}

void DeviceClose(void *device_ctx)
{
    DeviceContext *ctx = device_ctx;

    if(!ctx) return;

    if(ctx->device) cam_close_device(ctx->device);

    free(ctx);
}

int32_t GetDeviceType(void *device_ctx)
{
    DeviceContext *ctx = device_ctx;
    union ccb     *camccb;
    int            ret;
    int32_t        device_type = AARUREMOTE_DEVICE_TYPE_UNKNOWN;

    if(!ctx) return -1;
    if(!ctx->device) return -1;

    camccb = cam_getccb(ctx->device);

    if(!camccb) return device_type;

    camccb->ccb_h.func_code = XPT_GDEV_TYPE;

    ret = cam_send_ccb(ctx->device, camccb);

    if(ret < 0)
    {
        cam_freeccb(camccb);
        return device_type;
    }

    switch(camccb->cgd.protocol)
    {
        case PROTO_ATA:
        case PROTO_SATAPM:
            device_type = AARUREMOTE_DEVICE_TYPE_ATA;
            break;
        case PROTO_ATAPI:
            device_type = AARUREMOTE_DEVICE_TYPE_ATAPI;
            break;
        case PROTO_SCSI:
            device_type = AARUREMOTE_DEVICE_TYPE_SCSI;
            break;
        case PROTO_NVME:
            device_type = AARUREMOTE_DEVICE_TYPE_NVME;
            break;
        case PROTO_MMCSD:
            // TODO: MMC vs SD
            device_type = AARUREMOTE_DEVICE_TYPE_MMC;
            break;
        default:
            device_type = AARUREMOTE_DEVICE_TYPE_UNKNOWN;
            break;
    }

    cam_freeccb(camccb);
    return device_type;
}

int32_t ReOpen(void *device_ctx, uint32_t *closeFailed)
{
    DeviceContext *ctx = device_ctx;
    int            ret;
    *closeFailed = 0;

    if(!ctx) return -1;

    cam_close_device(ctx->device);

    ctx->device = cam_open_device(ctx->device_path, O_RDWR);

    return ctx->device == 0 ? errno : 0;
}

int32_t OsRead(void *device_ctx, char *buffer, uint64_t offset, uint32_t length, uint32_t *duration)
{
    DeviceContext *ctx = device_ctx;
    ssize_t        ret;
    *duration = 0;
    off_t pos;

    if(!ctx) return -1;

    // TODO: Timing
    pos = lseek(ctx->device.fd, (off_t)offset, SEEK_SET);

    if(pos < 0) return errno;

    // TODO: Timing
    ret = read(ctx->device.fd, (void *)buffer, (size_t)length);

    return ret < 0 ? errno : 0;
}