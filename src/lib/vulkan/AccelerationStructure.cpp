/*
 * AccelerationStructure.cpp
 *
 *  Created on: 12 Oct 2020
 *      Author: michi
 */

#if HAS_LIB_VULKAN

#include "AccelerationStructure.h"
#include "helper.h"
#include "common.h"
#include "../math/mat4.h"
#include "../os/msg.h"

namespace vulkan {


AccelerationStructure::AccelerationStructure(Device *_device, const VkAccelerationStructureTypeNV type, const Array<VkGeometryNV> &geo, const uint32_t instanceCount) {
	if (verbosity >= 2)
		msg_write(format(" + AccStruc  inst=%d  geo=%d", instanceCount, geo.num));
	device = _device->device;

	info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	info.type = type;
	info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_NV;
	info.instanceCount = instanceCount;
	info.geometryCount = geo.num;
	info.pGeometries = &geo[0];
	VkAccelerationStructureCreateInfoNV ci = {};
	ci.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	ci.compactedSize = 0;
	ci.info = info;

	VkResult error = _vkCreateAccelerationStructureNV(device, &ci, nullptr, &structure);
	if (VK_SUCCESS != error)
		throw Exception("failed to create acceleration structure");

	VkAccelerationStructureMemoryRequirementsInfoNV memory_requirements_info = {};
	memory_requirements_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	memory_requirements_info.accelerationStructure = structure;
	memory_requirements_info.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;

	VkMemoryRequirements2 memoryRequirements;
	_vkGetAccelerationStructureMemoryRequirementsNV(device, &memory_requirements_info, &memoryRequirements);

	VkMemoryAllocateInfo memory_allocate_info = {};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memoryRequirements.memoryRequirements.size;
	memory_allocate_info.memoryTypeIndex = _device->find_memory_type(memoryRequirements.memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	error = vkAllocateMemory(device, &memory_allocate_info, nullptr, &memory);
	if (VK_SUCCESS != error)
		throw Exception("failed to allocate memory");

	VkBindAccelerationStructureMemoryInfoNV bind_info = {};
	bind_info.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	bind_info.accelerationStructure = structure;
	bind_info.memory = memory;
	bind_info.memoryOffset = 0;
	bind_info.deviceIndexCount = 0;
	bind_info.pDeviceIndices = nullptr;

	error = _vkBindAccelerationStructureMemoryNV(device, 1, &bind_info);
	if (VK_SUCCESS != error)
		throw Exception("failed to bind acceleration structure");

	error = _vkGetAccelerationStructureHandleNV(device, structure, sizeof(uint64_t), &handle);
	if (VK_SUCCESS != error)
		throw Exception("failed to get acceleration structure handle");
	if (verbosity >= 2)
		msg_write("handle: " + i2s(handle));
}

AccelerationStructure::~AccelerationStructure() {
	_vkDestroyAccelerationStructureNV(device, structure, nullptr);
	structure = VK_NULL_HANDLE;
	vkFreeMemory(device, memory, nullptr);
	memory = VK_NULL_HANDLE;
}

void AccelerationStructure::build(const Array<VkGeometryNV> &geo, const Array<VkAccelerationStructureInstanceKHR> &instances, bool update) {
	if (verbosity >= 4)
		msg_write("   AccStr build");

	Buffer instances_buffer(default_device);
	if (instances.num > 0) {
		if (verbosity >= 4) {
			msg_write(p2s(&instances));
			msg_write(format("instance buffer %d*%d", instances.num, instances.element_size));
		}
		instances_buffer.create(instances.num * instances.element_size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		instances_buffer.update(instances.data);

		info.instanceCount = static_cast<uint32_t>(instances.num);
		info.geometryCount = geo.num;
		info.pGeometries = &geo[0];
	}




	VkAccelerationStructureMemoryRequirementsInfoNV mri;
	mri.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	mri.pNext = nullptr;
	mri.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
	mri.accelerationStructure = structure;

	VkMemoryRequirements2 mem_req = {};
	mem_req.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
	_vkGetAccelerationStructureMemoryRequirementsNV(device, &mri, &mem_req);

	Buffer scratch(default_device);
	scratch.create(mem_req.memoryRequirements.size, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	auto cb = begin_single_time_commands();

	_vkCmdBuildAccelerationStructureNV(cb->buffer, &info,
							instances_buffer.buffer, 0, update, //VK_FALSE,
							structure, VK_NULL_HANDLE,
							scratch.buffer, 0);

	// multiple needs a memory barrier
	VkMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	vkCmdPipelineBarrier(cb->buffer, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 1, &barrier, 0, 0, 0, 0);
	end_single_time_commands(cb);
}


static Array<VkGeometryNV> create_geometries(VertexBuffer *vb) {
	Array<VkGeometryNV> geo;

	//for (int i=0; i<vb.num; i++) {
		if (verbosity >= 4)
			msg_write(format("AS vertices=%d stride=%d", vb->vertex_count, vb->stride()));
		VkGeometryNV geometry = {};
		geometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		geometry.geometry.triangles.vertexData = vb->vertex_buffer.buffer;
		geometry.geometry.triangles.vertexOffset = 0;
		geometry.geometry.triangles.vertexCount = vb->vertex_count;
		geometry.geometry.triangles.vertexStride = vb->stride();
		geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		if (vb->is_indexed()) {
			if (verbosity >= 3)
				msg_write("AS indexed");
			geometry.geometry.triangles.indexData = vb->index_buffer.buffer;
			geometry.geometry.triangles.indexOffset = 0;
			geometry.geometry.triangles.indexCount = vb->output_count;
			geometry.geometry.triangles.indexType = vb->index_type;
		}
		geometry.geometry.triangles.transformData = VK_NULL_HANDLE;
		geometry.geometry.triangles.transformOffset = 0;
		geometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
		geo.add(geometry);
		if (verbosity >= 3)
			msg_write(vb->output_count);
	//}
	return geo;
}

static Array<VkAccelerationStructureInstanceKHR> create_instances(const Array<AccelerationStructure*> &blas, const Array<mat4> &matrices) {
    Array<VkAccelerationStructureInstanceKHR> instances;
    instances.resize(blas.num);

	int triangle_offset = 0;
    for (int i = 0; i < blas.num; i++) {
        auto &instance = instances[i];
        memcpy(&instance.transform, &matrices[i], 12*sizeof(float));
        instance.instanceCustomIndex = static_cast<uint32_t>(triangle_offset);
        instance.mask = 0xff;
        instance.instanceShaderBindingTableRecordOffset = 0;
        instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
        instance.accelerationStructureReference = blas[i]->handle;
		triangle_offset += blas[i]->triangle_count;
    }
	return instances;
}

void AccelerationStructure::update_top(const Array<AccelerationStructure*> &blas, const Array<mat4> &matrices) {
	auto instances = create_instances(blas, matrices);
	build({}, instances, true);
}


AccelerationStructure *AccelerationStructure::create_bottom(Device *device, VertexBuffer *vb) {
	auto geometries = create_geometries(vb);
	auto as = new AccelerationStructure(device, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV, geometries, 0);
	as->triangle_count = vb->output_count / 3;
	as->build(geometries, {}, false);
	return as;
}


AccelerationStructure *AccelerationStructure::create_top(Device *device, const Array<AccelerationStructure*> &blas, const Array<mat4> &matrices) {
	auto instances = create_instances(blas, matrices);
	auto as = new AccelerationStructure(device, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV, {}, instances.num);
	as->build({}, instances, false);
	return as;
}

} /* namespace vulkan */

#endif

