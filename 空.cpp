#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include<cstdio>
#include<winsock2.h>
#include<fstream>
#include<vector>
#include<complex>
#include<synchapi.h>
#include<io.h>
#include<iostream>
#include<vector>
#include<array>
#include <onnxruntime_cxx_api.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include <thread>
#include <mutex>

#include "model_predictor.hpp"
#include "mmwave_handle.hpp"
#include "ThreadSafeQueue.hpp"

#define DEBUG

#define BUFLEN 1476			//����������󳤶�
#ifdef DEBUG
#define ADDR "127.0.0.1"	//����ͨ�ŵ��������ĵ�ַ
#else
#define ADDR "192.168.33.30"
#endif


#define PORT 4098				//���ڽ��մ������ݵĶ˿�
#define boardid 0
#define packetsize 1456   // Ϊ����ʱ�俪����ʹ������ʽֵ

//ThreadSafeQueue<short*> data_queue; // ���ݶ���
ThreadSafeQueue<std::vector<short>> data_queue; // ���ݶ���

// cnt�Ļ�����
std::mutex cnt_mtx;
// t_data�Ļ�����
std::mutex t_data_mtx;
// data�Ļ�����
std::mutex data_mtx;


// ���ݽ��պ���
void receive_data(SOCKET s, std::vector<short>& t_data, std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>>& data_radar, int& cnt)
{
    char udpbuf[BUFLEN];
    int recv_len;
    int seqnum_new, bytenum_new;
    int seqmum_old = 0;
    int bytenum_old = 0;
    // ��������
    int lost_cnt = 0;
    // t_data.size()����
    int t_data_size = t_data.size();

    // ���ڴ洢����
    //short *data = new short[3916800];

    std::vector<short> data(3916800);
    
    while (1)
    {
        if ((recv_len = recvfrom(s, udpbuf, BUFLEN, 0, 0, 0)) == SOCKET_ERROR)
        {
            printf("recvfrom() ʧ�ܣ�������룺%d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        // ȡ��ǰ4���ֽڣ����к�
        memcpy(&seqnum_new, udpbuf, 4);
        memcpy(&bytenum_new, udpbuf + 4, 6);

        cnt_mtx.lock();  // ����������
        data_mtx.lock();  // ����������
        

  //      if (cnt + recv_len - 10 < 3916800)
  //      {
		//	// �����ݴ���data
		//	memcpy(data + cnt, udpbuf + 10, recv_len - 10);
		//	cnt += recv_len - 10;
		//}
  //      else
  //      {
		//	// �����ݴ���data
		//	memcpy(data + cnt, udpbuf + 10, 3916800 - cnt);
  //          short* data_temp;
  //          // ���Է����ڴ�
  //          try {
  //              data_temp = new short[3916800];
		//		memcpy(data_temp, data, 3916800);
		//		data_queue.enqueue(data_temp);
  //          }
  //          catch (std::bad_alloc& e) {
		//		std::cerr << "bad_alloc caught: " << e.what() << '\n';
		//	}
  //          data_queue.enqueue(data_temp);
  //          // ����data
  //          memcpy(data, udpbuf + 10 + 3916800 - cnt, recv_len - 10 - 3916800 + cnt);
  //          cnt = recv_len - 10 - 3916800 + cnt;
		//}

        if (cnt + recv_len - 10 < 3916800)
        {
            memcpy(data.data() + cnt, udpbuf + 10, recv_len - 10);
            cnt += recv_len - 10;
        }
        else
        {
            memcpy(data.data() + cnt, udpbuf + 10, 3916800 - cnt);

            // �����ݸ��Ƶ��µ�vector�����
            std::vector<short> data_temp(data.begin(), data.end());
            data_queue.enqueue(data_temp);

            // ����data
            memcpy(data.data(), udpbuf + 10 + 3916800 - cnt, recv_len - 10 - 3916800 + cnt);
            cnt = recv_len - 10 - 3916800 + cnt;

            // ������д�С
            //std::cout << "���д�С: " << data_queue.size() << std::endl;

        }


        data_mtx.unlock();  // ����������
        cnt_mtx.unlock();  // ����������
    }
}

// ģ��������
void inference_model(Ort::Session& session, std::vector<std::string>& input_names, std::vector<std::vector<float>>& input_tensor_values_list, std::vector<std::int64_t>& input_shapes, std::vector<std::string>& output_names, std::vector<short>& t_data, std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>>& data_radar, int& cnt)
{
    std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar_local;
    std::vector<std::vector<float>> input_tensor_values_list_local;
    // ��¼��ǰ��Ԥ�����
    float p_cnt = 0;
    // Ԥ��ɹ�����
    float cnt_success = 0;
    // ׼ȷ��
    float accuracy = 0;
    float* output_data;
    double max_value = 0;
    int max_index = 0;


    while (1)
    {
        //if (cnt == 3916800)
        //{
        //    t_data_mtx.lock();
        //    data_radar_local = mmwave_handle::reshape_data(t_data);

        //    // ������ȡ
        //    input_tensor_values_list_local = mmwave_handle::feature_extraction(data_radar_local, "avg");
        //    t_data_mtx.unlock();

        //    // ��������
        //    output_data = run_model_with_input(session, input_names, input_tensor_values_list_local, input_shapes, output_names);

        //    // ��������
        //    //plt::ion();
        //    //mmwave_handle::plot_features(input_tensor_values_list_local);
        //    //plt::pause(0.2);

        //    cnt_mtx.lock();  // ����������
        //    cnt = 0;
        //    cnt_mtx.unlock();
        //    // �ҵ����ֵ������
        //    max_index = 0;
        //    max_value = 0;

        //    for (int i = 0; i < 10; i++)
        //    {
        //        if (output_data[i] > max_value)
        //        {
        //            max_value = output_data[i];
        //            max_index = i;
        //        }
        //    }
        //    // �����1,˵����ȷ
        //    if (max_index == 1)
        //    {
        //        cnt_success++;
        //    }
        //    p_cnt++;
        //    std::cout << "׼ȷ��: " << cnt_success / p_cnt << std::endl;
        //    //std::cout << "max_index: " << max_index << std::endl;

        //}

        // �Ӷ�����ȡ������
        if (!data_queue.empty())
        {
			std::vector<short> data_temp;
			data_queue.try_dequeue(data_temp);

			data_radar_local = mmwave_handle::reshape_data(data_temp);

			// ������ȡ
			input_tensor_values_list_local = mmwave_handle::feature_extraction(data_radar_local, "avg");

			// ��������
			output_data = run_model_with_input(session, input_names, input_tensor_values_list_local, input_shapes, output_names);

			// ��������
			//plt::ion();
			//mmwave_handle::plot_features(input_tensor_values_list_local);
			//plt::pause(0.2);

			// �ҵ����ֵ������
			max_index = 0;
			max_value = 0;

            for (int i = 0; i < 10; i++)
            {
                if (output_data[i] > max_value)
                {
					max_value = output_data[i];
					max_index = i;
				}
			}
			// �����1,˵����ȷ
            if (max_index == 1)
            {
				cnt_success++;
			}
			p_cnt++;
			std::cout << "׼ȷ��: " << static_cast<float>(cnt_success) / p_cnt << std::endl;
			//std::cout << "max_index: " << max_index << std::endl;
        }

    }

}


int main()
{
    //-----------------------------------------
    //              ��ʼ������
    //-----------------------------------------


    std::vector<short> t_data(3916800);
    int recv_len, seqnum_prev, seqnum_new;
    int bytenum_prev = 0;
    int bytenum_new = 0;
    seqnum_prev = 0;    // ��һ���������к�Ϊ 1

    //һ��������,���ڼ�¼��ǰdata�е�λ��
    int cnt = 0;
    //-----------------------------------------
    //              ��ʼ�� Winsock
    //-----------------------------------------

    SOCKET s;
    struct sockaddr_in server;
    char udpbuf[BUFLEN];
    WSADATA wsa;

    //��ʼ�� Winsock
    printf("\n���ڳ�ʼ�� Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("ʧ�ܡ�������룺%d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("�ѳ�ʼ����\n");

    //�����׽���
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("�޷������׽��֣�%d", WSAGetLastError());
    }
    printf("�׽����Ѵ�����\n");

    //׼�� sockaddr_in �ṹ
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ADDR);
    server.sin_port = htons(PORT);

    //��
    if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("��ʧ�ܣ�������룺%d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("�����");

    //-----------------------------------------
    //              ��ʼ��ģ��
    //-----------------------------------------

    // ����ģ���ļ�·��
    std::basic_string<ORTCHAR_T> model_file = L"K:/aio_radar/avg_model.onnx";

    // onnxruntime ����
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
    Ort::SessionOptions session_options;
    Ort::Session session = Ort::Session(env, model_file.c_str(), session_options);

    // ��ȡ���������ڵ�����/��״
    Ort::AllocatorWithDefaultOptions allocator;
    std::vector<std::string> input_names;
    std::vector<std::int64_t> input_shapes;
    for (std::size_t i = 0; i < session.GetInputCount(); i++) {
        input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
        input_shapes = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
    }
    for (auto& s : input_shapes) {
        if (s < 0) {
            s = 1;
        }
    }

    std::vector<std::string> output_names;
    for (std::size_t i = 0; i < session.GetOutputCount(); i++) {
        output_names.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
        auto output_shapes = session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
    }

    // ����data_radar
    std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar(
        mmwave_handle::n_RX, std::vector<std::vector<std::vector<mmwave_handle::Complex>>>(
            mmwave_handle::n_samples, std::vector<std::vector<mmwave_handle::Complex>>(
                mmwave_handle::n_chirps, std::vector<mmwave_handle::Complex>(mmwave_handle::n_frames)
            )
        )
    );

    std::vector<std::vector<float>> input_tensor_values_list;

    //-----------------------------------------
    //               ��ʼ����
    //-----------------------------------------

    // �������ݽ����߳�
    std::thread recv_thread(receive_data, s, std::ref(t_data), std::ref(data_radar), std::ref(cnt));

    // ����ģ�������߳�
    std::thread infer_thread(inference_model, std::ref(session), std::ref(input_names), std::ref(input_tensor_values_list), std::ref(input_shapes), std::ref(output_names), std::ref(t_data), std::ref(data_radar), std::ref(cnt));

    // �ȴ��߳̽���
    recv_thread.join();
    infer_thread.join();

    //-----------------------------------------
    //              �ر� Winsock
    //-----------------------------------------

    closesocket(s);
    WSACleanup();





    return 0;
}
