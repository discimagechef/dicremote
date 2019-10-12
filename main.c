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

#include <errno.h>
#include <ifaddrs.h>
#include <libnet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

int main()
{
    struct ifaddrs*    ifa;
    struct ifaddrs*    ifa_start;
    int                ret;
    char               ipv4Address[INET_ADDRSTRLEN];
    int                sockfd, cli_sock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t          cli_len;
    struct utsname     utsname;
    DicPacketHello*    pkt_server_hello;

    printf("DiscImageChef Remote Server %s\n", DICMOTE_VERSION);
    printf("Copyright (C) 2019 Natalia Portillo\n");

    ret = uname(&utsname);

    if(ret)
    {
        printf("Error %d getting system version.\n", errno);
        return 1;
    }

    printf("Running under %s %s (%s).\n", utsname.sysname, utsname.release, utsname.machine);

    printf("Opening socket.\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        printf("Error %d opening socket.\n", errno);
        return 1;
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(DICMOTE_PORT);

    if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Error %d binding socket.\n", errno);
        close(sockfd);
        return 1;
    }

    ret = getifaddrs(&ifa);

    if(ret)
    {
        printf("Error %d enumerating interfaces\n", errno);
        return 1;
    }

    ifa_start = ifa;

    printf("Available addresses:\n");
    while(ifa != NULL)
    {
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr, ipv4Address, INET_ADDRSTRLEN);
            printf("%s port %d\n", ipv4Address, DICMOTE_PORT);
        }

        ifa = ifa->ifa_next;
    }

    freeifaddrs(ifa_start);

    ret = listen(sockfd, 1);

    if(ret)
    {
        printf("Error %d listening.\n", errno);
        close(sockfd);
        return 1;
    }

    printf("\n");
    printf("Waiting for a client...\n");

    cli_len  = sizeof(cli_addr);
    cli_sock = accept(sockfd, (struct sockaddr*)&cli_addr, &cli_len);

    if(cli_sock < 0)
    {
        printf("Error %d accepting incoming connection.\n", errno);
        close(sockfd);
        return 1;
    }

    inet_ntop(AF_INET, &cli_addr.sin_addr, ipv4Address, INET_ADDRSTRLEN);
    printf("Client %s connected successfully.\n", ipv4Address);

    pkt_server_hello = malloc(sizeof(DicPacketHello));

    if(!pkt_server_hello)
    {
        printf("Fatal error %d allocating memory.\n", errno);
        close(cli_sock);
        close(sockfd);
        return 1;
    }

    memset(pkt_server_hello, 0, sizeof(DicPacketHello));

    strncpy(pkt_server_hello->hdr.id, DICMOTE_PACKET_ID, sizeof(DICMOTE_PACKET_ID));
    pkt_server_hello->hdr.len         = sizeof(DicPacketHello);
    pkt_server_hello->hdr.version     = DICMOTE_PACKET_VERSION;
    pkt_server_hello->hdr.packet_type = DICMOTE_PACKET_TYPE_HELLO;
    strncpy(pkt_server_hello->application, DICMOTE_NAME, sizeof(DICMOTE_NAME));
    strncpy(pkt_server_hello->version, DICMOTE_VERSION, sizeof(DICMOTE_VERSION));
    pkt_server_hello->max_protocol = DICMOTE_PROTOCOL_MAX;
    strncpy(pkt_server_hello->sysname, utsname.sysname, 255);
    strncpy(pkt_server_hello->release, utsname.release, 255);
    strncpy(pkt_server_hello->machine, utsname.machine, 255);

    write(cli_sock, pkt_server_hello, sizeof(DicPacketHello));

    printf("Closing socket.\n");
    close(cli_sock);
    close(sockfd);
    free(pkt_server_hello);

    return 0;
}