/*
 * This file is part of the DiscImageChef Remote Server.
 * Copyright (c) 2019 Natalia Portillo.
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

#include "dicmote.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include "linux/linux.h"
#endif

#include <stddef.h>
#include <stdlib.h>

DeviceInfoList* ListDevices()
{
#if defined(__linux__) && !defined(__ANDROID__)
    return linux_list_devices();
#else
    return NULL;
#endif
}

void FreeDeviceInfoList(DeviceInfoList* start)
{
    DeviceInfoList* current;

    while(start)
    {
        current = start;
        start   = current->next;
        free(current);
    }
}