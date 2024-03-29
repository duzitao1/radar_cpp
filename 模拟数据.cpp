//// 版权所有 (c) Microsoft Corporation。保留所有权利。
//// 根据 MIT 许可证获得许可。
//
///**
// * 本示例应用程序演示如何使用实验性的 C++ API 组件
// * 查询模型的输入/输出以及如何在模型上运行推理。
// *
// * 此示例最好与 onnx 模型库中的 ResNet 模型（例如 ResNet18）一起运行，
// * 该库位于 https://github.com/onnx/models
// *
// * 此示例中做出的假设：
// *  1) onnx 模型有 1 个输入节点和 1 个输出节点
// *  2) onnx 模型应该有 float 输入
// *
// * 在此示例中，我们做了以下操作：
// *  1) 读取一个 onnx 模型
// *  2) 打印出模型期望的输入和输出的一些元数据信息
// *  3) 为输入张量生成随机数据
// *  4) 将张量通过模型并检查结果张量
// *
// */
//
//#include <algorithm>  // std::generate
//#include <cassert>
//#include <cstddef>
//#include <cstdint>
//#include <iostream>
//#include <sstream>
//#include <string>
//#include <vector>
//#include <onnxruntime_cxx_api.h>
//
// // 美观地打印形状维度向量
//std::string print_shape(const std::vector<std::int64_t>& v) {
//    std::stringstream ss("");
//    for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
//    ss << v[v.size() - 1];
//    return ss.str();
//}
//
//int calculate_product(const std::vector<std::int64_t>& v) {
//    int total = 1;
//    for (auto& i : v) total *= i;
//    return total;
//}
//
//template <typename T>
//Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
//    Ort::MemoryInfo mem_info =
//        Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);
//    auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());
//    return tensor;
//}
//
//#ifdef _WIN32
//int wmain(int argc, ORTCHAR_T* argv[]) {
//#else
//int main(int argc, ORTCHAR_T * argv[]) {
//#endif
//    //if (argc != 2) {
//    //    std::cout << "Usage: ./onnx-api-example <onnx_model.onnx>" << std::endl;
//    //    return -1;
//    //}
//
//    //std::basic_string<ORTCHAR_T> model_file = argv[1];
//
//    // 直接写死模型路径
//    //std::basic_string<ORTCHAR_T> model_file = L"K:/mnist.onnx";
//    std::basic_string<ORTCHAR_T> model_file = L"K:/aio_radar/model.onnx";
//
//
//    // onnxruntime 设置
//    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
//    Ort::SessionOptions session_options;
//    Ort::Session session = Ort::Session(env, model_file.c_str(), session_options);
//
//    // 打印输入的名称/形状
//    Ort::AllocatorWithDefaultOptions allocator;
//    std::vector<std::string> input_names;
//    std::vector<std::int64_t> input_shapes;
//    std::cout << "输入节点名称/形状 (" << input_names.size() << "):" << std::endl;
//    for (std::size_t i = 0; i < session.GetInputCount(); i++) {
//        input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
//        input_shapes = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
//        std::cout << "\t" << input_names.at(i) << " : " << print_shape(input_shapes) << std::endl;
//    }
//    // 一些模型可能有负形状值以指示动态形状，例如，对于可变的批次大小。
//    for (auto& s : input_shapes) {
//        if (s < 0) {
//            s = 1;
//        }
//    }
//
//    // 打印输出的名称/形状
//    std::vector<std::string> output_names;
//    std::cout << "输出节点名称/形状 (" << output_names.size() << "):" << std::endl;
//    for (std::size_t i = 0; i < session.GetOutputCount(); i++) {
//        output_names.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
//        auto output_shapes = session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
//        std::cout << "\t" << output_names.at(i) << " : " << print_shape(output_shapes) << std::endl;
//    }
//
//    // 假设模型有 1 个输入节点和 1 个输出节点。
//    assert(input_names.size() == 1 && output_names.size() == 1);
//
//    // 创建一个包含随机数字的单个 Ort 张量
//    auto input_shape = input_shapes;
//    auto total_number_elements = calculate_product(input_shape);
//
//    // 在范围 [0, 255] 中生成随机数字
//    std::vector<float> input_tensor_values(total_number_elements);
//    std::generate(input_tensor_values.begin(), input_tensor_values.end(), [&] { return rand() % 255; });
//    std::vector<Ort::Value> input_tensors;
//    input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values, input_shape));
//
//    // 再次检查输入张量的维度
//    assert(input_tensors[0].IsTensor() && input_tensors[0].GetTensorTypeAndShapeInfo().GetShape() == input_shape);
//    std::cout << "\n输入张量形状: " << print_shape(input_tensors[0].GetTensorTypeAndShapeInfo().GetShape()) << std::endl;
//
//    // 通过模型传递数据
//    std::vector<const char*> input_names_char(input_names.size(), nullptr);
//    std::transform(std::begin(input_names), std::end(input_names), std::begin(input_names_char),
//        [&](const std::string& str) { return str.c_str(); });
//
//    std::vector<const char*> output_names_char(output_names.size(), nullptr);
//    std::transform(std::begin(output_names), std::end(output_names), std::begin(output_names_char),
//        [&](const std::string& str) { return str.c_str(); });
//
//    std::cout << "运行模型..." << std::endl;
//    try {
//        auto output_tensors = session.Run(Ort::RunOptions{ nullptr }, input_names_char.data(), input_tensors.data(),
//            input_names_char.size(), output_names_char.data(), output_names_char.size());
//        std::cout << "Done!" << std::endl;
//
//        // 再次检查输出张量的维度
//        // 注意：输出张量的数量等于在 Run() 调用中指定的输出节点数量
//        assert(output_tensors.size() == output_names.size() && output_tensors[0].IsTensor());
//    }
//    catch (const Ort::Exception& exception) {
//        std::cout << "ERROR running model inference: " << exception.what() << std::endl;
//        exit(-1);
//    }
//}
