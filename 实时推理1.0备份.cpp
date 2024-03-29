//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
//
//#include<cstdio>
//#include<winsock2.h>
//#include<fstream>
//#include<vector>
//#include<complex>
//#include<synchapi.h>
//#include<io.h>
//#include<iostream>
//#include<vector>
//#include<array>
//#include <onnxruntime_cxx_api.h>
//#pragma comment(lib,"ws2_32.lib") //Winsock Library
//
//#include "model_predictor.hpp"
//#include "mmwave_handle.hpp"
//
//#define DEBUGk
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
//    //-----------------------------------------
//    //              初始化 Winsock
//    //-----------------------------------------
//
//
//    SOCKET s;
//    struct sockaddr_in server;
//    int recv_len, seqnum_prev, seqnum_new;
//    int bytenum_prev, bytenum_new;
//    char udpbuf[BUFLEN];
//    WSADATA wsa;
//    //std::vector<short> data;
//    std::vector<short> t_data(3916800);
//
//
//    //一个计数器,用于记录当前data中的位置
//    int cnt = 0;
//    int zeros_cnt = 0;
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
//    //-----------------------------------------
//    //              初始化模型
//    //-----------------------------------------
//
//    // 设置模型文件路径
//    std::basic_string<ORTCHAR_T> model_file = L"K:/aio_radar/model.onnx";
//
//    // onnxruntime 设置
//    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
//    Ort::SessionOptions session_options;
//    Ort::Session session = Ort::Session(env, model_file.c_str(), session_options);
//
//    // 获取输入和输出节点名称/形状
//    Ort::AllocatorWithDefaultOptions allocator;
//    std::vector<std::string> input_names;
//    std::vector<std::int64_t> input_shapes;
//    for (std::size_t i = 0; i < session.GetInputCount(); i++) {
//        input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
//        input_shapes = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
//    }
//    for (auto& s : input_shapes) {
//        if (s < 0) {
//            s = 1;
//        }
//    }
//
//    std::vector<std::string> output_names;
//    for (std::size_t i = 0; i < session.GetOutputCount(); i++) {
//        output_names.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
//        auto output_shapes = session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
//    }
//
//    // 创建data_radar
//    std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar(
//        mmwave_handle::n_RX, std::vector<std::vector<std::vector<mmwave_handle::Complex>>>(
//            mmwave_handle::n_samples, std::vector<std::vector<mmwave_handle::Complex>>(
//                mmwave_handle::n_chirps, std::vector<mmwave_handle::Complex>(mmwave_handle::n_frames)
//            )
//        )
//    );
//
//    std::vector<std::vector<float>> input_tensor_values_list;
//
//    //-----------------------------------------
//    //               开始监听
//    //-----------------------------------------
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
//
//
//
//
//        //获取序列号和字节数
//        memcpy(&seqnum_new, udpbuf, 4);
//        memcpy(&bytenum_new, udpbuf + 4, 6);
//
//        //获取数据(到data中,不要序列号和字节数)
//        for (int i = 10; i < recv_len; i += 2)
//        {
//            t_data[cnt++] = (udpbuf[i] << 8) | udpbuf[i + 1];
//
//            // 如果data已经满了，就进行推理
//            if (cnt == t_data.size())
//            {
//                std::cout << t_data.size() << std::endl;
//                data_radar = mmwave_handle::reshape_data(t_data);
//                input_tensor_values_list = mmwave_handle::feature_extraction(data_radar, "avg");
//
//                run_model_with_input(session, input_names, input_tensor_values_list, input_shapes, output_names);
//                cnt = 0;
//            }
//        }
//
//        //如果到达旧的数据包，则丢弃它
//        if (seqnum_new <= seqnum_prev)
//            continue;
//
//        // TODO : 如果有丢失的数据包，则进行零填充
//        if (seqnum_new - seqnum_prev != 1)
//        {
//            zeros_cnt += seqnum_new - seqnum_prev - 1;
//
//            for (int i = seqnum_prev + 1; i < seqnum_new; i++)
//            {
//                // 在data中写入零填充数据
//
//            }
//            std::cout << std::endl << "丢失数据包" << zeros_cnt << std::endl;
//        }
//
//        //更新历史记录
//        seqnum_prev = seqnum_new;
//        bytenum_prev = bytenum_new + recv_len - 10;
//    }
//
//    closesocket(s);
//    WSACleanup();
//
//    return 0;
//}
