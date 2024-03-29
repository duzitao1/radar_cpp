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
//// 导入matplotlibcpp库
//#include "matplotlibcpp.h"
//
//
//using namespace std;
//namespace plt = matplotlibcpp;
//
//typedef complex<double> Complex;  // 定义复数类型为双精度复数
//
// // 定义一个函数，用于从二进制文件中读取数据并转化成(n_RX, n_samples, n_chirps, n_frames)的形式
//void read_data(const string& filename, vector<vector<vector<vector<Complex>>>>& data_radar,
//    int n_RX = 4, int n_samples = 64, int n_chirps = 255, int n_frames = 30)
//{
//	// 读取文件
//	ifstream file(filename, ios::binary);
//    if (!file.is_open()) {
//		cerr << "打开文件错误！" << endl;
//		return;
//	}
//
//	vector<short> adcData(n_RX * 2 * n_samples * n_chirps * n_frames);
//	file.read(reinterpret_cast<char*>(adcData.data()), adcData.size() * sizeof(short));
//	file.close();
//
//	// 将adcData重塑为(numLanes*2, -1)
//	int newRows = n_RX * 2;
//	int newCols = adcData.size() / newRows;
//	std::vector<std::vector<short>> reshapedData(newRows, std::vector<short>(newCols));
//
//    for (int i = 0; i < adcData.size(); ++i) {
//		int row = i % (n_RX * 2);
//		int col = i / (n_RX * 2);
//		reshapedData[row][col] = adcData[i];
//	}
//
//	// 将reshapedData转换为复数形式
//	vector<vector<Complex>> combinedData(n_RX, vector<Complex>(newCols));
//    for (int i = 0; i < newCols; ++i) {
//        for (int j = 0; j < n_RX; ++j) {
//			combinedData[j][i] = Complex(reshapedData[j][i], reshapedData[j + 4][i]);
//		}
//	}
//
//	// 将数据复制到data_radar中
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_samples; ++j) {
//            for (int k = 0; k < n_chirps; ++k) {
//                for (int l = 0; l < n_frames; ++l) {
//					data_radar[i][j][k][l] = combinedData[i][j + k * n_samples + l * n_samples * n_chirps];
//				}
//			}
//		}
//	}
//}
//
//void fftshift(fftw_complex* &data, int N){
//	fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//    for (int i = 0; i < N; ++i) {
//		temp[i][0] = data[i][0];
//		temp[i][1] = data[i][1];
//	}
//    for (int i = 0; i < N; ++i) {
//		data[i][0] = temp[(i + N / 2) % N][0];
//		data[i][1] = temp[(i + N / 2) % N][1];
//	}
//}
//
//
//
//std::vector<std::vector<float>> feature_extraction_single(const string& filename, const string& clutter_removal = "",
//    int N = 64, int M = 64, int Q = 64, int numLanes = 4,
//    int n_RX = 4, int n_samples = 64, int n_chirps = 255, int n_frames = 30)
//{
//
//    // 定义data_radar
//    std::vector<std::vector<std::vector<std::vector<Complex>>>> data_radar(
//        n_RX, std::vector<std::vector<std::vector<Complex>>>(
//            n_samples, std::vector<std::vector<Complex>>(
//                n_chirps, std::vector<Complex>(n_frames)
//            )
//        )
//    );
//
//    // 定义range_profile
//    std::vector<std::vector<std::vector<fftw_complex*>>> range_profile(
//        n_RX, std::vector<std::vector<fftw_complex*>>(
//            n_chirps, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // 定义speed_profile
//    std::vector<std::vector<std::vector<fftw_complex*>>> speed_profile(
//        n_RX, std::vector<std::vector<fftw_complex*>>(
//            N, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // 定义angle_profile
//    std::vector<std::vector<std::vector<fftw_complex*>>> angle_profile(
//        N, std::vector<std::vector<fftw_complex*>>(
//            M, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // 为range_profile, speed_profile, angle_profile分配内存
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_chirps; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//				range_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//			}
//		}
//	}
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
//				angle_profile[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * Q);
//			}
//		}
//	}
//
//    // 给data_radar赋值
//    read_data(filename, data_radar, n_RX, n_samples, n_chirps, n_frames);
//
//    // 创建in
//    std::vector<std::vector<std::vector<fftw_complex*>>> in(
//        n_RX, std::vector<std::vector<fftw_complex*>>(
//            n_chirps, std::vector<fftw_complex*>(n_frames)
//        )
//    );
//
//    // 分配内存
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_chirps; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                in[i][j][k] = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * n_samples);
//            }
//        }
//    }
//
//    // 将数据复制到in中
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
//    // 执行FFT
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
//       // 杂波去除
//    if (clutter_removal == "avg") {
//        // [ ] 使用平均方法实现杂波去除
//        cout << "杂波去除" << endl;
//        // range_profile(n_RX, n_chirps, n_frames, N) -> range_profile(n_RX, n_chirps, n_frames, N)
//        //range_profile = range_profile - np.mean(range_profile, axis = 3)[:, : , np.newaxis, : ]
//        for (int i = 0; i < n_frames; ++i) {
//            for (int j = 0; j < n_RX; ++j) {
//                for (int k = 0; k < N; ++k) {
//                    double mean_real = 0;
//                    double mean_imag = 0;
//                    for (int l = 0; l < n_chirps; ++l) {
//                        mean_real += range_profile[j][l][i][k][0];
//						mean_imag += range_profile[j][l][i][k][1];
//					}
//					mean_real /= n_chirps;
//                    mean_imag /= n_chirps;
//                    for (int l = 0; l < n_chirps; ++l) {
//                        range_profile[j][l][i][k][0] -= mean_real;
//						range_profile[j][l][i][k][1] -= mean_imag;
//					}
//                }
//            }
//        }
//    }
//    else if (clutter_removal == "mti") {
//        // [ ] 实现移动目标指示（MTI）滤波
//    }
//    
//    // 速度轮廓的FFT(range_profile(n_RX, n_chirps, n_frames) -> speed_profile(n_RX, M, n_frames))
//    plan = fftw_plan_dft_1d(M, speed_profile[0][0][0], speed_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//    fftw_complex* temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < n_frames; ++j) {
//            for (int k = 0; k < N; ++k) {
//                for (int l = 0; l < M; ++l) {
//					temp[l][0] = range_profile[i][l][j][k][0];
//                    temp[l][1] = range_profile[i][l][j][k][1];
//				}
//                fftw_execute_dft(plan, temp, speed_profile[i][k][j]);
//                fftshift(speed_profile[i][k][j], M);
//			}
//		}
//    }
//
//  //  // 使用matplotlibcpp绘制FFT结果的绝对值的折线图
//  //  std::vector<double> x(M), y(M);
//  //  for (int i = 0; i < M; i++) {
//  //      x[i] = i;
//  //  }
//  //  for (int i = 0; i < M; i++) {
//  //      y[i] = abs(Complex(speed_profile[0][0][0][i][0], speed_profile[0][0][0][i][1]));
//  //  }
//  //  plt::plot(x, y);
//  //  plt::show();
//       // 角度轮廓的FFT
//       // [ ] 实现角度轮廓的FFT和其他操作
//    plan = fftw_plan_dft_1d(Q, angle_profile[0][0][0], angle_profile[0][0][0], FFTW_FORWARD, FFTW_ESTIMATE);
//    // 临时变量
//    temp = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * M);
//
//    // 角度轮廓的FFT(speed_profile(n_RX, N, n_frames) -> angle_profile(N, M, n_frames))
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < n_frames; ++j) {
//            for (int k = 0; k < M; ++k) {
//                for (int l = 0; l < n_RX; ++l) {
//					temp[l][0] = speed_profile[l][i][j][k][0];
//					temp[l][1] = speed_profile[l][i][j][k][1];
//				}
//				fftw_execute_dft(plan, temp, angle_profile[i][k][j]);
//                fftshift(angle_profile[i][k][j], Q);
//            }
//        }
//    }
//
//
//    // 压缩维度
//    std::vector<std::vector<double>> range_profile_compressed(n_frames, std::vector<double>(N));
//    std::vector<std::vector<double>> speed_profile_compressed(n_frames, std::vector<double>(M));
//    std::vector<std::vector<double>> angle_profile_compressed(n_frames, std::vector<double>(Q));
//
//
//    // 压缩range_profile
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
//    // 使用matplotlibcpp绘制range_profile_compressed
// //   std::vector<double> x(N), y(N);
// //   for (int i = 0; i < N; i++) {
//	//	x[i] = i;
//	//}
// //   for (int i = 0; i < N; i++) {
//	//	y[i] = range_profile_compressed[0][i];
//	//}
//	//plt::plot(x, y);
//	//plt::show();
//    //仿照上面的方法,使用ion()和draw()实现动态绘制
//    //plt::ion();
//    //plt::figure_size(1200, 780);
//    //for (int i = 0; i < n_frames; ++i) {
//    //    for (int j = 0; j < N; ++j) {
//		  //      y[j] = range_profile_compressed[i][j];
//	   // }
//	   // plt::plot(x, y);
//    //    plt::pause(0.2);
//    //}
//    //plt::pause(10);
//
//    // 压缩speed_profile(n_RX, N, n_frames) -> speed_profile_compressed(n_frames, M)
//    for (int i = 0; i < n_RX; ++i) {
//        for (int j = 0; j < N; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                for (int l = 0; l < M; ++l) {
//					speed_profile_compressed[k][l] += abs(Complex(speed_profile[i][j][k][l][0], speed_profile[i][j][k][l][1]));
//				}
//			}
//		}
//	}
//
//    // 压缩angle_profile(N, M, n_frames) -> angle_profile_compressed(n_frames, Q)
//    for (int i = 0; i < N; ++i) {
//        for (int j = 0; j < M; ++j) {
//            for (int k = 0; k < n_frames; ++k) {
//                for (int l = 0; l < Q; ++l) {
//					angle_profile_compressed[k][l] += abs(Complex(angle_profile[i][j][k][l][0], angle_profile[i][j][k][l][1]));
//				}
//            }
//        }
//    }
//
//    // 使用matplotlibcpp绘制range_profile_compressed热图
//    int rows = n_frames;
//    int columns = N;
//    unsigned char* image_data_gray = new unsigned char[rows * columns];
//
//    // 将range_profile_compressed转换为image_data(0-255)
//    double range_profile_max = 0;
//    for (int i = 0; i < n_frames; ++i) {
//        for (int j = 0; j < N; ++j) {
//			range_profile_max = max(range_profile_max, range_profile_compressed[i][j]);
//		}
//	}
//    cout << "range_profile_max: " << range_profile_max << endl;
//    int ccnt = 0;
//    for (int i = 0; i < rows; ++i) {
//        for (int j = 0; j < columns; ++j) {
//		image_data_gray[ccnt++] = static_cast<int>(range_profile_compressed[i][j])/range_profile_max*255;
//	    }
//	}
//    std::map<std::string, std::string> imshow_params;
//    imshow_params["cmap"] = "jet"; // 设置颜色映射为jet
//    imshow_params["aspect"] = "auto"; // 设置纵横比为auto
//    plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
//    plt::title("Gray Scale Image");
//    plt::show();
//
// //   // 将speed_profile_compressed转换为image_data(0-255)
// //   rows = n_frames;
// //   columns = M;
// //   image_data_gray = new unsigned char[rows * columns];
//
// //   double speed_profile_max = 0;
// //   for (int i = 0; i < n_frames; ++i) {
// //       for (int j = 0; j < M; ++j) {
// //           speed_profile_max = max(speed_profile_max, speed_profile_compressed[i][j]);
// //       }
// //   }
// //   cout << "speed_profile_max: " << speed_profile_max << endl;
// //   ccnt = 0;
// //   for (int i = 0; i < rows; ++i) {
// //       for (int j = 0; j < columns; ++j) {
//	//		image_data_gray[ccnt++] = static_cast<int>(speed_profile_compressed[i][j]) / speed_profile_max * 255;
//	//	}
//	//}
// //   plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
// //   plt::title("Gray Scale Image");
// //   plt::show();
//
// //   // 将angle_profile_compressed转换为image_data(0-255)
// //   rows = n_frames;
// //   columns = Q;
// //   image_data_gray = new unsigned char[rows * columns];
//
// //   double angle_profile_max = 0;
// //   for (int i = 0; i < n_frames; ++i) {
// //       for (int j = 0; j < Q; ++j) {
//	//		angle_profile_max = max(angle_profile_max, angle_profile_compressed[i][j]);
//	//	}
//	//}
// //   cout << "angle_profile_max: " << angle_profile_max << endl;
// //   ccnt = 0;
// //   for (int i = 0; i < rows; ++i) {
// //       for (int j = 0; j < columns; ++j) {
// //           image_data_gray[ccnt++] = static_cast<int>(angle_profile_compressed[i][j]) / angle_profile_max * 255;
// //       }
// //   }
// //   plt::imshow(image_data_gray, rows, columns, 1, imshow_params);
// //   plt::title("Gray Scale Image");
// //   plt::show();
//
//    // 将range_profile_compressed, speed_profile_compressed, angle_profile_compressed先变成一维数组，再存入feature_vector
//    std::vector<std::vector<float>> feature_vector(3);
//    for (int i = 0; i < n_frames; ++i) {
//        for (int j = 0; j < N; ++j) {
//			feature_vector[0].push_back(range_profile_compressed[i][j]);
//		}
//	}
//    for (int i = 0; i < n_frames; ++i) {
//        for (int j = 0; j < M; ++j) {
//            feature_vector[1].push_back(speed_profile_compressed[i][j]);
//        }
//    }
//    for (int i = 0; i < n_frames; ++i) {
//        for (int j = 0; j < Q; ++j) {
//			feature_vector[2].push_back(angle_profile_compressed[i][j]);
//		}
//	}
//    // 绘制存储在feature_vector中的range_profile_compressed
//    plt::figure_size(1200, 780);
//    plt::plot(feature_vector[0]);
//    plt::show();
//
//
//
//    return feature_vector;
//
//
//
//
//
//}
//
//int main() {
//    //feature_extraction_single("K:/dataset/2024_3_22/1/1.bin", "avg");
//    vector<vector<float>> feature_vector = feature_extraction_single("K:/dataset/2024_3_22/1/1.bin", "avg");
//
//
//
//
//    return 0;
//}
