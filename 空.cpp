//#include <iostream>
//#include <fstream>
//#include <complex>
//#include <vector>
//#include <cmath>
//#include <fftw3.h>
//#include <opencv2/opencv.hpp>
//#include <chrono>
//#include <Windows.h>
//#include <algorithm>
//
//#include "matplotlibcpp.h"
//
//namespace plt = matplotlibcpp;
//
//namespace mmwave_handle {
//	typedef std::complex<double> Complex;  // ���帴������Ϊ˫���ȸ���
//
//	constexpr int N = 64;
//	constexpr int M = 64;
//	constexpr int Q = 64;
//	constexpr int numLanes = 4;
//	constexpr int n_RX = 4;
//	constexpr int n_samples = 64;
//	constexpr int n_chirps = 255;
//	constexpr int n_frames = 30;
//	using namespace std;
//	namespace plt = matplotlibcpp;
//
//	typedef complex<double> Complex;  // ���帴������Ϊ˫���ȸ���
//
//	/**
//	 * @brief ����ADC������ƥ���ض������ݽṹ��
//	 *
//	 * �˺����������ADC��������Ϊһ����ά�����������顣���������ڱ�ʾ�״����ݣ�
//	 * ���ض����״����ݸ�ʽ��ƥ�䡣
//	 *
//	 * @param adcData �����ADC���ݣ�һ������ԭʼ���ݵĶ�����������
//	 * @return һ����ά�����������飬��ʾ���ܺ���״����ݡ�
//	 *
//	 * @details
//	 * - ���ȣ�������ʼ��һ����ά��������`data_radar`�����ڴ洢���ܺ���״����ݡ�
//	 * - ���ţ�������ADC��������Ϊһ����ά��������������`reshapedData`��
//	 * - ��󣬺��������ܺ��ADC����ת��Ϊ������ʽ������䵽`data_radar`�С�
//	 *
//	 * @note
//	 * - �����ADC���ݴ�СӦΪ `n_RX * 2 * newCols`��
//	 * - �ú�����ʵ�ֻ����ض����״����ݽṹ��ADC���ݸ�ʽ��
//	 *
//	 * @warning
//	 * - �ڵ��ô˺���֮ǰ��ȷ��������`n_RX`, `n_samples`, `n_chirps`, �� `n_frames`��ֵ��
//	 * - �����ADC����Ӧ��Ԥ�ڵ����ݸ�ʽ�ʹ�С��ƥ�䡣
//	 */
//	std::vector<std::vector<std::vector<std::vector<Complex>>>> reshape_data(const std::vector<short>& adcData) {
//		// ��ʼ��data_radar
//		std::vector<std::vector<std::vector<std::vector<Complex>>>> data_radar(
//			n_RX, std::vector<std::vector<std::vector<Complex>>>(
//				n_samples, std::vector<std::vector<Complex>>(
//					n_chirps, std::vector<Complex>(n_frames)
//				)
//			)
//		);
//
//		// ��������Ϊ2D����
//		int newRows = n_RX * 2;
//		int newCols = adcData.size() / newRows;
//		std::vector<std::vector<short>> reshapedData(newRows, std::vector<short>(newCols));
//
//		for (int i = 0; i < adcData.size(); ++i) {
//			int row = i % newRows;
//			int col = i / newRows;
//			reshapedData[row][col] = adcData[i];
//		}
//
//		// ת��Ϊ������ʽ����䵽data_radar
//		data_radar.resize(n_RX);
//		for (int i = 0; i < n_RX; ++i) {
//			data_radar[i].resize(n_samples);
//			for (int j = 0; j < n_samples; ++j) {
//				data_radar[i][j].resize(n_chirps);
//				for (int k = 0; k < n_chirps; ++k) {
//					data_radar[i][j][k].resize(n_frames);
//					for (int l = 0; l < n_frames; ++l) {
//						int idx = j + k * n_samples + l * n_samples * n_chirps;
//						data_radar[i][j][k][l] = Complex(reshapedData[i][idx], reshapedData[i + 4][idx]);
//					}
//				}
//			}
//		}
//		// �鿴adcdata����״
//		//std::cout << "ADC���ݵ���״Ϊ��" << adcData.size() << std::endl;//3916800
//
//		return data_radar;
//	}
//
//	/**
//	 * @brief ��ѹ������������ȡ����������
//	 *
//	 * �˺���������ѹ����������range_profile_compressed��speed_profile_compressed �� angle_profile_compressed
//	 * ����ȡ����������ÿ�������������֡��ÿ��֡����������ݵ㡣������ÿ�����������ݵ����ӳɵ���������������
//	 *
//	 * @param range_profile_compressed ����ѹ���ľ��������Ķ�ά������
//	 * @param speed_profile_compressed ����ѹ�����ٶ������Ķ�ά������
//	 * @param angle_profile_compressed ����ѹ���ĽǶ������Ķ�ά������
//	 * @return ���������������������롢�ٶȡ��Ƕȣ��Ķ�ά������
//	 */
//	std::vector<std::vector<float>> extract_feature_vector(
//		std::vector<std::vector<float>>& range_profile_compressed,
//		std::vector<std::vector<float>>& speed_profile_compressed,
//		std::vector<std::vector<float>>& angle_profile_compressed
//	)
//	{
//		std::vector<std::vector<float>> feature_vector(3);
//
//		// ��ȡ range_profile_compressed
//		for (int i = 0; i < n_frames; ++i) {
//			for (int j = 0; j < N; ++j) {
//				feature_vector[0].push_back(range_profile_compressed[i][j]);
//			}
//		}
//
//		// ��ȡ speed_profile_compressed
//		for (int i = 0; i < n_frames; ++i) {
//			for (int j = 0; j < M; ++j) {
//				feature_vector[1].push_back(speed_profile_compressed[i][j]);
//			}
//		}
//
//		// ��ȡ angle_profile_compressed
//		for (int i = 0; i < n_frames; ++i) {
//			for (int j = 0; j < Q; ++j) {
//				feature_vector[2].push_back(angle_profile_compressed[i][j]);
//			}
//		}
//
//		return feature_vector;
//	}
//
//	/**
//	 * @brief �������������ع�ѹ����������
//	 *
//	 * �˺��������������������ع�ѹ����������range_profile_compressed��speed_profile_compressed �� angle_profile_compressed��
//	 * ÿ�������������֡��ÿ��֡����������ݵ㡣
//	 *
//	 * @param feature_vector ���������������������롢�ٶȡ��Ƕȣ��Ķ�ά������
//	 * @param n_frames ÿ�������е�֡�������룩��
//	 * @param N ÿ����������֡�е����ݵ��������룩��
//	 * @param M ÿ���ٶ�����֡�е����ݵ��������룩��
//	 * @param Q ÿ���Ƕ�����֡�е����ݵ��������룩��
//	 * @param[out] range_profile_compressed �ع��ľ���������
//	 * @param[out] speed_profile_compressed �ع����ٶ�������
//	 * @param[out] angle_profile_compressed �ع��ĽǶ�������
//	 */
//	void reconstruct_profiles(
//		const std::vector<std::vector<float>>& feature_vector,
//		int n_frames, int N, int M, int Q,
//		std::vector<std::vector<float>>& range_profile_compressed,
//		std::vector<std::vector<float>>& speed_profile_compressed,
//		std::vector<std::vector<float>>& angle_profile_compressed)
//	{
//		// ���ԭʼ����
//		range_profile_compressed.clear();
//		speed_profile_compressed.clear();
//		angle_profile_compressed.clear();
//
//		// �ع� range_profile_compressed
//		for (int i = 0; i < n_frames; ++i) {
//			std::vector<float> frame_data(N);
//			for (int j = 0; j < N; ++j) {
//				frame_data[j] = feature_vector[0][i * N + j];
//			}
//			range_profile_compressed.push_back(frame_data);
//		}
//
//		// �ع� speed_profile_compressed
//		for (int i = 0; i < n_frames; ++i) {
//			std::vector<float> frame_data(M);
//			for (int j = 0; j < M; ++j) {
//				frame_data[j] = feature_vector[1][i * M + j];
//			}
//			speed_profile_compressed.push_back(frame_data);
//		}
//
//		// �ع� angle_profile_compressed
//		for (int i = 0; i < n_frames; ++i) {
//			std::vector<float> frame_data(Q);
//			for (int j = 0; j < Q; ++j) {
//				frame_data[j] = feature_vector[2][i * Q + j];
//			}
//			angle_profile_compressed.push_back(frame_data);
//		}
//	}
//
//	/**
//	 * @brief �Ӷ������ļ��ж�ȡ�״�ԭʼ���ݣ�����������Ϊָ����״�ĸ�����ʽ��
//	 *
//	 * @param filename    ����Ķ������ļ�����
//	 * @param data_radar  �洢ת����ĸ����״����ݵ���ά��������״Ϊ (n_RX, n_samples, n_chirps, n_frames)��
//	 *
//	 * @details
//	 * - ��ָ���Ķ������ļ��ж�ȡԭʼADC���ݡ�
//	 * - ����ADC����Ϊ��״Ϊ (numLanes*2, -1) �Ķ�ά���ݡ�
//	 * - ����ά����ת��Ϊ������ʽ�����洢��һ���µĶ�ά�����С�
//	 * - ��ת��������ݸ��Ƶ�ָ������ά���ݽṹ�С�
//	 */
//	void read_data(const std::string& filename, std::vector<std::vector<std::vector<std::vector<Complex>>>>& data_radar) {
//		// ��ȡ�ļ�
//		std::ifstream file(filename, std::ios::binary);
//		if (!file.is_open()) {
//			std::cerr << "���ļ�����" << std::endl;
//			return;
//		}
//
//		// ��ȡADC���ݵ�vector
//		std::vector<short> adcData(n_RX * 2 * n_samples * n_chirps * n_frames);
//		file.read(reinterpret_cast<char*>(adcData.data()), adcData.size() * sizeof(short));
//		file.close();
//
//		data_radar = reshape_data(adcData);
//	}
//
//	/**
//	 * @brief �Ը�����FFT���ݽ�����λ����������Ƶ�ʷ����Ƶ����ġ�
//	 *
//	 * @param data  ������ĸ������飬�洢FFT��ʵ�����鲿��
//	 * @param N     ����ĳ��ȡ�
//	 */
//	void fftshift(fftw_complex*& data, int N) {
//		fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//		for (int i = 0; i < N; ++i) {
//			temp[i][0] = data[i][0];
//			temp[i][1] = data[i][1];
//		}
//		for (int i = 0; i < N; ++i) {
//			data[i][0] = temp[(i + N / 2) % N][0];
//			data[i][1] = temp[(i + N / 2) % N][1];
//		}
//		fftw_free(temp);
//	}
//
//	/**
//	 * @brief ���Ҷ�ά���ݵ����ֵ��
//	 *
//	 * @tparam T ��������
//	 * @param data_2d ��ά����
//	 * @return ���ֵ
//	 */
//	template<typename T>
//	T find_max_value(const std::vector<std::vector<T>>& data_2d) {
//		T max_value = 0;
//		for (const auto& row : data_2d) {
//			for (T value : row) {
//				max_value = max(max_value, value);
//			}
//		}
//		return max_value;
//	}
//
//	/**
//	 * @brief ����ά����ѹ����ת��Ϊͼ�����ݷ�Χ(0-255)��
//	 *
//	 * @tparam T ��������
//	 * @param data_2d ��ά����
//	 * @param image_data_gray ͼ����������
//	 */
//	template<typename T>
//	void compress_to_image_data(const std::vector<std::vector<T>>& data_2d, unsigned char*& image_data_gray) {
//		int ccnt = 0;
//		T max_value = find_max_value(data_2d);
//		for (const auto& row : data_2d) {
//			for (T value : row) {
//				image_data_gray[ccnt++] = static_cast<unsigned char>(value / max_value * 255);
//			}
//		}
//	}
//
//	/**
//	 * @brief ����ѹ�������ݵ���ͼ��
//	 *
//	 * @tparam T ��������
//	 * @param data2d ��ά����������ѹ��������
//	 * @param rows ���ݵ�����
//	 * @param columns ���ݵ�����
//	 * @param cmap ��ɫӳ������
//	 * @param title ͼ�����
//	 */
//	template<typename T>
//	void plot_data2d(const std::vector<std::vector<T>>& data2d, int rows, int columns, const std::string& cmap = "jet", const std::string& title = "Image") {
//		unsigned char* image_data_gray = new unsigned char[rows * columns];
//
//		// ת��compressed_data��ͼ�����ݷ�Χ(0-255)
//		compress_to_image_data(data2d, image_data_gray);
//
//		// ����imshow��������ʾͼ��
//		std::map<std::string, std::string> imshow_params = {
//			{"cmap", cmap},
//			{"aspect", "auto"}
//		};
//		plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
//		plt::title(title);
//
//		// �ͷŶ�̬������ڴ�
//		delete[] image_data_gray;
//	}
//
//	/**
// * @brief ��ȡ���������е�range_profile, speed_profile, angle_profile��
// *
// * @tparam T ��������
// * @param feature_vector ��������������range_profile, speed_profile, angle_profile
// * @param n_frames ֡��
// * @param N range_profile������
// * @param M speed_profile������
// * @param Q angle_profile������
// * @return std::tuple<std::vector<std::vector<T>>, std::vector<std::vector<T>>, std::vector<std::vector<T>>>
// *         ����range_profile, speed_profile, angle_profile�Ķ�ά����
// */
//	template<typename T>
//	std::tuple<std::vector<std::vector<T>>, std::vector<std::vector<T>>, std::vector<std::vector<T>>>
//		extract_profiles(const std::vector<std::vector<T>>& feature_vector, int n_frames, int N, int M, int Q) {
//		std::vector<std::vector<T>> range_profile(n_frames, std::vector<T>(N));
//		std::vector<std::vector<T>> speed_profile(n_frames, std::vector<T>(M));
//		std::vector<std::vector<T>> angle_profile(n_frames, std::vector<T>(Q));
//
//		for (int i = 0; i < n_frames; ++i) {
//			for (int j = 0; j < N; ++j) {
//				range_profile[i][j] = feature_vector[0][i * N + j];
//			}
//		}
//
//		for (int i = 0; i < n_frames; ++i) {
//			for (int j = 0; j < M; ++j) {
//				speed_profile[i][j] = feature_vector[1][i * M + j];
//			}
//		}
//
//		for (int i = 0; i < n_frames; ++i) {
//			for (int j = 0; j < Q; ++j) {
//				angle_profile[i][j] = feature_vector[2][i * Q + j];
//			}
//		}
//
//		return std::make_tuple(range_profile, speed_profile, angle_profile);
//	}
//
//	/**
//	 * @brief ����range_profile_compressed����ͼ
//	 *
//	 * @param range_profile_compressed ��ά����������ѹ����range profile����
//	 * @param n_frames ���ݵ�����
//	 * @param N ���ݵ�����
//	 */
//	void plot_range_profile_compressed(const std::vector<std::vector<float>>& range_profile_compressed, int n_frames, int N) {
//		plot_data2d(range_profile_compressed, n_frames, N, "jet", "range_profile_compressed");
//	}
//
//	void plot_speed_profile_compressed(const std::vector<std::vector<float>>& speed_profile_compressed, int n_frames, int M) {
//		plot_data2d(speed_profile_compressed, n_frames, M, "jet", "speed_profile_compressed");
//	}
//
//	void plot_angle_profile_compressed(const std::vector<std::vector<float>>& angle_profile_compressed, int n_frames, int Q) {
//		plot_data2d(angle_profile_compressed, n_frames, Q, "jet", "angle_profile_compressed");
//	}
//
//	// ��feature_vector����ȡrange_profile, speed_profile, angle_profile,Ȼ�����ͼ�����
//	void plot_features(const std::vector<std::vector<float>>& feature_vector) {
//		auto [range_profile, speed_profile, angle_profile] = extract_profiles<float>(feature_vector, n_frames, N, M, Q);
//
//		plt::subplot(1, 3, 1);
//		plot_range_profile_compressed(range_profile, n_frames, N);
//		plt::subplot(1, 3, 2);
//		plot_speed_profile_compressed(speed_profile, n_frames, M);
//		plt::subplot(1, 3, 3);
//		plot_angle_profile_compressed(angle_profile, n_frames, Q);
//	}
//
//	/**
//	 * @brief �Ը������״����ݽ���������ȡ��
//	 *
//	 * @param data_radar       �Ӷ������ļ��ж�ȡ���״����ݡ�
//	 * @param clutter_removal  �Ӳ�ȥ��������Ĭ��Ϊ�ա�
//	 * @param range_profile    �洢range_profile������.
//	 * @param speed_profile    �洢speed_profile������.
//	 * @param angle_profile    �洢angle_profile������.
//	 *
//	 * @return һ�������������������Ķ�ά�������ֱ�Ϊrange_profile_compressed, speed_profile_compressed, angle_profile_compressed��
//	 */
//	std::vector<std::vector<float>> feature_extraction(const std::vector<std::vector<std::vector<std::vector<Complex>>>>& data_radar, const std::string& clutter_removal)
//	{
//		// ����range_profile
//		std::vector<std::vector<std::vector<fftw_complex*>>> range_profile(
//			n_RX, std::vector<std::vector<fftw_complex*>>(
//				n_chirps, std::vector<fftw_complex*>(n_frames)
//			)
//		);
//
//		// ����speed_profile
//		std::vector<std::vector<std::vector<fftw_complex*>>> speed_profile(
//			n_RX, std::vector<std::vector<fftw_complex*>>(
//				N, std::vector<fftw_complex*>(n_frames)
//			)
//		);
//
//		// ����angle_profile
//		std::vector<std::vector<std::vector<fftw_complex*>>> angle_profile(
//			N, std::vector<std::vector<fftw_complex*>>(
//				M, std::vector<fftw_complex*>(n_frames)
//			)
//		);
//
//		// ����in
//		std::vector<std::vector<std::vector<fftw_complex*>>> in(
//			n_RX, std::vector<std::vector<fftw_complex*>>(
//				n_chirps, std::vector<fftw_complex*>(n_frames)
//			)
//		);
//
//		// Ϊrange_profile, speed_profile, angle_profile�����ڴ�
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_chirps; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					range_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//				}
//			}
//		}
//
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < N; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					speed_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * M);
//				}
//			}
//		}
//		for (int i = 0; i < N; ++i) {
//			for (int j = 0; j < M; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					angle_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * Q);
//				}
//			}
//		}
//
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_chirps; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					in[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n_samples);
//				}
//			}
//		}
//
//		// �����ݸ��Ƶ�in��
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_chirps; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					for (int l = 0; l < n_samples; ++l) {
//						in[i][j][k][l][0] = data_radar[i][l][j][k].real();
//						in[i][j][k][l][1] = data_radar[i][l][j][k].imag();
//					}
//				}
//			}
//		}
//
//		// ִ��FFT
//
//		fftw_plan plan = fftw_plan_dft_1d(n_samples, in[0][0][0], range_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_chirps; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					fftw_execute_dft(plan, in[i][j][k], range_profile[i][j][k]);
//				}
//			}
//		}
//		fftw_destroy_plan(plan);
//
//		// �Ӳ�ȥ��
//		if (clutter_removal == "avg") {
//			// range_profile(n_RX, n_chirps, n_frames, N) -> range_profile(n_RX, n_chirps, n_frames, N)
//			//range_profile = range_profile - np.mean(range_profile, axis = 3)[:, : , np.newaxis, : ]
//			for (int i = 0; i < n_frames; ++i) {
//				for (int j = 0; j < n_RX; ++j) {
//					for (int k = 0; k < N; ++k) {
//						double mean_real = 0;
//						double mean_imag = 0;
//						for (int l = 0; l < n_chirps; ++l) {
//							mean_real += range_profile[j][l][i][k][0];
//							mean_imag += range_profile[j][l][i][k][1];
//						}
//						mean_real /= n_chirps;
//						mean_imag /= n_chirps;
//						for (int l = 0; l < n_chirps; ++l) {
//							range_profile[j][l][i][k][0] -= mean_real;
//							range_profile[j][l][i][k][1] -= mean_imag;
//						}
//					}
//				}
//			}
//		}
//		else if (clutter_removal == "mti") {
//			// [ ] ʵ���ƶ�Ŀ��ָʾ��MTI���˲�
//		}
//
//		// �ٶ�������FFT(range_profile(n_RX, n_chirps, n_frames) -> speed_profile(n_RX, M, n_frames))
//		plan = fftw_plan_dft_1d(M, speed_profile[0][0][0], speed_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//		fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_frames; ++j) {
//				for (int k = 0; k < N; ++k) {
//					for (int l = 0; l < M; ++l) {
//						temp[l][0] = range_profile[i][l][j][k][0];
//						temp[l][1] = range_profile[i][l][j][k][1];
//					}
//					fftw_execute_dft(plan, temp, speed_profile[i][k][j]);
//					fftshift(speed_profile[i][k][j], M);
//				}
//			}
//		}
//		fftw_free(temp);
//
//		plan = fftw_plan_dft_1d(Q, angle_profile[0][0][0], angle_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//		// ��ʱ����
//		temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * M);
//
//		// �Ƕ�������FFT(speed_profile(n_RX, N, n_frames) -> angle_profile(N, M, n_frames))
//		for (int i = 0; i < N; ++i) {
//			for (int j = 0; j < n_frames; ++j) {
//				for (int k = 0; k < M; ++k) {
//					for (int l = 0; l < n_RX; ++l) {
//						temp[l][0] = speed_profile[l][i][j][k][0];
//						temp[l][1] = speed_profile[l][i][j][k][1];
//					}
//					fftw_execute_dft(plan, temp, angle_profile[i][k][j]);
//					fftshift(angle_profile[i][k][j], Q);
//				}
//			}
//		}
//		fftw_free(temp);
//
//		// ѹ��ά��
//		std::vector<std::vector<float>> range_profile_compressed(n_frames, std::vector<float>(N));
//		std::vector<std::vector<float>> speed_profile_compressed(n_frames, std::vector<float>(M));
//		std::vector<std::vector<float>> angle_profile_compressed(n_frames, std::vector<float>(Q));
//
//		// ѹ��range_profile
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_chirps; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					for (int l = 0; l < N; ++l) {
//						range_profile_compressed[k][l] += abs(Complex(range_profile[i][j][k][l][0], range_profile[i][j][k][l][1]));
//					}
//				}
//			}
//		}
//
//		// ѹ��speed_profile(n_RX, N, n_frames) -> speed_profile_compressed(n_frames, M)
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < N; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					for (int l = 0; l < M; ++l) {
//						speed_profile_compressed[k][l] += abs(Complex(speed_profile[i][j][k][l][0], speed_profile[i][j][k][l][1]));
//					}
//				}
//			}
//		}
//
//		// ѹ��angle_profile(N, M, n_frames) -> angle_profile_compressed(n_frames, Q)
//		for (int i = 0; i < N; ++i) {
//			for (int j = 0; j < M; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					for (int l = 0; l < Q; ++l) {
//						angle_profile_compressed[k][l] += abs(Complex(angle_profile[i][j][k][l][0], angle_profile[i][j][k][l][1]));
//					}
//				}
//			}
//		}
//
//		std::vector<std::vector<float>> feature_vector = extract_feature_vector(range_profile_compressed, speed_profile_compressed, angle_profile_compressed);
//
//		//�ͷ��ڴ�
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_chirps; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					fftw_free(range_profile[i][j][k]);
//				}
//			}
//		}
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < N; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					fftw_free(speed_profile[i][j][k]);
//				}
//			}
//		}
//		for (int i = 0; i < N; ++i) {
//			for (int j = 0; j < M; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					fftw_free(angle_profile[i][j][k]);
//				}
//			}
//		}
//		for (int i = 0; i < n_RX; ++i) {
//			for (int j = 0; j < n_chirps; ++j) {
//				for (int k = 0; k < n_frames; ++k) {
//					fftw_free(in[i][j][k]);
//				}
//			}
//		}
//
//		return feature_vector;
//	}
//
//	/**
//	* @brief �Ը����Ķ������ļ�����������ȡ��
//	*
//	* @param filename ����Ķ������ļ�����
//	* @param clutter_removal �Ӳ�ȥ��������Ĭ��Ϊ�ա�
//	* @return һ�������������������Ķ�ά�������ֱ�Ϊrange_profile_compressed, speed_profile_compressed, angle_profile_compressed��
//	*/
//	std::vector<std::vector<float>> feature_extraction_single(const string& filename, const string& clutter_removal = "")
//	{
//		// ����data_radar
//		std::vector<std::vector<std::vector<std::vector<Complex>>>> data_radar(
//			n_RX, std::vector<std::vector<std::vector<Complex>>>(
//				n_samples, std::vector<std::vector<Complex>>(
//					n_chirps, std::vector<Complex>(n_frames)
//				)
//			)
//		);
//
//		// ��data_radar��ֵ
//		read_data(filename, data_radar);
//
//		return feature_extraction(data_radar, clutter_removal);
//	}
//}
//
//int main() {
//    //���ļ�����ȡdata_radar
//    std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar;
//    mmwave_handle::read_data("K:/dataset/1000/1/1.bin", data_radar);
//    // ������ȡ
//    std::vector<std::vector<float>> feature_vector = mmwave_handle::feature_extraction(data_radar, "avg");
//
//	//plt::figure_size(1200, 400);
// //   mmwave_handle::plot_features(feature_vector);
//
//	auto [range_profile, speed_profile, angle_profile] = mmwave_handle::extract_profiles<float>(feature_vector, mmwave_handle::n_frames, mmwave_handle::N, mmwave_handle::M, mmwave_handle::Q);
//	// ����ѹ�������ݵ���ͼ
//	plt::figure_size(1200, 400);
//	plt::subplot(1, 3, 1);
//	mmwave_handle::plot_range_profile_compressed(range_profile, mmwave_handle::n_frames, mmwave_handle::N);
//	plt::subplot(1, 3, 2);
//	mmwave_handle::plot_speed_profile_compressed(speed_profile, mmwave_handle::n_frames, mmwave_handle::M);
//	plt::subplot(1, 3, 3);
//	mmwave_handle::plot_angle_profile_compressed(angle_profile, mmwave_handle::n_frames, mmwave_handle::Q);
//	plt::show();
//    return 0;
//}