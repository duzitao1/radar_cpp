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
//#include <thread>
//#include <mutex>
//
//#include "model_predictor.hpp"
//#include "mmwave_handle.hpp"
//
////#define DEBUG
//
//#define BUFLEN 2048				//����������󳤶�
//#ifdef DEBUG
//    #define ADDR "127.0.0.1"	//����ͨ�ŵ��������ĵ�ַ
//#else
//    #define ADDR "192.168.33.30"
//#endif
//
//
//#define PORT 4098				//���ڽ��մ������ݵĶ˿�
//#define boardid 0
//#define packetsize 1456   // Ϊ����ʱ�俪����ʹ������ʽֵ
//
//// �����������ڱ�����������
//std::mutex mtx;
//
//// ���ݽ��պ���
//void receive_data(SOCKET s, std::vector<short>& t_data, std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>>& data_radar, int& cnt)
//{
//    char udpbuf[BUFLEN];
//    int recv_len;
//    while (1)
//    {
//        if ((recv_len = recvfrom(s, udpbuf, BUFLEN, 0, 0, 0)) == SOCKET_ERROR)
//        {
//            printf("recvfrom() ʧ�ܣ�������룺%d", WSAGetLastError());
//            exit(EXIT_FAILURE);
//        }
//        // ȡ��ǰ4���ֽڣ����к�
//        int seqnum_new, bytenum_new;
//        memcpy(&seqnum_new, udpbuf, 4);
//        memcpy(&bytenum_new, udpbuf + 4, 6);
//        //std::cout << "seqnum_new: " << seqnum_new << " bytenum_new: " << bytenum_new << std::endl;
//
//        // ��ȡ����
//        for (int i = 10; i < recv_len; i += 2)
//        {
//            mtx.lock();  // ����������
//            if (cnt == t_data.size())
//			{
//				mtx.unlock();  // ����������
//				continue;
//			}
//            t_data[cnt] = (udpbuf[i] & 0xff) | ((udpbuf[i + 1] & 0xff) << 8);
//            cnt++;
//
//            mtx.unlock();  // ����������
//        }
//    }
//}
//
//// ģ��������
//void inference_model(Ort::Session& session, std::vector<std::string>& input_names, std::vector<std::vector<float>>& input_tensor_values_list, std::vector<std::int64_t>& input_shapes, std::vector<std::string>& output_names, std::vector<short>& t_data, std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>>& data_radar, int& cnt)
//{
//    std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar_local;
//    std::vector<std::vector<float>> input_tensor_values_list_local;
//    // ��¼��ǰ��Ԥ�����
//    float p_cnt = 0;
//    // Ԥ��ɹ�����
//    float cnt_success = 0;
//    // ׼ȷ��
//    float accuracy = 0;
//    float* output_data;
//    double max_value = 0;
//    int max_index = 0;
//
//
//    while (1)
//    {
//        if (cnt == t_data.size())
//        {
//            std::cout << cnt << std::endl;
//            mtx.lock();  // ����������
//            data_radar_local = mmwave_handle::reshape_data(t_data);
//
//            // ������ȡ
//            input_tensor_values_list_local = mmwave_handle::feature_extraction(data_radar_local, "avg");
//            mtx.unlock();  // ����������
//
//            // ��������
//            output_data = run_model_with_input(session, input_names, input_tensor_values_list_local, input_shapes, output_names);
//            mtx.lock();  // ����������
//            cnt = 0;
//            mtx.unlock();  // ����������
//
//            // �ҵ����ֵ������
//            max_index = 0;
//            max_value = 0;
//
//            for (int i = 0; i < 10; i++)
//            {
//                if (output_data[i] > max_value)
//                {
//					max_value = output_data[i];
//					max_index = i;
//				}
//			}
//            // �����1,˵����ȷ
//            if (max_index == 1)
//            {
//				cnt_success++;
//			}
//            p_cnt++;
//            std::cout << "׼ȷ��: " << cnt_success / p_cnt << std::endl;
//
//
//            std::cout<<"max_index: "<<max_index<<std::endl;
//            std::cout << "ģ���������" << std::endl;
//
//        }
//    }
//
//}
//
//
//int main()
//{
//    //-----------------------------------------
//    //              ��ʼ�� Winsock
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
//    //һ��������,���ڼ�¼��ǰdata�е�λ��
//    int cnt = 0;
//    int zeros_cnt = 0;
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
//    //-----------------------------------------
//    //              ��ʼ��ģ��
//    //-----------------------------------------
//
//    // ����ģ���ļ�·��
//    std::basic_string<ORTCHAR_T> model_file = L"K:/aio_radar/avg_model.onnx";
//
//    // onnxruntime ����
//    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
//    Ort::SessionOptions session_options;
//    Ort::Session session = Ort::Session(env, model_file.c_str(), session_options);
//
//    // ��ȡ���������ڵ�����/��״
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
//     // ����data_radar
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
//    //               ��ʼ����
//    //-----------------------------------------
//
//    // �������ݽ����߳�
//    std::thread recv_thread(receive_data, s, std::ref(t_data), std::ref(data_radar), std::ref(cnt));
//
//    // ����ģ�������߳�
//    std::thread infer_thread(inference_model, std::ref(session), std::ref(input_names), std::ref(input_tensor_values_list), std::ref(input_shapes), std::ref(output_names), std::ref(t_data), std::ref(data_radar), std::ref(cnt));
//
//    // �ȴ��߳̽���
//    recv_thread.join();
//    infer_thread.join();
//
//    //-----------------------------------------
//    //              �ر� Winsock
//    //-----------------------------------------
//
//    closesocket(s);
//    WSACleanup();
//
//
//
//
//
//    return 0;
//}