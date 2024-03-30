#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <onnxruntime_cxx_api.h>

// ���۵ش�ӡ��״ά������
std::string print_shape(const std::vector<std::int64_t>& v) {
	std::stringstream ss("");
	for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
	ss << v[v.size() - 1];
	return ss.str();
}

/**
 * @brief �����������������Ԫ�صĳ˻���
 *
 * @param v ��������������
 * @return ��������������Ԫ�صĳ˻���
 */
int calculate_product(const std::vector<std::int64_t>& v) {
	int total = 1;
	for (auto& i : v) total *= i;
	return total;
}

/**
 * @brief ������������ת��ΪONNX��������
 *
 * @tparam T �����е��������͡�
 * @param data Ҫת��������������
 * @param shape ��������״��
 * @return ����ת�����ONNX������
 */
template <typename T>
Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
	// ���������������ڴ���Ϣ
	Ort::MemoryInfo mem_info =
		Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

	// ʹ�ø��������ݺ���״��������
	auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());

	return tensor;
}

// ��ӡ���������ڵ�����/��״�ĺ���
void print_node_shapes(Ort::Session& session) {
	Ort::AllocatorWithDefaultOptions allocator;
	std::vector<std::string> input_names;
	std::vector<std::int64_t> input_shapes;
	std::cout << "����ڵ�����/��״ (" << input_names.size() << "):" << std::endl;
	for (std::size_t i = 0; i < session.GetInputCount(); i++) {
		input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
		input_shapes = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
		std::cout << "\t" << input_names.at(i) << " : " << print_shape(input_shapes) << std::endl;
	}

	std::vector<std::string> output_names;
	std::cout << "����ڵ�����/��״ (" << output_names.size() << "):" << std::endl;
	for (std::size_t i = 0; i < session.GetOutputCount(); i++) {
		output_names.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
		auto output_shapes = session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
		std::cout << "\t" << output_names.at(i) << " : " << print_shape(output_shapes) << std::endl;
	}
}

// ��������ȡ�Ĵ����װΪ����
float* run_model_with_input(Ort::Session& session, const std::vector<std::string>& input_names,
	std::vector<std::vector<float>> input_tensor_values_list,
	const std::vector<std::int64_t>& input_shape,
	const std::vector<std::string>& output_names) {
	std::vector<Ort::Value> input_tensors;
	for (int i = 0; i < 3; ++i) {
		input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values_list[i], input_shape));
	}

	// ͨ��ģ�ʹ�������
	std::vector<const char*> input_names_char(input_names.size(), nullptr);
	std::transform(std::begin(input_names), std::end(input_names), std::begin(input_names_char),
		[&](const std::string& str) { return str.c_str(); });

	std::vector<const char*> output_names_char(output_names.size(), nullptr);
	std::transform(std::begin(output_names), std::end(output_names), std::begin(output_names_char),
		[&](const std::string& str) { return str.c_str(); });

	auto output_tensors = session.Run(Ort::RunOptions{ nullptr }, input_names_char.data(), input_tensors.data(),
		input_names_char.size(), output_names_char.data(), output_names_char.size());

	// �ٴμ�����������ά��
	assert(output_tensors.size() == output_names.size() && output_tensors[0].IsTensor());

	// ��ӡ�������
	auto output_data = output_tensors[0].GetTensorMutableData<float>();
	for (int i = 0; i < 10; ++i) {
		std::cout << output_data[i] << " ";
	}
	std::cout << std::endl;

	return output_data;
}