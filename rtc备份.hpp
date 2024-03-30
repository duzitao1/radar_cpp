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
//// ����matplotlibcpp��
//#include "matplotlibcpp.h"
//
//
//using namespace std;
//namespace plt = matplotlibcpp;
//
//typedef complex<double> Complex;  // ���帴������Ϊ˫���ȸ���
//
///**
// * @brief �Ӷ������ļ��ж�ȡ�״�ԭʼ���ݣ�����������Ϊָ����״�ĸ�����ʽ��
// *
// * @param filename    ����Ķ������ļ�����
// * @param data_radar  �洢ת����ĸ����״����ݵ���ά��������״Ϊ (n_RX, n_samples, n_chirps, n_frames)��
// * @param n_RX        ����ͨ������Ĭ��Ϊ4��
// * @param n_samples   ÿ��chirp����������Ĭ��Ϊ64��
// * @param n_chirps    chirps��������Ĭ��Ϊ255��
// * @param n_frames    ����֡��������Ĭ��Ϊ30��
// *
// * @details
// * - ��ָ���Ķ������ļ��ж�ȡԭʼADC���ݡ�
// * - ����ADC����Ϊ��״Ϊ (numLanes*2, -1) �Ķ�ά���ݡ�
// * - ����ά����ת��Ϊ������ʽ�����洢��һ���µĶ�ά�����С�
// * - ��ת��������ݸ��Ƶ�ָ������ά���ݽṹ�С�
// */
//void read_data(const string& filename, vector<vector<vector<vector<Complex>>>>& data_radar, int n_RX = 4, int n_samples = 64, int n_chirps = 255, int n_frames = 30)
//{
//    // ��ȡ�ļ�
//    ifstream file(filename, ios::binary);
//    if (!file.is_open()) {
//        cerr << "���ļ�����" << endl;
//        return;
//    }
//
//    vector<short> adcData(n_RX * 2 * n_samples * n_chirps * n_frames);
//    file.read(reinterpret_cast<char*>(adcData.data()), adcData.size() * sizeof(short));
//    file.close();
//
//    // ��adcData����Ϊ(numLanes*2, -1)
//    int newRows = n_RX * 2;
//    int newCols = adcData.size() / newRows;
//    std::vector<std::vector<short>> reshapedData(newRows, std::vector<short>(newCols));
//
//    for (int i = 0; i < adcData.size(); ++i) {
//        int row = i % (n_RX * 2);
//        int col = i / (n_RX * 2);
//        reshapedData[row][col] = adcData[i];
//    }
//
//    // ��reshapedDataת��Ϊ������ʽ
//    vector<vector<Complex>> combinedData(n_RX, vector<Complex>(newCols));
//    for (int i = 0; i < newCols; ++i) {
//        for (int j = 0; j < n_RX; ++j) {
//            combinedData[j][i] = Complex(reshapedData[j][i], reshapedData[j + 4][i]);
//        }
//    }
//
//    // �����ݸ��Ƶ�data_radar��
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_samples; ++j) {
//            for (int k = 0; k < n_chirps; ++k) {
//                for (int l = 0; l < n_frames; ++l) {
//                    data_radar[i][j][k][l] = combinedData[i][j + k * n_samples + l * n_samples * n_chirps];
//                }
//            }
//        }
//    }
//}
//
///**
// * @brief �Ը�����FFT���ݽ�����λ����������Ƶ�ʷ����Ƶ����ġ�
// *
// * @param data  ������ĸ������飬�洢FFT��ʵ�����鲿��
// * @param N     ����ĳ��ȡ�
// */
//void fftshift(fftw_complex*& data, int N) {
//    fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//    for (int i = 0; i < N; ++i) {
//        temp[i][0] = data[i][0];
//        temp[i][1] = data[i][1];
//    }
//    for (int i = 0; i < N; ++i) {
//        data[i][0] = temp[(i + N / 2) % N][0];
//        data[i][1] = temp[(i + N / 2) % N][1];
//    }
//}
//
//// �Դ����data_radar����������ȡ�ĺ���
//
///**
// * @brief �Ը������״����ݽ���������ȡ��
// *
// * @param data_radar       �Ӷ������ļ��ж�ȡ���״����ݡ�
// * @param clutter_removal  �Ӳ�ȥ��������Ĭ��Ϊ�ա�
// * @param N                range_profile�ĳ��ȣ�Ĭ��Ϊ64��
// * @param M                speed_profile�ĳ��ȣ�Ĭ��Ϊ64��
// * @param Q                angle_profile�ĳ��ȣ�Ĭ��Ϊ64��
// * @param numLanes         ����ͨ������Ĭ��Ϊ4��
// * @param n_RX             ����ͨ������Ĭ��Ϊ4��
// * @param n_samples        ÿ��chirp����������Ĭ��Ϊ64��
// * @param n_chirps         chirps��������Ĭ��Ϊ255��
// * @param n_frames         ����֡��������Ĭ��Ϊ30��
// *
// * @return һ�������������������Ķ�ά�������ֱ�Ϊrange_profile_compressed, speed_profile_compressed, angle_profile_compressed��
// */
//std::vector<std::vector<float>> feature_extraction(const std::vector<std::vector<std::vector<std::vector<Complex>>>>& data_radar, const string& clutter_removal = "",
//    int N = 64, int M = 64, int Q = 64, int numLanes = 4,
//    int n_RX = 4, int n_samples = 64, int n_chirps = 255, int n_frames = 30)
//{
//    // ����range_profile
//    std::vector<std::vector<std::vector<fftw_complex*>>> range_profile(
//        n_RX, std::vector<std::vector<fftw_complex*>>(
//            n_chirps, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // ����speed_profile
//    std::vector<std::vector<std::vector<fftw_complex*>>> speed_profile(
//        n_RX, std::vector<std::vector<fftw_complex*>>(
//            N, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // ����angle_profile
//    std::vector<std::vector<std::vector<fftw_complex*>>> angle_profile(
//        N, std::vector<std::vector<fftw_complex*>>(
//            M, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // Ϊrange_profile, speed_profile, angle_profile�����ڴ�
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_chirps; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                range_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//            }
//        }
//    }
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < N; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                speed_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * M);
//            }
//        }
//    }
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < M; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                angle_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * Q);
//            }
//        }
//    }
//
//    // ����in
//    std::vector<std::vector<std::vector<fftw_complex*>>> in(
//        n_RX, std::vector<std::vector<fftw_complex*>>(
//            n_chirps, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // �����ڴ�
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_chirps; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                in[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n_samples);
//            }
//        }
//    }
//
//    // �����ݸ��Ƶ�in��
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_chirps; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                for (int l = 0; l < n_samples; ++l) {
//                    in[i][j][k][l][0] = data_radar[i][l][j][k].real();
//                    in[i][j][k][l][1] = data_radar[i][l][j][k].imag();
//                }
//            }
//        }
//    }
//
//    // ִ��FFT
//
//    fftw_plan plan = fftw_plan_dft_1d(n_samples, in[0][0][0], range_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_chirps; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                fftw_execute_dft(plan, in[i][j][k], range_profile[i][j][k]);
//            }
//        }
//    }
//    fftw_destroy_plan(plan);
//
//#ifdef DEBUG
//    std::cout << "����ʹ�þ�̬�Ӳ�ȥ������" << clutter_removal << std::endl;
//#endif
//
//
//    // �Ӳ�ȥ��
//    if (clutter_removal == "avg") {
//        // range_profile(n_RX, n_chirps, n_frames, N) -> range_profile(n_RX, n_chirps, n_frames, N)
//        //range_profile = range_profile - np.mean(range_profile, axis = 3)[:, : , np.newaxis, : ]
//        for (int i = 0; i < n_frames; ++i) {
//            for (int j = 0; j < n_RX; ++j) {
//                for (int k = 0; k < N; ++k) {
//                    double mean_real = 0;
//                    double mean_imag = 0;
//                    for (int l = 0; l < n_chirps; ++l) {
//                        mean_real += range_profile[j][l][i][k][0];
//                        mean_imag += range_profile[j][l][i][k][1];
//                    }
//                    mean_real /= n_chirps;
//                    mean_imag /= n_chirps;
//                    for (int l = 0; l < n_chirps; ++l) {
//                        range_profile[j][l][i][k][0] -= mean_real;
//                        range_profile[j][l][i][k][1] -= mean_imag;
//                    }
//                }
//            }
//        }
//    }
//    else if (clutter_removal == "mti") {
//        // [ ] ʵ���ƶ�Ŀ��ָʾ��MTI���˲�
//    }
//
//    // �ٶ�������FFT(range_profile(n_RX, n_chirps, n_frames) -> speed_profile(n_RX, M, n_frames))
//    plan = fftw_plan_dft_1d(M, speed_profile[0][0][0], speed_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//    fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_frames; ++j) {
//            for (int k = 0; k < N; ++k) {
//                for (int l = 0; l < M; ++l) {
//                    temp[l][0] = range_profile[i][l][j][k][0];
//                    temp[l][1] = range_profile[i][l][j][k][1];
//                }
//                fftw_execute_dft(plan, temp, speed_profile[i][k][j]);
//                fftshift(speed_profile[i][k][j], M);
//            }
//        }
//    }
//
//    plan = fftw_plan_dft_1d(Q, angle_profile[0][0][0], angle_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//    // ��ʱ����
//    temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * M);
//
//    // �Ƕ�������FFT(speed_profile(n_RX, N, n_frames) -> angle_profile(N, M, n_frames))
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < n_frames; ++j) {
//            for (int k = 0; k < M; ++k) {
//                for (int l = 0; l < n_RX; ++l) {
//                    temp[l][0] = speed_profile[l][i][j][k][0];
//                    temp[l][1] = speed_profile[l][i][j][k][1];
//                }
//                fftw_execute_dft(plan, temp, angle_profile[i][k][j]);
//                fftshift(angle_profile[i][k][j], Q);
//            }
//        }
//    }
//
//
//    // ѹ��ά��
//    std::vector<std::vector<double>> range_profile_compressed(n_frames, std::vector<double>(N));
//    std::vector<std::vector<double>> speed_profile_compressed(n_frames, std::vector<double>(M));
//    std::vector<std::vector<double>> angle_profile_compressed(n_frames, std::vector<double>(Q));
//
//
//    // ѹ��range_profile
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_chirps; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                for (int l = 0; l < N; ++l) {
//                    range_profile_compressed[k][l] += abs(Complex(range_profile[i][j][k][l][0], range_profile[i][j][k][l][1]));
//                }
//            }
//        }
//    }
//
//    // ѹ��speed_profile(n_RX, N, n_frames) -> speed_profile_compressed(n_frames, M)
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < N; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                for (int l = 0; l < M; ++l) {
//                    speed_profile_compressed[k][l] += abs(Complex(speed_profile[i][j][k][l][0], speed_profile[i][j][k][l][1]));
//                }
//            }
//        }
//    }
//
//    // ѹ��angle_profile(N, M, n_frames) -> angle_profile_compressed(n_frames, Q)
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < M; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                for (int l = 0; l < Q; ++l) {
//                    angle_profile_compressed[k][l] += abs(Complex(angle_profile[i][j][k][l][0], angle_profile[i][j][k][l][1]));
//                }
//            }
//        }
//    }
//
//    // ��range_profile_compressed, speed_profile_compressed, angle_profile_compressed�ȱ��һά���飬�ٴ���feature_vector
//    std::vector<std::vector<float>> feature_vector(3);
//    for (int i = 0; i < n_frames; ++i) {
//        for (int j = 0; j < N; ++j) {
//            feature_vector[0].push_back(range_profile_compressed[i][j]);
//        }
//    }
//    for (int i = 0; i < n_frames; ++i) {
//        for (int j = 0; j < M; ++j) {
//            feature_vector[1].push_back(speed_profile_compressed[i][j]);
//        }
//    }
//    for (int i = 0; i < n_frames; ++i) {
//        for (int j = 0; j < Q; ++j) {
//            feature_vector[2].push_back(angle_profile_compressed[i][j]);
//        }
//    }
//
//    return feature_vector;
//
//}
//
//std::vector<std::vector<float>> feature_extraction_single(const string& filename, const string& clutter_removal = "",
//    int N = 64, int M = 64, int Q = 64, int numLanes = 4,
//    int n_RX = 4, int n_samples = 64, int n_chirps = 255, int n_frames = 30)
//{
//
//    // ����data_radar
//    std::vector<std::vector<std::vector<std::vector<Complex>>>> data_radar(
//        n_RX, std::vector<std::vector<std::vector<Complex>>>(
//            n_samples, std::vector<std::vector<Complex>>(
//                n_chirps, std::vector<Complex>(n_frames)
//            )
//        )
//    );
//
//    // ��data_radar��ֵ
//    read_data(filename, data_radar, n_RX, n_samples, n_chirps, n_frames);
//
//    return feature_extraction(data_radar, clutter_removal, N, M, Q, numLanes, n_RX, n_samples, n_chirps, n_frames);
//
//}