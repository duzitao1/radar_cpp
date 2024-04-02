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

//#define DEBUG

#define BUFLEN 1476			//缓冲区的最大长度
#ifdef DEBUG
#define ADDR "127.0.0.1"	//用于通信的适配器的地址
#else
#define ADDR "192.168.33.30"
#endif

#define PORT 4098				//用于接收传入数据的端口
#define boardid 0
#define packetsize 1456   // 为减少时间开销，使用启发式值

//ThreadSafeQueue<short*> data_queue; // 数据队列
ThreadSafeQueue<std::vector<short>> data_queue; // 数据队列

// cnt的互斥锁
std::mutex cnt_mtx;
// t_data的互斥锁
std::mutex t_data_mtx;
// data的互斥锁
std::mutex data_mtx;

// 数据接收函数
void receive_data(SOCKET s, std::vector<short>& t_data, std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>>& data_radar, int& cnt)
{
	char udpbuf[BUFLEN];
	int recv_len;
	int seqnum_new, bytenum_new;
	int seqmum_old = 0;
	int bytenum_old = 0;
	// 丢包总数
	int lost_cnt = 0;
	// t_data.size()变量
	int t_data_size = t_data.size();

	// 用于存储数据
	//short *data = new short[3916800];

	std::vector<short> data(3916800);

	while (1)
	{
		if ((recv_len = recvfrom(s, udpbuf, BUFLEN, 0, 0, 0)) == SOCKET_ERROR)
		{
			printf("recvfrom() 失败，错误代码：%d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		// 取出前4个字节，序列号
		memcpy(&seqnum_new, udpbuf, 4);
		memcpy(&bytenum_new, udpbuf + 4, 6);

		cnt_mtx.lock();  // 锁定互斥锁
		data_mtx.lock();  // 锁定互斥锁

		if (cnt + (recv_len - 10) / 2 < 3916800)
		{
			memcpy(data.data() + cnt, udpbuf + 10, recv_len - 10);
			cnt += (recv_len - 10) / 2;
		}
		else
		{
			memcpy(data.data() + cnt, udpbuf + 10, (3916800 - cnt) * 2);

			// 将数据复制到新的vector并入队
			std::vector<short> data_temp(data.begin(), data.end());
			data_queue.enqueue(data_temp);

			// 更新data
			memcpy(data.data(), udpbuf + 10 + (3916800 - cnt) * 2, recv_len - 10 - (3916800 - cnt) * 2);
			cnt = recv_len - 10 - (3916800 - cnt) * 2;

			// 输出队列大小
			//std::cout << "队列大小: " << data_queue.size() << std::endl;
		}

		data_mtx.unlock();  // 解锁互斥锁
		cnt_mtx.unlock();  // 解锁互斥锁
	}
}

// 模型推理函数
void inference_model(Ort::Session& session, std::vector<std::string>& input_names, std::vector<std::vector<float>>& input_tensor_values_list, std::vector<std::int64_t>& input_shapes, std::vector<std::string>& output_names, std::vector<short>& t_data, std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>>& data_radar, int& cnt)
{
	std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar_local;
	std::vector<std::vector<float>> input_tensor_values_list_local(3, std::vector<float>(1920, 0));
	// 存储上一个数据
	std::vector<std::vector<float>> input_tensor_values_list_local_old(3, std::vector<float>(1920, 0));
	// 存储需要推理的数据(初始化为(3, 1920)的0矩阵)
	std::vector<std::vector<float>> input_tensor_values_list_local_temp(3, std::vector<float>(1920, 0));
	float* output_data;
	double max_value = 0;
	int max_index = 0;

	bool status = true;


	while (1)
	{
		// 从队列中取出数据
		if (!data_queue.empty())
		{
			std::vector<short> data_temp;
			data_queue.try_dequeue(data_temp);

			data_radar_local = mmwave_handle::reshape_data(data_temp);

			// 特征提取
			input_tensor_values_list_local = mmwave_handle::feature_extraction(data_radar_local, "avg");

			if (status == true) {
				input_tensor_values_list_local_old = input_tensor_values_list_local;
				status = false;
			}

			// 起始时间(一共4个时间窗口)
			for (int t = 0; t <= 30; t += 10)
			{
				std::cout << "t: " << t << std::endl;
				int cnt = 0;

				for (int i = t; i < t + 30; i++) {
					if (i < 30) {
						for (int j = 0; j < 64; j++) {
							for (int k = 0; k < 3; k++) {
								input_tensor_values_list_local_temp[k][cnt++] = (input_tensor_values_list_local_old[k][i * 64 + j]);
							}
						}
					}
					else {
						for (int j = 0; j < 64; j++) {
							for(int k=0;k<3;k++)
								input_tensor_values_list_local_temp[k][cnt++] = input_tensor_values_list_local[k][(i - 30) * 64 + j];
						}
					}
				}
				// 输出input_tensor_values_list_local的形状
				std::cout<< "input_tensor_values_list_local_temp: " << input_tensor_values_list_local_temp.size() << " " << input_tensor_values_list_local_temp[0].size() << std::endl;


				//output_data = run_model_with_input(session, input_names, input_tensor_values_list_local_temp, input_shapes, output_names);
				//for (int i = 0; i < 10; i++)
				//	std::cout << output_data[i] << " ";
			}

			// 运行推理
			output_data = run_model_with_input(session, input_names, input_tensor_values_list_local, input_shapes, output_names);

			// 绘制特征
			//plt::ion();
			//mmwave_handle::plot_features(input_tensor_values_list_local);
			//plt::pause(0.2);

			// 找到最大值的索引
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

			//if(max_index != 4)
				std::cout << "max_index: " << max_index << std::endl;
			
			input_tensor_values_list_local_old = input_tensor_values_list_local;
		}
	}
}

int main()
{
	//-----------------------------------------
	//              初始化数据
	//-----------------------------------------

	std::vector<short> t_data(3916800);
	int recv_len, seqnum_prev, seqnum_new;
	int bytenum_prev = 0;
	int bytenum_new = 0;
	seqnum_prev = 0;    // 第一个包的序列号为 1

	//一个计数器,用于记录当前data中的位置
	int cnt = 0;
	//-----------------------------------------
	//              初始化 Winsock
	//-----------------------------------------

	SOCKET s;
	struct sockaddr_in server;
	char udpbuf[BUFLEN];
	WSADATA wsa;

	//初始化 Winsock
	printf("\n正在初始化 Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("失败。错误代码：%d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("已初始化。\n");

	//创建套接字
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("无法创建套接字：%d", WSAGetLastError());
	}
	printf("套接字已创建。\n");

	//准备 sockaddr_in 结构
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ADDR);
	server.sin_port = htons(PORT);

	//绑定
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("绑定失败，错误代码：%d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("绑定完成");

	//-----------------------------------------
	//              初始化模型
	//-----------------------------------------

	// 设置模型文件路径
	std::basic_string<ORTCHAR_T> model_file = L"K:/aio_radar/avg_model.onnx";

	// onnxruntime 设置
	Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
	Ort::SessionOptions session_options;
	Ort::Session session = Ort::Session(env, model_file.c_str(), session_options);

	// 获取输入和输出节点名称/形状
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

	// 创建data_radar
	std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar(
		mmwave_handle::n_RX, std::vector<std::vector<std::vector<mmwave_handle::Complex>>>(
			mmwave_handle::n_samples, std::vector<std::vector<mmwave_handle::Complex>>(
				mmwave_handle::n_chirps, std::vector<mmwave_handle::Complex>(mmwave_handle::n_frames)
			)
		)
	);

	std::vector<std::vector<float>> input_tensor_values_list;

	//-----------------------------------------
	//               开始监听
	//-----------------------------------------

	// 启动数据接收线程
	std::thread recv_thread(receive_data, s, std::ref(t_data), std::ref(data_radar), std::ref(cnt));

	// 启动模型推理线程
	std::thread infer_thread(inference_model, std::ref(session), std::ref(input_names), std::ref(input_tensor_values_list), std::ref(input_shapes), std::ref(output_names), std::ref(t_data), std::ref(data_radar), std::ref(cnt));

	// 等待线程结束
	recv_thread.join();
	infer_thread.join();

	//-----------------------------------------
	//              关闭 Winsock
	//-----------------------------------------

	closesocket(s);
	WSACleanup();

	return 0;
}