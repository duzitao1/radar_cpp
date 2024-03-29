#include <iostream>
#include <fstream>
#include <complex>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <Windows.h>
#include <algorithm>

#include "matplotlibcpp.h"

namespace plt = matplotlibcpp;

namespace mmwave_handle {



    typedef std::complex<double> Complex;  // 定义复数类型为双精度复数

    constexpr int N = 64;
    constexpr int M = 64;
    constexpr int Q = 64;
    constexpr int numLanes = 4;
    constexpr int n_RX = 4;
    constexpr int n_samples = 64;
    constexpr int n_chirps = 255;
    constexpr int n_frames = 30;
    using namespace std;
    namespace plt = matplotlibcpp;

    typedef complex<double> Complex;  // 定义复数类型为双精度复数

    /**
     * @brief 重塑ADC数据以匹配特定的数据结构。
     *
     * 此函数将输入的ADC数据重塑为一个四维复数向量数组。该数组用于表示雷达数据，
     * 与特定的雷达数据格式相匹配。
     *
     * @param adcData 输入的ADC数据，一个包含原始数据的短整型向量。
     * @return 一个四维复数向量数组，表示重塑后的雷达数据。
     *
     * @details
     * - 首先，函数初始化一个四维向量数组`data_radar`，用于存储重塑后的雷达数据。
     * - 接着，函数将ADC数据重塑为一个二维短整型向量数组`reshapedData`。
     * - 最后，函数将重塑后的ADC数据转换为复数形式，并填充到`data_radar`中。
     *
     * @note
     * - 输入的ADC数据大小应为 `n_RX * 2 * newCols`。
     * - 该函数的实现基于特定的雷达数据结构和ADC数据格式。
     *
     * @warning
     * - 在调用此函数之前，确保设置了`n_RX`, `n_samples`, `n_chirps`, 和 `n_frames`的值。
     * - 输入的ADC数据应与预期的数据格式和大小相匹配。
     */
    std::vector<std::vector<std::vector<std::vector<Complex>>>> reshape_data(const std::vector<short>& adcData) {

        // 初始化data_radar
        std::vector<std::vector<std::vector<std::vector<Complex>>>> data_radar(
            n_RX, std::vector<std::vector<std::vector<Complex>>>(
                n_samples, std::vector<std::vector<Complex>>(
                    n_chirps, std::vector<Complex>(n_frames)
                )
            )
        );

        // 数据重塑为2D向量
        int newRows = n_RX * 2;
        int newCols = adcData.size() / newRows;
        std::vector<std::vector<short>> reshapedData(newRows, std::vector<short>(newCols));

        for (int i = 0; i < adcData.size(); ++i) {
            int row = i % newRows;
            int col = i / newRows;
            reshapedData[row][col] = adcData[i];
        }

        // 转换为复数形式并填充到data_radar
        data_radar.resize(n_RX);
        for (int i = 0; i < n_RX; ++i) {
            data_radar[i].resize(n_samples);
            for (int j = 0; j < n_samples; ++j) {
                data_radar[i][j].resize(n_chirps);
                for (int k = 0; k < n_chirps; ++k) {
                    data_radar[i][j][k].resize(n_frames);
                    for (int l = 0; l < n_frames; ++l) {
                        int idx = j + k * n_samples + l * n_samples * n_chirps;
                        data_radar[i][j][k][l] = Complex(reshapedData[i][idx], reshapedData[i + 4][idx]);
                    }
                }
            }
        }
        // 查看adcdata的形状
        //std::cout << "ADC数据的形状为：" << adcData.size() << std::endl;//3916800

        return data_radar;
    }


    /**
     * @brief 从二进制文件中读取雷达原始数据，并将其重塑为指定形状的复数形式。
     *
     * @param filename    输入的二进制文件名。
     * @param data_radar  存储转换后的复数雷达数据的四维向量，形状为 (n_RX, n_samples, n_chirps, n_frames)。
     *
     * @details
     * - 从指定的二进制文件中读取原始ADC数据。
     * - 重塑ADC数据为形状为 (numLanes*2, -1) 的二维数据。
     * - 将二维数据转换为复数形式，并存储在一个新的二维向量中。
     * - 将转换后的数据复制到指定的四维数据结构中。
     */
    void read_data(const std::string& filename, std::vector<std::vector<std::vector<std::vector<Complex>>>>& data_radar) {

        // 读取文件
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "打开文件错误！" << std::endl;
            return;
        }

        // 读取ADC数据到vector
        std::vector<short> adcData(n_RX * 2 * n_samples * n_chirps * n_frames);
        file.read(reinterpret_cast<char*>(adcData.data()), adcData.size() * sizeof(short));
        file.close();

        data_radar = reshape_data(adcData);
    }


    /**
     * @brief 对给定的FFT数据进行移位操作，将零频率分量移到中心。
     *
     * @param data  待处理的复数数组，存储FFT的实部和虚部。
     * @param N     数组的长度。
     */
    void fftshift(fftw_complex*& data, int N) {
        fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
        for (int i = 0; i < N; ++i) {
            temp[i][0] = data[i][0];
            temp[i][1] = data[i][1];
        }
        for (int i = 0; i < N; ++i) {
            data[i][0] = temp[(i + N / 2) % N][0];
            data[i][1] = temp[(i + N / 2) % N][1];
        }
        fftw_free(temp);
    }
    /**
     * @brief 绘制range_profile_compressed的热图
     *
     * @param range_profile_compressed 二维向量，包含压缩的range profile数据
     * @param n_frames 数据的行数
     * @param N 数据的列数
     */
    void plot_range_profile_compressed(const std::vector<std::vector<double>>& range_profile_compressed, int n_frames, int N) {
        // 初始化图像数据数组
        int rows = n_frames;
        int columns = N;
        unsigned char* image_data_gray = new unsigned char[rows * columns];

        // 查找range_profile_compressed的最大值
        double range_profile_max = 0;
        for (int i = 0; i < n_frames; ++i) {
            for (int j = 0; j < N; ++j) {
                range_profile_max = max(range_profile_max, range_profile_compressed[i][j]);
            }
        }
        std::cout << "range_profile_max: " << range_profile_max << std::endl;

        // 转换range_profile_compressed到图像数据范围(0-255)
        int ccnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                image_data_gray[ccnt++] = static_cast<unsigned char>(range_profile_compressed[i][j] / range_profile_max * 255);
            }
        }

        // 设置imshow参数并显示图像
        std::map<std::string, std::string> imshow_params = {
            {"cmap", "jet"},
            {"aspect", "auto"}
        };
        plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
        plt::title("Gray Scale Image");
        plt::show();

        // 释放动态分配的内存
        delete[] image_data_gray;
    }

    void plot_speed_profile_compressed(const std::vector<std::vector<double>>& speed_profile_compressed, int n_frames, int M) {
        // 初始化图像数据数组
        int rows = n_frames;
        int columns = M;
        unsigned char* image_data_gray = new unsigned char[rows * columns];

        // 查找speed_profile_compressed的最大值
        double speed_profile_max = 0;
        for (int i = 0; i < n_frames; ++i) {
            for (int j = 0; j < M; ++j) {
                speed_profile_max = max(speed_profile_max, speed_profile_compressed[i][j]);
            }
        }
        std::cout << "speed_profile_max: " << speed_profile_max << std::endl;

        // 转换speed_profile_compressed到图像数据范围(0-255)
        int ccnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                image_data_gray[ccnt++] = static_cast<unsigned char>(speed_profile_compressed[i][j] / speed_profile_max * 255);
            }
        }

        // 设置imshow参数并显示图像
        std::map<std::string, std::string> imshow_params = {
            {"cmap", "jet"},
            {"aspect", "auto"}
        };
        plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
        plt::title("Gray Scale Image");
        plt::show();

        // 释放动态分配的内存
        delete[] image_data_gray;
    }

    void plot_angle_profile_compressed(const std::vector<std::vector<double>>& angle_profile_compressed, int n_frames, int Q) {
        // 初始化图像数据数组
        int rows = n_frames;
        int columns = Q;
        unsigned char* image_data_gray = new unsigned char[rows * columns];

        // 查找angle_profile_compressed的最大值
        double angle_profile_max = 0;
        for (int i = 0; i < n_frames; ++i) {
            for (int j = 0; j < Q; ++j) {
                angle_profile_max = max(angle_profile_max, angle_profile_compressed[i][j]);
            }
        }
        std::cout << "angle_profile_max: " << angle_profile_max << std::endl;

        // 转换angle_profile_compressed到图像数据范围(0-255)
        int ccnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                image_data_gray[ccnt++] = static_cast<unsigned char>(angle_profile_compressed[i][j] / angle_profile_max * 255);
            }
        }

        // 设置imshow参数并显示图像
        std::map<std::string, std::string> imshow_params = {
            {"cmap", "jet"},
            {"aspect", "auto"}
        };
        plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
        plt::title("Gray Scale Image");
        plt::show();

        // 释放动态分配的内存
        delete[] image_data_gray;
    }

    /**
     * @brief 绘制压缩的数据的热图
     *
     * @param compressed_data 二维向量，包含压缩的数据
     * @param n_frames 数据的行数
     * @param N 数据的列数
     * @param cmap 颜色映射名称
     */
    void plot_compressed_data(const std::vector<std::vector<double>>& compressed_data, int n_frames, int N, const std::string& cmap = "jet") {
        // 初始化图像数据数组
        int rows = n_frames;
        int columns = N;
        unsigned char* image_data_gray = new unsigned char[rows * columns];

        // 查找compressed_data的最大值
        double max_value = 0;
        for (int i = 0; i < n_frames; ++i) {
            for (int j = 0; j < N; ++j) {
                max_value = max(max_value, compressed_data[i][j]);
            }
        }
        std::cout << "max_value: " << max_value << std::endl;

        // 转换compressed_data到图像数据范围(0-255)
        int ccnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                image_data_gray[ccnt++] = static_cast<unsigned char>(compressed_data[i][j] / max_value * 255);
            }
        }

        // 设置imshow参数并显示图像
        std::map<std::string, std::string> imshow_params = {
            {"cmap", cmap},
            {"aspect", "auto"}
        };
        plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
        plt::title("Gray Scale Image");
        plt::show();

        // 释放动态分配的内存
        delete[] image_data_gray;
    }

    // 从feature_vector中提取range_profile, speed_profile, angle_profile,然后进行图像绘制
    void plot_features(const std::vector<std::vector<float>>& feature_vector) {
        // 提取range_profile, speed_profile, angle_profile
        std::vector<std::vector<double>> range_profile(mmwave_handle::n_frames, std::vector<double>(mmwave_handle::N));
        std::vector<std::vector<double>> speed_profile(mmwave_handle::n_frames, std::vector<double>(mmwave_handle::M));
        std::vector<std::vector<double>> angle_profile(mmwave_handle::n_frames, std::vector<double>(mmwave_handle::Q));

        for (int i = 0; i < mmwave_handle::n_frames; ++i) {
            for (int j = 0; j < mmwave_handle::N; ++j) {
                range_profile[i][j] = feature_vector[0][i * mmwave_handle::N + j];
                speed_profile[i][j] = feature_vector[1][i * mmwave_handle::M + j];
                angle_profile[i][j] = feature_vector[2][i * mmwave_handle::Q + j];
            }
        }

        // 查找range_profile_compressed的最大值
        double range_profile_max = 0;
        for (int i = 0; i < mmwave_handle::n_frames; ++i) {
            for (int j = 0; j < mmwave_handle::N; ++j) {
                range_profile_max = max(range_profile_max, range_profile[i][j]);
            }
        }
        double speed_profile_max = 0;
        for (int i = 0; i < mmwave_handle::n_frames; ++i) {
            for (int j = 0; j < mmwave_handle::M; ++j) {
                speed_profile_max = max(speed_profile_max, speed_profile[i][j]);
            }
        }
        double angle_profile_max = 0;
        for (int i = 0; i < mmwave_handle::n_frames; ++i) {
            for (int j = 0; j < mmwave_handle::Q; ++j) {
                angle_profile_max = max(angle_profile_max, angle_profile[i][j]);
            }
        }

        // 初始化图像数据数组
        int rows = mmwave_handle::n_frames;
        int columns = mmwave_handle::N;
        unsigned char* image_data_gray = new unsigned char[rows * columns];

        // 转换range_profile到图像数据范围(0-255)
        int ccnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                image_data_gray[ccnt++] = static_cast<unsigned char>(range_profile[i][j] / range_profile_max * 255);
            }
        }
        plt::figure_size(1200, 400);
        plt::subplot(1, 3, 1);
        // 设置imshow参数并显示图像
        std::map<std::string, std::string> imshow_params = {
            {"cmap", "jet"},
            {"aspect", "auto"}
        };
        plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
        plt::title("range_profile");

        ccnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                image_data_gray[ccnt++] = static_cast<unsigned char>(speed_profile[i][j] / speed_profile_max * 255);
            }
        }
        plt::subplot(1, 3, 2);
        plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
        plt::title("speed_profile");

        ccnt = 0;
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                image_data_gray[ccnt++] = static_cast<unsigned char>(angle_profile[i][j] / angle_profile_max * 255);
            }
        }
        plt::subplot(1, 3, 3);

        plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
        plt::title("angle_profile");

        plt::show();

        // 释放动态分配的内存
        delete[] image_data_gray;



    }

    /**
     * @brief 对给定的雷达数据进行特征提取。
     *
     * @param data_radar       从二进制文件中读取的雷达数据。
     * @param clutter_removal  杂波去除方法，默认为空。
     * @param range_profile    存储range_profile的向量.
     * @param speed_profile    存储speed_profile的向量.
     * @param angle_profile    存储angle_profile的向量.
     *
     * @return 一个包含三个特征向量的二维向量，分别为range_profile_compressed, speed_profile_compressed, angle_profile_compressed。
     */
    std::vector<std::vector<float>> feature_extraction(const std::vector<std::vector<std::vector<std::vector<Complex>>>>& data_radar, const std::string& clutter_removal)
    {

        // 定义range_profile
        std::vector<std::vector<std::vector<fftw_complex*>>> range_profile(
            n_RX, std::vector<std::vector<fftw_complex*>>(
                n_chirps, std::vector<fftw_complex*>(n_frames)
            )
        );

        // 定义speed_profile
        std::vector<std::vector<std::vector<fftw_complex*>>> speed_profile(
            n_RX, std::vector<std::vector<fftw_complex*>>(
                N, std::vector<fftw_complex*>(n_frames)
            )
        );

        // 定义angle_profile
        std::vector<std::vector<std::vector<fftw_complex*>>> angle_profile(
            N, std::vector<std::vector<fftw_complex*>>(
                M, std::vector<fftw_complex*>(n_frames)
            )
        );

        // 创建in
        std::vector<std::vector<std::vector<fftw_complex*>>> in(
            n_RX, std::vector<std::vector<fftw_complex*>>(
                n_chirps, std::vector<fftw_complex*>(n_frames)
            )
        );

        // 为range_profile, speed_profile, angle_profile分配内存
        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_chirps; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    range_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
                }
            }
        }

        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < N; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    speed_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * M);
                }
            }
        }
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    angle_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * Q);
                }
            }
        }

        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_chirps; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    in[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n_samples);
                }
            }
        }

        // 将数据复制到in中
        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_chirps; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    for (int l = 0; l < n_samples; ++l) {
                        in[i][j][k][l][0] = data_radar[i][l][j][k].real();
                        in[i][j][k][l][1] = data_radar[i][l][j][k].imag();
                    }
                }
            }
        }

        // 执行FFT

        fftw_plan plan = fftw_plan_dft_1d(n_samples, in[0][0][0], range_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);

        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_chirps; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    fftw_execute_dft(plan, in[i][j][k], range_profile[i][j][k]);
                }
            }
        }
        fftw_destroy_plan(plan);

        // 杂波去除
        if (clutter_removal == "avg") {
            // [ ] 使用平均方法实现杂波去除
            cout << "杂波去除" << endl;
            // range_profile(n_RX, n_chirps, n_frames, N) -> range_profile(n_RX, n_chirps, n_frames, N)
            //range_profile = range_profile - np.mean(range_profile, axis = 3)[:, : , np.newaxis, : ]
            for (int i = 0; i < n_frames; ++i) {
                for (int j = 0; j < n_RX; ++j) {
                    for (int k = 0; k < N; ++k) {
                        double mean_real = 0;
                        double mean_imag = 0;
                        for (int l = 0; l < n_chirps; ++l) {
                            mean_real += range_profile[j][l][i][k][0];
                            mean_imag += range_profile[j][l][i][k][1];
                        }
                        mean_real /= n_chirps;
                        mean_imag /= n_chirps;
                        for (int l = 0; l < n_chirps; ++l) {
                            range_profile[j][l][i][k][0] -= mean_real;
                            range_profile[j][l][i][k][1] -= mean_imag;
                        }
                    }
                }
            }
        }
        else if (clutter_removal == "mti") {
            // [ ] 实现移动目标指示（MTI）滤波
        }

        // 速度轮廓的FFT(range_profile(n_RX, n_chirps, n_frames) -> speed_profile(n_RX, M, n_frames))
        plan = fftw_plan_dft_1d(M, speed_profile[0][0][0], speed_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_frames; ++j) {
                for (int k = 0; k < N; ++k) {
                    for (int l = 0; l < M; ++l) {
                        temp[l][0] = range_profile[i][l][j][k][0];
                        temp[l][1] = range_profile[i][l][j][k][1];
                    }
                    fftw_execute_dft(plan, temp, speed_profile[i][k][j]);
                    fftshift(speed_profile[i][k][j], M);
                }
            }
        }
        fftw_free(temp);

        plan = fftw_plan_dft_1d(Q, angle_profile[0][0][0], angle_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
        // 临时变量
        temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * M);

        // 角度轮廓的FFT(speed_profile(n_RX, N, n_frames) -> angle_profile(N, M, n_frames))
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < n_frames; ++j) {
                for (int k = 0; k < M; ++k) {
                    for (int l = 0; l < n_RX; ++l) {
                        temp[l][0] = speed_profile[l][i][j][k][0];
                        temp[l][1] = speed_profile[l][i][j][k][1];
                    }
                    fftw_execute_dft(plan, temp, angle_profile[i][k][j]);
                    fftshift(angle_profile[i][k][j], Q);
                }
            }
        }
        fftw_free(temp);

        // 压缩维度
        std::vector<std::vector<double>> range_profile_compressed(n_frames, std::vector<double>(N));
        std::vector<std::vector<double>> speed_profile_compressed(n_frames, std::vector<double>(M));
        std::vector<std::vector<double>> angle_profile_compressed(n_frames, std::vector<double>(Q));


        // 压缩range_profile
        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_chirps; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    for (int l = 0; l < N; ++l) {
                        range_profile_compressed[k][l] += abs(Complex(range_profile[i][j][k][l][0], range_profile[i][j][k][l][1]));
                    }
                }
            }
        }

        // 压缩speed_profile(n_RX, N, n_frames) -> speed_profile_compressed(n_frames, M)
        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < N; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    for (int l = 0; l < M; ++l) {
                        speed_profile_compressed[k][l] += abs(Complex(speed_profile[i][j][k][l][0], speed_profile[i][j][k][l][1]));
                    }
                }
            }
        }

        // 压缩angle_profile(N, M, n_frames) -> angle_profile_compressed(n_frames, Q)
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    for (int l = 0; l < Q; ++l) {
                        angle_profile_compressed[k][l] += abs(Complex(angle_profile[i][j][k][l][0], angle_profile[i][j][k][l][1]));
                    }
                }
            }
        }

        std::vector<std::vector<float>> feature_vector(3);
        for (int i = 0; i < n_frames; ++i) {
            for (int j = 0; j < N; ++j) {
                feature_vector[0].push_back(range_profile_compressed[i][j]);
            }
        }
        for (int i = 0; i < n_frames; ++i) {
            for (int j = 0; j < M; ++j) {
                feature_vector[1].push_back(speed_profile_compressed[i][j]);
            }
        }
        for (int i = 0; i < n_frames; ++i) {
            for (int j = 0; j < Q; ++j) {
                feature_vector[2].push_back(angle_profile_compressed[i][j]);
            }
        }

        //释放内存
        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_chirps; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    fftw_free(range_profile[i][j][k]);
                }
            }
        }
        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < N; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    fftw_free(speed_profile[i][j][k]);
                }
            }
        }
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    fftw_free(angle_profile[i][j][k]);
                }
            }
        }
        for (int i = 0; i < n_RX; ++i) {
            for (int j = 0; j < n_chirps; ++j) {
                for (int k = 0; k < n_frames; ++k) {
                    fftw_free(in[i][j][k]);
                }
            }
        }

        return feature_vector;
    }

    /**
    * @brief 对给定的二进制文件进行特征提取。
    *
    * @param filename 输入的二进制文件名。
    * @param clutter_removal 杂波去除方法，默认为空。
    * @return 一个包含三个特征向量的二维向量，分别为range_profile_compressed, speed_profile_compressed, angle_profile_compressed。
    */
    std::vector<std::vector<float>> feature_extraction_single(const string& filename, const string& clutter_removal = "")
    {
        // 定义data_radar
        std::vector<std::vector<std::vector<std::vector<Complex>>>> data_radar(
            n_RX, std::vector<std::vector<std::vector<Complex>>>(
                n_samples, std::vector<std::vector<Complex>>(
                    n_chirps, std::vector<Complex>(n_frames)
                )
            )
        );

        // 给data_radar赋值
        read_data(filename, data_radar);

        return feature_extraction(data_radar, clutter_removal);
    }
}


//int main() {
//    //从文件中提取data_radar
//    std::vector<std::vector<std::vector<std::vector<mmwave_handle::Complex>>>> data_radar;
//    mmwave_handle::read_data("K:/dataset/1000/1/1.bin", data_radar);
//    // 特征提取
//    std::vector<std::vector<float>> feature_vector = mmwave_handle::feature_extraction(data_radar, "avg");
//
//    mmwave_handle::plot_features(feature_vector);
//
//    return 0;
//}