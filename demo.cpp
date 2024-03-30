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
//#define BUFLEN 2048				//缓冲区的最大长度
//#ifdef DEBUG
//#define ADDR "127.0.0.1"	//用于通信的适配器的地址
//#else
//#define ADDR "192.168.33.30"
//#endif
//
//
//#define PORT 4098				//用于接收传入数据的端口
//#define boardid 0
//#define packetsize 1456   // 为减少时间开销，使用启发式值
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
//    //一个计数器,用于记录当前data中的位置
//    int cnt = 0;
//
//
//    //用于零填充
//    std::vector<char> zeros(packetsize, 0);
//
//    //保存数据的位置（初始化足够长）
//    std::string filename("K:/data_0.bin");
//    std::ofstream output_file;
//    std::remove(filename.c_str());		// 清除以前的数据
//    output_file.open(filename.c_str(), std::ofstream::in | std::ofstream::out | std::ofstream::app | std::ofstream::binary);
//    if (output_file.is_open())
//        printf("板卡 %u: 正在写入文件 %s...\n", boardid, filename.c_str());
//    else
//    {
//        printf("板卡 %u: 打开文件 %s 失败\n", boardid, filename.c_str());
//        exit(EXIT_FAILURE);
//    }
//
//    //初始化 Winsock
//    printf("\n正在初始化 Winsock...");
//    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
//    {
//        printf("失败。错误代码：%d", WSAGetLastError());
//        exit(EXIT_FAILURE);
//    }
//    printf("已初始化。\n");
//
//    //创建套接字
//    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
//    {
//        printf("无法创建套接字：%d", WSAGetLastError());
//    }
//    printf("套接字已创建。\n");
//
//    //准备 sockaddr_in 结构
//    server.sin_family = AF_INET;
//    server.sin_addr.s_addr = inet_addr(ADDR);
//    server.sin_port = htons(PORT);
//
//    //绑定
//    if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
//    {
//        printf("绑定失败，错误代码：%d", WSAGetLastError());
//        exit(EXIT_FAILURE);
//    }
//    puts("绑定完成");
//
//    seqnum_prev = 0;    // 第一个包的序列号为 1
//    bytenum_prev = 0;   // 第一个包的字节数为 0
//
//    //保持监听数据
//    while (1)
//    {
//        if ((recv_len = recvfrom(s, udpbuf, BUFLEN, 0, 0, 0)) == SOCKET_ERROR)
//        {
//            printf("recvfrom() 失败，错误代码：%d", WSAGetLastError());
//            exit(EXIT_FAILURE);
//        }
//
//        //获取序列号和字节数
//        memcpy(&seqnum_new, udpbuf, 4);
//        memcpy(&bytenum_new, udpbuf + 4, 6);
//
//        //获取数据(到data中,不要序列号和字节数)
//        for (int i = 10; i < recv_len; i += 2)
//		{
//			data[cnt++] = (udpbuf[i] << 8) | udpbuf[i + 1];
//		}
//
//
//
//        //printf("数据:\n");
//        //for (int i = 0; i < recv_len; i++)
//        //{
//        //    printf("%02x", (unsigned char)udpbuf[i]);
//        //}
//        //printf("\n");
//
//
//        //如果到达旧的数据包，则丢弃它
//        if (seqnum_new <= seqnum_prev)
//            continue;
//
//        //如果有丢失的数据包，则进行零填充
//        if (seqnum_new - seqnum_prev != 1)
//        {
//            //printf("丢失了数据包!! 新的：%d，旧的：%d，正在进行零填充..\n", seqnum_new, seqnum_prev);
//            //printf("当前缓冲区填充大小为 %d \n", buff_ptrs[0] - buff_begins[0]);
//            //printf("字节新：%d，字节旧：%d，正在填充 %d \n", bytenum_new, bytenum_prev, (int)(bytenum_new - bytenum_prev));
//            //std::fill(buff_ptrs[0], buff_ptrs[0] + (int)(bytenum_new - bytenum_prev), 0);
//
//            for (int i = seqnum_prev + 1; i < seqnum_new; i++)
//            {
//                output_file.write(&*zeros.begin(), packetsize);
//            }
//
//
//            printf("零填充完成。\n");
//        }
//
//
//        //保存此数据包的数据
//        //std::copy(udpbuf, udpbuf + recv_len - 10, buff_ptrs[0]);
//        //memcpy(buff_ptrs[0], udpbuf + 10, recv_len - 10);
//        //output_file.write(udpbuf + 10, recv_len - 10);
//
//        //更新历史记录
//        seqnum_prev = seqnum_new;
//        bytenum_prev = bytenum_new + recv_len - 10;
//        //printf("新的字节预计为 %d，通过 %d + %d \n", bytenum_prev, bytenum_new, recv_len);
//        //printf("当前样本数：%d\n", nsampcum);
//    }
//
//    closesocket(s);
//    WSACleanup();
//
//    return 0;
//}