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

#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

#ifndef _WIN32
#include <stdint.h>
#endif

#include "../aaruremote.h"
#include "../endian.h"

AaruPacketHello *GetHello()
{
    struct utsname   utsname;
    int              ret;
    AaruPacketHello *pkt_server_hello;

    ret = uname(&utsname);

    if(ret) { return 0; }

    pkt_server_hello = malloc(sizeof(AaruPacketHello));

    if(!pkt_server_hello) { return 0; }

    memset(pkt_server_hello, 0, sizeof(AaruPacketHello));

    pkt_server_hello->hdr.remote_id   = htole32(AARUREMOTE_REMOTE_ID);
    pkt_server_hello->hdr.packet_id   = htole32(AARUREMOTE_PACKET_ID);
    pkt_server_hello->hdr.len         = htole32(sizeof(AaruPacketHello));
    pkt_server_hello->hdr.version     = AARUREMOTE_PACKET_VERSION;
    pkt_server_hello->hdr.packet_type = AARUREMOTE_PACKET_TYPE_HELLO;
    strncpy(pkt_server_hello->application, AARUREMOTE_NAME, sizeof(AARUREMOTE_NAME));
    strncpy(pkt_server_hello->version, AARUREMOTE_VERSION, sizeof(AARUREMOTE_VERSION));
    pkt_server_hello->max_protocol = AARUREMOTE_PROTOCOL_MAX;
    strncpy(pkt_server_hello->sysname, utsname.sysname, 255);
    strncpy(pkt_server_hello->release, utsname.release, 255);
    strncpy(pkt_server_hello->machine, utsname.machine, 255);

    return pkt_server_hello;
}