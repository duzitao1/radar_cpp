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

// 美观地打印形状维度向量
std::string print_shape(const std::vector<std::int64_t>& v) {
	std::stringstream ss("");
	for (std::size_t i = 0; i < v.size() - 1; i++) ss << v[i] << "x";
	ss << v[v.size() - 1];
	return ss.str();
}

/**
 * @brief 计算给定向量中所有元素的乘积。
 *
 * @param v 输入整数向量。
 * @return 返回向量中所有元素的乘积。
 */
int calculate_product(const std::vector<std::int64_t>& v) {
	int total = 1;
	for (auto& i : v) total *= i;
	return total;
}

/**
 * @brief 将给定的向量转换为ONNX的张量。
 *
 * @tparam T 向量中的数据类型。
 * @param data 要转换的数据向量。
 * @param shape 张量的形状。
 * @return 返回转换后的ONNX张量。
 */
template <typename T>
Ort::Value vec_to_tensor(std::vector<T>& data, const std::vector<std::int64_t>& shape) {
	// 创建用于张量的内存信息
	Ort::MemoryInfo mem_info =
		Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

	// 使用给定的数据和形状创建张量
	auto tensor = Ort::Value::CreateTensor<T>(mem_info, data.data(), data.size(), shape.data(), shape.size());

	return tensor;
}

// 打印输入和输出节点名称/形状的函数
void print_node_shapes(Ort::Session& session) {
	Ort::AllocatorWithDefaultOptions allocator;
	std::vector<std::string> input_names;
	std::vector<std::int64_t> input_shapes;
	std::cout << "输入节点名称/形状 (" << input_names.size() << "):" << std::endl;
	for (std::size_t i = 0; i < session.GetInputCount(); i++) {
		input_names.emplace_back(session.GetInputNameAllocated(i, allocator).get());
		input_shapes = session.GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
		std::cout << "\t" << input_names.at(i) << " : " << print_shape(input_shapes) << std::endl;
	}

	std::vector<std::string> output_names;
	std::cout << "输出节点名称/形状 (" << output_names.size() << "):" << std::endl;
	for (std::size_t i = 0; i < session.GetOutputCount(); i++) {
		output_names.emplace_back(session.GetOutputNameAllocated(i, allocator).get());
		auto output_shapes = session.GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
		std::cout << "\t" << output_names.at(i) << " : " << print_shape(output_shapes) << std::endl;
	}
}

// 将特征提取的代码封装为函数
float* run_model_with_input(Ort::Session& session, const std::vector<std::string>& input_names,
	std::vector<std::vector<float>> input_tensor_values_list,
	const std::vector<std::int64_t>& input_shape,
	const std::vector<std::string>& output_names) {
	std::vector<Ort::Value> input_tensors;
	for (int i = 0; i < 3; ++i) {
		input_tensors.emplace_back(vec_to_tensor<float>(input_tensor_values_list[i], input_shape));
	}

	// 通过模型传递数据
	std::vector<const char*> input_names_char(input_names.size(), nullptr);
	std::transform(std::begin(input_names), std::end(input_names), std::begin(input_names_char),
		[&](const std::string& str) { return str.c_str(); });

	std::vector<const char*> output_names_char(output_names.size(), nullptr);
	std::transform(std::begin(output_names), std::end(output_names), std::begin(output_names_char),
		[&](const std::string& str) { return str.c_str(); });

	auto output_tensors = session.Run(Ort::RunOptions{ nullptr }, input_names_char.data(), input_tensors.data(),
		input_names_char.size(), output_names_char.data(), output_names_char.size());

	// 再次检查输出张量的维度
	assert(output_tensors.size() == output_names.size() && output_tensors[0].IsTensor());

	// 打印输出张量
	auto output_data = output_tensors[0].GetTensorMutableData<float>();
	for (int i = 0; i < 10; ++i) {
		std::cout << output_data[i] << " ";
	}
	std::cout << std::endl;

	return output_data;
}