//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
//
//#include<cstdio>
//#include<winsock2.h>
//#include<fstream>
////#include<iostream>
//#include<vector>
//#include<complex>
//#include<synchapi.h>
//#include<io.h>
//#include<iostream>
//#include<vector>
//#include<array>
//
//#pragma comment(lib,"ws2_32.lib") //Winsock Library
//
////#define DEBUG
//
//#define BUFLEN 2048				//����������󳤶�
//#ifdef DEBUG
//#define ADDR "127.0.0.1"	//����ͨ�ŵ��������ĵ�ַ
//#else
//#define ADDR "192.168.33.30"
//#endif
//
//
//#define PORT 4098				//���ڽ��մ������ݵĶ˿�
//#define boardid 0
//#define packetsize 1456   // Ϊ����ʱ�俪����ʹ������ʽֵ
//
//
//int main()
//{
//    SOCKET s;
//    struct sockaddr_in server;
//    int recv_len, seqnum_prev, seqnum_new;
//    int bytenum_prev, bytenum_new;
//    char udpbuf[BUFLEN];
//    WSADATA wsa;
//    //std::vector<short> data;
//
//    std::array<short, 3916800> data;
//
//    //һ��������,���ڼ�¼��ǰdata�е�λ��
//    int cnt = 0;
//
//
//    //���������
//    std::vector<char> zeros(packetsize, 0);
//
//    //�������ݵ�λ�ã���ʼ���㹻����
//    std::string filename("K:/data_0.bin");
//    std::ofstream output_file;
//    std::remove(filename.c_str());		// �����ǰ������
//    output_file.open(filename.c_str(), std::ofstream::in | std::ofstream::out | std::ofstream::app | std::ofstream::binary);
//    if (output_file.is_open())
//        printf("�忨 %u: ����д���ļ� %s...\n", boardid, filename.c_str());
//    else
//    {
//        printf("�忨 %u: ���ļ� %s ʧ��\n", boardid, filename.c_str());
//        exit(EXIT_FAILURE);
//    }
//
//    //��ʼ�� Winsock
//    printf("\n���ڳ�ʼ�� Winsock...");
//    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
//    {
//        printf("ʧ�ܡ�������룺%d", WSAGetLastError());
//        exit(EXIT_FAILURE);
//    }
//    printf("�ѳ�ʼ����\n");
//
//    //�����׽���
//    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
//    {
//        printf("�޷������׽��֣�%d", WSAGetLastError());
//    }
//    printf("�׽����Ѵ�����\n");
//
//    //׼�� sockaddr_in �ṹ
//    server.sin_family = AF_INET;
//    server.sin_addr.s_addr = inet_addr(ADDR);
//    server.sin_port = htons(PORT);
//
//    //��
//    if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
//    {
//        printf("��ʧ�ܣ�������룺%d", WSAGetLastError());
//        exit(EXIT_FAILURE);
//    }
//    puts("�����");
//
//    seqnum_prev = 0;    // ��һ���������к�Ϊ 1
//    bytenum_prev = 0;   // ��һ�������ֽ���Ϊ 0
//
//    //���ּ�������
//    while (1)
//    {
//        if ((recv_len = recvfrom(s, udpbuf, BUFLEN, 0, 0, 0)) == SOCKET_ERROR)
//        {
//            printf("recvfrom() ʧ�ܣ�������룺%d", WSAGetLastError());
//            exit(EXIT_FAILURE);
//        }
//
//        //��ȡ���кź��ֽ���
//        memcpy(&seqnum_new, udpbuf, 4);
//        memcpy(&bytenum_new, udpbuf + 4, 6);
//
//        //��ȡ����(��data��,��Ҫ���кź��ֽ���)
//        for (int i = 10; i < recv_len; i += 2)
//		{
//			data[cnt++] = (udpbuf[i] << 8) | udpbuf[i + 1];
//		}
//
//
//
//        //printf("����:\n");
//        //for (int i = 0; i < recv_len; i++)
//        //{
//        //    printf("%02x", (unsigned char)udpbuf[i]);
//        //}
//        //printf("\n");
//
//
//        //�������ɵ����ݰ���������
//        if (seqnum_new <= seqnum_prev)
//            continue;
//
//        //����ж�ʧ�����ݰ�������������
//        if (seqnum_new - seqnum_prev != 1)
//        {
//            //printf("��ʧ�����ݰ�!! �µģ�%d���ɵģ�%d�����ڽ��������..\n", seqnum_new, seqnum_prev);
//            //printf("��ǰ����������СΪ %d \n", buff_ptrs[0] - buff_begins[0]);
//            //printf("�ֽ��£�%d���ֽھɣ�%d��������� %d \n", bytenum_new, bytenum_prev, (int)(bytenum_new - bytenum_prev));
//            //std::fill(buff_ptrs[0], buff_ptrs[0] + (int)(bytenum_new - bytenum_prev), 0);
//
//            for (int i = seqnum_prev + 1; i < seqnum_new; i++)
//            {
//                output_file.write(&*zeros.begin(), packetsize);
//            }
//
//
//            printf("�������ɡ�\n");
//        }
//
//
//        //��������ݰ�������
//        //std::copy(udpbuf, udpbuf + recv_len - 10, buff_ptrs[0]);
//        //memcpy(buff_ptrs[0], udpbuf + 10, recv_len - 10);
//        //output_file.write(udpbuf + 10, recv_len - 10);
//
//        //������ʷ��¼
//        seqnum_prev = seqnum_new;
//        bytenum_prev = bytenum_new + recv_len - 10;
//        //printf("�µ��ֽ�Ԥ��Ϊ %d��ͨ�� %d + %d \n", bytenum_prev, bytenum_new, recv_len);
//        //printf("��ǰ��������%d\n", nsampcum);
//    }
//
//    closesocket(s);
//    WSACleanup();
//
//    return 0;
//}