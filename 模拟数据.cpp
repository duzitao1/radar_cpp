//// ��Ȩ���� (c) Microsoft Corporation����������Ȩ����
//// ���� MIT ���֤�����ɡ�
//
///**
// * ��ʾ��Ӧ�ó�����ʾ���ʹ��ʵ���Ե� C++ API ���
// * ��ѯģ�͵�����/����Լ������ģ������������
// *
// * ��ʾ������� onnx ģ�Ϳ��е� ResNet ģ�ͣ����� ResNet18��һ�����У�
// * �ÿ�λ�� https://github.com/onnx/models
// *
// * ��ʾ���������ļ��裺
// *  1) onnx ģ���� 1 ������ڵ�� 1 ������ڵ�
// *  2) onnx ģ��Ӧ���� float ����
// *
// * �ڴ�ʾ���У������������²�����
// *  1) ��ȡһ�� onnx ģ��
// *  2) ��ӡ��ģ������������������һЩԪ������Ϣ
// *  3) Ϊ�������������������
// *  4) ������ͨ��ģ�Ͳ����������
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
// // ���۵ش�ӡ��״ά������
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
//    // ֱ��д��ģ��·��
//    //std::basic_string<ORTCHAR_T> model_file = L"K:/mnist.onnx";
//    std::basic_string<ORTCHAR_T> model_file = L"K:/aio_radar/model.onnx";
//
//
//    // onnxruntime ����
//    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "example-model-explorer");
//    Ort::SessionOptions session_options;
//    Ort::Session session = Ort::Session(env, model_file.c_str(), session_options);
//
//    // ��ӡ���������/��״
//    Ort::AllocatorWithDefaultOptions allocator;
//    std::vector<std::string> input_names;
//    std::vector<std::int64_t> input_shapes;
//    std::cout << "����ڵ�����/��״ (" << input_names.size() << "):" << std::endl;
//    for (std::size_t i = 0; i < session.GetInputCount(); i++) {
//        input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
//        input_shapes = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
//        std::cout << "\t" << input_names.at(i) << " : " << print_shape(input_shapes) << std::endl;
//    }
//    // һЩģ�Ϳ����и���״ֵ��ָʾ��̬��״�����磬���ڿɱ�����δ�С��
//    for (auto& s : input_shapes) {
//        if (s < 0) {
//            s = 1;
//        }
//    }
//
//    // ��ӡ���������/��״
//    std::vector<std::string> output_names;
//    std::cout << "����ڵ�����/��״ (" << output_names.size() << "):" << std::endl;
//    for (std::size_t i = 0; i < session.GetOutputCount(); i++) {
//        output_names.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
//        auto output_shapes = session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
//        std::cout << "\t" << output_names.at(i) << " : " << print_shape(output_shapes) << std::endl;
//    }
//
//    // ����ģ���� 1 ������ڵ�� 1 ������ڵ㡣
//    assert(input_names.size() == 1 && output_names.size() == 1);
//
//    // ����һ������������ֵĵ��� Ort ����
//    auto input_shape = input_shapes;
//    auto total_number_elements = calculate_product(input_shape);
//
//    // �ڷ�Χ [0, 255] �������������
//    std::vector<float> input_tensor_values(total_number_elements);
//    std::generate(input_tensor_values.begin(), input_tensor_values.end(), [&] { return rand() % 255; });
//    std::vector<Ort::Value> input_tensors;
//    input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values, input_shape));
//
//    // �ٴμ������������ά��
//    assert(input_tensors[0].IsTensor() && input_tensors[0].GetTensorTypeAndShapeInfo().GetShape() == input_shape);
//    std::cout << "\n����������״: " << print_shape(input_tensors[0].GetTensorTypeAndShapeInfo().GetShape()) << std::endl;
//
//    // ͨ��ģ�ʹ�������
//    std::vector<const char*> input_names_char(input_names.size(), nullptr);
//    std::transform(std::begin(input_names), std::end(input_names), std::begin(input_names_char),
//        [&](const std::string& str) { return str.c_str(); });
//
//    std::vector<const char*> output_names_char(output_names.size(), nullptr);
//    std::transform(std::begin(output_names), std::end(output_names), std::begin(output_names_char),
//        [&](const std::string& str) { return str.c_str(); });
//
//    std::cout << "����ģ��..." << std::endl;
//    try {
//        auto output_tensors = session.Run(Ort::RunOptions{ nullptr }, input_names_char.data(), input_tensors.data(),
//            input_names_char.size(), output_names_char.data(), output_names_char.size());
//        std::cout << "Done!" << std::endl;
//
//        // �ٴμ�����������ά��
//        // ע�⣺������������������� Run() ������ָ��������ڵ�����
//        assert(output_tensors.size() == output_names.size() && output_tensors[0].IsTensor());
//    }
//    catch (const Ort::Exception& exception) {
//        std::cout << "ERROR running model inference: " << exception.what() << std::endl;
//        exit(-1);
//    }
//}
