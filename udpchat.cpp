#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#define IP_TARGET "127.0.0.1"

BOOL bEND = FALSE;

SOCKET MakeSocket(WORD wPort) {
    SOCKET sock = INVALID_SOCKET;
    SOCKADDR_IN Addr = {0};

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        return INVALID_SOCKET;
    }
    Addr.sin_family = AF_INET;
    Addr.sin_port = htons(wPort);
    Addr.sin_addr.s_addr = inet_addr(IP_TARGET);

    if (bind(sock, (SOCKADDR *)&Addr, sizeof(Addr)) == SOCKET_ERROR) {
        closesocket(sock);
        return INVALID_SOCKET;
    }
    return sock;
}

BOOL SendData(SOCKET sock, WORD wDstPort) {
    SOCKADDR_IN SendAddr = {0};
    char buf[1024];

    SendAddr.sin_family = AF_INET;
    SendAddr.sin_port = htons(wDstPort);
    SendAddr.sin_addr.s_addr = inet_addr(IP_TARGET);

    printf("Enter Message: ");
    fgets(buf, 1024, stdin);

    if (buf[0] == 'q')
        return FALSE;

    sendto(sock, buf, strlen(buf), 0, (SOCKADDR*)&SendAddr, sizeof(SendAddr));
    return TRUE;    
}

DWORD WINAPI RecvThread(LPVOID pParam) {
    SOCKET sock = (SOCKET)pParam;
    SOCKADDR_IN RecvAddr = {0};
    int iRet, iRecvSize;
    char buf[1024];

    while (1) {
        iRecvSize = sizeof(RecvAddr);
        iRet = recvfrom(sock, buf, 1024, 0, (SOCKADDR*)&RecvAddr, &iRecvSize);

        if (iRet == SOCKET_ERROR) {
            continue;
        }
        buf[iRet] = '\0';  // Correct null-termination
        printf("[%s:%d]: %s\n", inet_ntoa(RecvAddr.sin_addr), ntohs(RecvAddr.sin_port), buf);
    }
}

int main(int argc, char** argv) {
    WSADATA WSAData = {0};
    SOCKET sock;
    WORD wSrcPort, wDstPort;

    if (argc != 3) {
        printf("Usage: udpchat [srcport] [dstport]\n");
        return -1;
    }

    wSrcPort = (WORD)atoi(argv[1]);
    wDstPort = (WORD)atoi(argv[2]);

    WSAStartup(MAKEWORD(2, 2), &WSAData);
    sock = MakeSocket(wSrcPort);

    if (sock != INVALID_SOCKET) {
        HANDLE hThread = CreateThread(NULL, 0, RecvThread, (PVOID)sock, 0, NULL);
        while (1) {
            if (!SendData(sock, wDstPort)) {
                break;
            }
        }
        bEND = TRUE;
        closesocket(sock);

        if (WaitForSingleObject(hThread, 3000) == WAIT_TIMEOUT) {
            printf("Error: Thread End\n");
            TerminateThread(hThread, 0);
        }
    }

    WSACleanup();
    return 0;
}