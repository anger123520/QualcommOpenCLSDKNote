//--------------------------------------------------------------------------------------
// File: compressed_tp10_vector_image_ops.cpp
// Desc:
//
// Author:      QUALCOMM
//
//               Copyright (c) 2017 QUALCOMM Technologies, Inc.
//                         All Rights Reserved.
//                      QUALCOMM Proprietary/GTDR
//--------------------------------------------------------------------------------------

// Std includes
#include <cstdlib>
#include <cstring>
#include <iostream>

// Project includes
#include "util/cl_wrapper.h"
#include "util/util.h"

// Library includes
#include <CL/cl.h>
#include <CL/cl_ext_qcom.h>
#include <array>

static const char *HELP_MESSAGE = "\n"
"Usage: compressed_tp10_vector_image_ops <source image data file>\n"
"\n"
"Demonstrates use of vector image ops using Qualcomm extensions to OpenCL.\n"
"These functions read and write several pixels at once.\n"
"This example has several kernels which simply copy an image, and verifies each\n"
"result at runtime, indicating on std err if there is an error.\n"
"There is no output image.\n";

// This examples use floating point valued functions, but half-float equivalents
// may be used in a completely analogous way.
static const char *PROGRAM_SOURCE[] = {
// The image format determines how the 4th argument to vector read operations is interpreted.
// For YUV images:
"static const int Y_PLANE = 0;\n",
"static const int U_PLANE = 1;\n",
"static const int V_PLANE = 2;\n",
"\n",
// Reads 1x1 from a YUV image and writes 3x1 to a Y-only image
"__kernel void read_yuv_1x1_write_y_3x1(__read_only  image2d_t src_image,\n",
"                                       __write_only image2d_t dest_image_y_plane,\n",
"                                                    sampler_t sampler)\n",
"{\n",
"    const int    wid_x       = get_global_id(0);\n",
"    const int    wid_y       = get_global_id(1);\n",
"    const int2   read_coord  = (int2)(3 * wid_x, wid_y);\n",
"    const int2   write_coord = (int2)(3 * wid_x, wid_y);\n",
"    const float4 pixels_in[] = {\n",
"        read_imagef(src_image, sampler, read_coord),\n",
"        read_imagef(src_image, sampler, read_coord + (int2)(1, 0)),\n",
"        read_imagef(src_image, sampler, read_coord + (int2)(2, 0)),\n",
"        };\n",
"    float        y_pixels_out[] = {pixels_in[0].s0, pixels_in[1].s0, pixels_in[2].s0};\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord, y_pixels_out);\n",
"}\n",
"\n",
// Reads 1x1 from a YUV image and writes 3x1 to a UV-only image
"__kernel void read_yuv_1x1_write_uv_3x1(__read_only  image2d_t src_image,\n",
"                                        __write_only image2d_t dest_image_uv_plane,\n",
"                                                     sampler_t sampler)\n",
"{\n",
"    const int    wid_x       = get_global_id(0);\n",
"    const int    wid_y       = get_global_id(1);\n",
"    const int2   read_coord  = (int2)(6 * wid_x, 2 * wid_y);\n",
"    const int2   write_coord = (int2)(3 * wid_x, wid_y);\n",
"    const float4 pixels_in[] = {\n",
"        read_imagef(src_image, sampler, read_coord),\n",
"        read_imagef(src_image, sampler, read_coord + (int2)(2, 0)),\n",
"        read_imagef(src_image, sampler, read_coord + (int2)(4, 0)),\n",
"        };\n",
"    float2       uv_pixels_out[] = {\n",
"        {pixels_in[0].s1, pixels_in[0].s2},\n",
"        {pixels_in[1].s1, pixels_in[1].s2},\n",
"        {pixels_in[2].s1, pixels_in[2].s2},\n",
"        };\n",
"    qcom_write_imagefv_3x1_n10t01(dest_image_uv_plane, write_coord, uv_pixels_out);\n",
"}\n",
"\n",
// Reads 2x2 from a YUV image and writes 3x1 to a Y-only image
"__kernel void read_yuv_2x2_write_y_3x1(__read_only  image2d_t src_image,\n",
"                                       __write_only image2d_t dest_image_y_plane,\n",
"                                                    sampler_t sampler)\n",
"{\n",
"    const int    wid_x       = get_global_id(0);\n",
"    const int    wid_y       = get_global_id(1);\n",
"    const float2 read_coord  = (float2)(6 * wid_x, 2 * wid_y) + 0.5f;\n",
"    const int2   write_coord = (int2)(6 * wid_x, 2 * wid_y);\n",
"    const float4 pixels_in[] = {\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord,                    Y_PLANE),\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord + (float2)(2., 0.), Y_PLANE),\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord + (float2)(4., 0.), Y_PLANE),\n",
"        };\n",
"    float        y_pixels_out[4][3] = {\n",
"        {pixels_in[0].s3, pixels_in[0].s2, pixels_in[1].s3}, {pixels_in[1].s2, pixels_in[2].s3, pixels_in[2].s2},\n",
"        {pixels_in[0].s0, pixels_in[0].s1, pixels_in[1].s0}, {pixels_in[1].s1, pixels_in[2].s0, pixels_in[2].s1},\n",
"        };\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord,                y_pixels_out[0]);\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord + (int2)(3, 0), y_pixels_out[1]);\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord + (int2)(0, 1), y_pixels_out[2]);\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord + (int2)(3, 1), y_pixels_out[3]);\n",
"}\n",
"\n",
// Reads 2x2 from a YUV image and writes 3x1 to a UV-only image
"__kernel void read_yuv_2x2_write_uv_3x1(__read_only  image2d_t src_image,\n",
"                                        __write_only image2d_t dest_image_uv_plane,\n",
"                                                     sampler_t sampler)\n",
"{\n",
"    const int    wid_x         = get_global_id(0);\n",
"    const int    wid_y         = get_global_id(1);\n",
"    const float2 read_coord    = 2 * ((float2)(6 * wid_x, 2 * wid_y) + 0.5f);\n",
"    const int2   write_coord   = (int2)(6 * wid_x, 2 * wid_y);\n",
"    const float4 u_pixels_in[] = {\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord,                    U_PLANE),\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord + (float2)(4., 0.), U_PLANE),\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord + (float2)(8., 0.), U_PLANE),\n",
"        };\n",
"    const float4 v_pixels_in[] = {\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord,                    V_PLANE),\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord + (float2)(4., 0.), V_PLANE),\n",
"        qcom_read_imagef_2x2(src_image, sampler, read_coord + (float2)(8., 0.), V_PLANE),\n",
"        };\n",
"    float2       uv_pixels_out[4][3] = {\n",
"        {{u_pixels_in[0].s3, v_pixels_in[0].s3}, {u_pixels_in[0].s2, v_pixels_in[0].s2}, {u_pixels_in[1].s3, v_pixels_in[1].s3}},\n",
"        {{u_pixels_in[1].s2, v_pixels_in[1].s2}, {u_pixels_in[2].s3, v_pixels_in[2].s3}, {u_pixels_in[2].s2, v_pixels_in[2].s2}},\n",
"        {{u_pixels_in[0].s0, v_pixels_in[0].s0}, {u_pixels_in[0].s1, v_pixels_in[0].s1}, {u_pixels_in[1].s0, v_pixels_in[1].s0}},\n",
"        {{u_pixels_in[1].s1, v_pixels_in[1].s1}, {u_pixels_in[2].s0, v_pixels_in[2].s0}, {u_pixels_in[2].s1, v_pixels_in[2].s1}},\n",
"        };\n",
"    qcom_write_imagefv_3x1_n10t01(dest_image_uv_plane, write_coord,                uv_pixels_out[0]);\n",
"    qcom_write_imagefv_3x1_n10t01(dest_image_uv_plane, write_coord + (int2)(3, 0), uv_pixels_out[1]);\n",
"    qcom_write_imagefv_3x1_n10t01(dest_image_uv_plane, write_coord + (int2)(0, 1), uv_pixels_out[2]);\n",
"    qcom_write_imagefv_3x1_n10t01(dest_image_uv_plane, write_coord + (int2)(3, 1), uv_pixels_out[3]);\n",
"}\n",
"\n",
// Reads 4x1 from a YUV image and writes 3x1 to a Y-only image
"__kernel void read_yuv_4x1_write_y_3x1(__read_only  image2d_t src_image,\n",
"                                       __write_only image2d_t dest_image_y_plane,\n",
"                                                    sampler_t sampler)\n",
"{\n",
"    const int    wid_x           = get_global_id(0);\n",
"    const int    wid_y           = get_global_id(1);\n",
"    const float2 read_coord      = (float2)(12 * wid_x, wid_y) + 0.5f;\n",
"    const int2   write_coord     = (int2)(12 * wid_x, wid_y);\n",
"    const float4 y_pixels_in[]   = {\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord,                    Y_PLANE),\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord + (float2)(4., 0.), Y_PLANE),\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord + (float2)(8., 0.), Y_PLANE),\n",
"        };\n",
"    float        y_pixels_out[4][3] = {\n",
"        {y_pixels_in[0].s0, y_pixels_in[0].s1, y_pixels_in[0].s2},\n",
"        {y_pixels_in[0].s3, y_pixels_in[1].s0, y_pixels_in[1].s1},\n",
"        {y_pixels_in[1].s2, y_pixels_in[1].s3, y_pixels_in[2].s0},\n",
"        {y_pixels_in[2].s1, y_pixels_in[2].s2, y_pixels_in[2].s3},\n",
        "};\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord, y_pixels_out[0]);\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord + (int2)(3, 0), y_pixels_out[1]);\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord + (int2)(6, 0), y_pixels_out[2]);\n",
"    qcom_write_imagefv_3x1_n10t00(dest_image_y_plane, write_coord + (int2)(9, 0), y_pixels_out[3]);\n",
"}\n",
"\n",
// Reads 4x1 from a YUV image and writes 3x1 to a UV-only image
"__kernel void read_yuv_4x1_write_uv_3x1(__read_only  image2d_t src_image,\n",
"                                        __write_only image2d_t dest_image_uv_plane,\n",
"                                                     sampler_t sampler)\n",
"{\n",
"    const int    wid_x         = get_global_id(0);\n",
"    const int    wid_y         = get_global_id(1);\n",
"    const float2 read_coord    = (float2)(12 * wid_x, 2 * wid_y) + 0.5f;\n",
"    const int2   write_coord   = (int2)(6 * wid_x, wid_y);\n",
"    const float4 u_pixels_in[] = {\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord,                    U_PLANE),\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord + (float2)(4., 0.), U_PLANE),\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord + (float2)(8., 0.), U_PLANE),\n",
"        };\n",
"    const float4 v_pixels_in[] = {\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord,                    V_PLANE),\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord + (float2)(4., 0.), V_PLANE),\n",
"        qcom_read_imagef_4x1(src_image, sampler, read_coord + (float2)(8., 0.), V_PLANE),\n",
"        };\n",
"    float2       uv_pixels_out[2][3] = {\n",
"        {{u_pixels_in[0].s0, v_pixels_in[0].s0}, {u_pixels_in[0].s2, v_pixels_in[0].s2}, {u_pixels_in[1].s0, v_pixels_in[1].s0}},\n",
"        {{u_pixels_in[1].s2, v_pixels_in[1].s2}, {u_pixels_in[2].s0, v_pixels_in[2].s0}, {u_pixels_in[2].s2, v_pixels_in[2].s2}},\n",
"        };\n",
"    qcom_write_imagefv_3x1_n10t01(dest_image_uv_plane, write_coord,                uv_pixels_out[0]);\n",
"    qcom_write_imagefv_3x1_n10t01(dest_image_uv_plane, write_coord + (int2)(3, 0), uv_pixels_out[1]);\n",
"}\n"
};

static const cl_uint PROGRAM_SOURCE_LEN = sizeof(PROGRAM_SOURCE) / sizeof(const char *);

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Please specify source and output images.\n";
        std::cerr << HELP_MESSAGE;
        std::exit(EXIT_SUCCESS);
    }
    const std::string src_image_filename(argv[1]);

    cl_wrapper wrapper;
    cl_program   program             = wrapper.make_program(PROGRAM_SOURCE, PROGRAM_SOURCE_LEN);
    cl_kernel    copy_kernels[]      = {
            wrapper.make_kernel("read_yuv_1x1_write_y_3x1",  program),
            wrapper.make_kernel("read_yuv_1x1_write_uv_3x1", program),
            wrapper.make_kernel("read_yuv_2x2_write_y_3x1",  program),
            wrapper.make_kernel("read_yuv_4x1_write_uv_3x1", program),
            wrapper.make_kernel("read_yuv_4x1_write_y_3x1",  program),
            wrapper.make_kernel("read_yuv_2x2_write_uv_3x1", program),
    };
    static const size_t copy_kernels_size = sizeof(copy_kernels) / sizeof(copy_kernels[0]);
    cl_kernel    conversion_kernel_y      = wrapper.make_kernel("read_yuv_1x1_write_y_3x1", program);
    cl_kernel    conversion_kernel_uv     = wrapper.make_kernel("read_yuv_1x1_write_uv_3x1", program);
    cl_context   context                  = wrapper.get_context();
    tp10_image_t src_tp10_image_info      = load_tp10_image_data(src_image_filename);

    /*
     * Step 0: Confirm the required OpenCL extensions are supported.
     */

    if (!wrapper.check_extension_support("cl_qcom_other_image"))
    {
        std::cerr << "Extension cl_qcom_other_image needed for TP10 image format is not supported.\n";
        std::exit(EXIT_FAILURE);
    }

    if (!wrapper.check_extension_support("cl_qcom_vector_image_ops"))
    {
        std::cerr << "Extension cl_qcom_vector_image_ops needed for vector image reads/writes is not supported.\n";
        std::exit(EXIT_FAILURE);
    }

    if (!wrapper.check_extension_support("cl_qcom_ext_host_ptr"))
    {
        std::cerr << "Extension cl_qcom_ext_host_ptr needed for ION-backed images is not supported.\n";
        std::exit(EXIT_FAILURE);
    }

    if (!wrapper.check_extension_support("cl_qcom_ion_host_ptr"))
    {
        std::cerr << "Extension cl_qcom_ion_host_ptr needed for ION-backed images is not supported.\n";
        std::exit(EXIT_FAILURE);
    }

    /*
     * Step 1: Create suitable ion buffer-backed CL images. Note that planar formats (like TP10) must be read only,
     * but you can write to child images derived from the planes. (See step 2 for deriving child images.)
     */

    cl_image_format src_tp10_format;
    src_tp10_format.image_channel_order     = CL_QCOM_TP10;
    src_tp10_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc src_tp10_desc;
    std::memset(&src_tp10_desc, 0, sizeof(src_tp10_desc));
    src_tp10_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    src_tp10_desc.image_width  = src_tp10_image_info.y_width;
    src_tp10_desc.image_height = src_tp10_image_info.y_height;

    cl_int err = 0;
    cl_mem_ion_host_ptr src_tp10_ion_mem = wrapper.make_ion_buffer_for_yuv_image(src_tp10_format, src_tp10_desc);
    cl_mem src_tp10_image = clCreateImage(
            context,
            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | CL_MEM_EXT_HOST_PTR_QCOM,
            &src_tp10_format,
            &src_tp10_desc,
            &src_tp10_ion_mem,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for source image." << "\n";
        std::exit(err);
    }

    cl_image_format out_tp10_format;
    out_tp10_format.image_channel_order     = CL_QCOM_TP10;
    out_tp10_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc out_tp10_desc;
    std::memset(&out_tp10_desc, 0, sizeof(out_tp10_desc));
    out_tp10_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    out_tp10_desc.image_width  = src_tp10_image_info.y_width;
    out_tp10_desc.image_height = src_tp10_image_info.y_height;

    cl_mem_ion_host_ptr out_tp10_ion_mem = wrapper.make_ion_buffer_for_yuv_image(out_tp10_format, out_tp10_desc);
    cl_mem out_tp10_image = clCreateImage(
            context,
            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | CL_MEM_EXT_HOST_PTR_QCOM,
            &out_tp10_format,
            &out_tp10_desc,
            &out_tp10_ion_mem,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for output image." << "\n";
        std::exit(err);
    }

    cl_image_format compressed_tp10_format;
    compressed_tp10_format.image_channel_order     = CL_QCOM_COMPRESSED_TP10;
    compressed_tp10_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc compressed_tp10_desc;
    std::memset(&compressed_tp10_desc, 0, sizeof(compressed_tp10_desc));
    compressed_tp10_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    compressed_tp10_desc.image_width  = src_tp10_image_info.y_width;
    compressed_tp10_desc.image_height = src_tp10_image_info.y_height;

    cl_mem_ion_host_ptr compressed_tp10_ion_mem = wrapper.make_ion_buffer_for_compressed_image(compressed_tp10_format,
                                                                                               compressed_tp10_desc);
    cl_mem compressed_tp10_image = clCreateImage(
            context,
            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR | CL_MEM_EXT_HOST_PTR_QCOM,
            &compressed_tp10_format,
            &compressed_tp10_desc,
            &compressed_tp10_ion_mem,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for compressed image." << "\n";
        std::exit(err);
    }
    
    /*
     * Step 2: Separate planar TP10 images into their component planes.
     */

    cl_image_format src_y_plane_format;
    src_y_plane_format.image_channel_order     = CL_QCOM_TP10_Y;
    src_y_plane_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc src_y_plane_desc;
    std::memset(&src_y_plane_desc, 0, sizeof(src_y_plane_desc));
    src_y_plane_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    src_y_plane_desc.image_width  = src_tp10_image_info.y_width;
    src_y_plane_desc.image_height = src_tp10_image_info.y_height;
    src_y_plane_desc.mem_object   = src_tp10_image;

    cl_mem src_y_plane = clCreateImage(
            context,
            CL_MEM_READ_ONLY,
            &src_y_plane_format,
            &src_y_plane_desc,
            NULL,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for source image y plane." << "\n";
        std::exit(err);
    }

    cl_image_format src_uv_plane_format;
    src_uv_plane_format.image_channel_order     = CL_QCOM_TP10_UV;
    src_uv_plane_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc src_uv_plane_desc;
    std::memset(&src_uv_plane_desc, 0, sizeof(src_uv_plane_desc));
    src_uv_plane_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    // The image dimensions for the uv-plane derived image must be the same as the parent image, even though the
    // actual dimensions of the uv-plane differ by a factor of 2 in each dimension.
    src_uv_plane_desc.image_width  = src_tp10_image_info.y_width;
    src_uv_plane_desc.image_height = src_tp10_image_info.y_height;
    src_uv_plane_desc.mem_object   = src_tp10_image;

    cl_mem src_uv_plane = clCreateImage(
            context,
            CL_MEM_READ_ONLY,
            &src_uv_plane_format,
            &src_uv_plane_desc,
            NULL,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for source image uv plane." << "\n";
        std::exit(err);
    }

    cl_image_format out_y_plane_format;
    out_y_plane_format.image_channel_order     = CL_QCOM_TP10_Y;
    out_y_plane_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc out_y_plane_desc;
    std::memset(&out_y_plane_desc, 0, sizeof(out_y_plane_desc));
    out_y_plane_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    out_y_plane_desc.image_width  = out_tp10_desc.image_width;
    out_y_plane_desc.image_height = out_tp10_desc.image_height;
    out_y_plane_desc.mem_object   = out_tp10_image;

    cl_mem out_y_plane = clCreateImage(
            context,
            CL_MEM_WRITE_ONLY,
            &out_y_plane_format,
            &out_y_plane_desc,
            NULL,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for destination image y plane." << "\n";
        std::exit(err);
    }

    cl_image_format out_uv_plane_format;
    out_uv_plane_format.image_channel_order     = CL_QCOM_TP10_UV;
    out_uv_plane_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc out_uv_plane_desc;
    std::memset(&out_uv_plane_desc, 0, sizeof(out_uv_plane_desc));
    out_uv_plane_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    // The image dimensions for the uv-plane derived image must be the same as the parent image, even though the
    // actual dimensions of the uv-plane differ by a factor of 2 in each dimension.
    out_uv_plane_desc.image_width  = out_tp10_desc.image_width;
    out_uv_plane_desc.image_height = out_tp10_desc.image_height;
    out_uv_plane_desc.mem_object   = out_tp10_image;

    cl_mem out_uv_plane = clCreateImage(
            context,
            CL_MEM_WRITE_ONLY,
            &out_uv_plane_format,
            &out_uv_plane_desc,
            NULL,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for destination image uv plane." << "\n";
        std::exit(err);
    }

    cl_image_format compressed_y_plane_format;
    compressed_y_plane_format.image_channel_order     = CL_QCOM_COMPRESSED_TP10_Y;
    compressed_y_plane_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc compressed_y_plane_desc;
    std::memset(&compressed_y_plane_desc, 0, sizeof(compressed_y_plane_desc));
    compressed_y_plane_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    compressed_y_plane_desc.image_width  = compressed_tp10_desc.image_width;
    compressed_y_plane_desc.image_height = compressed_tp10_desc.image_height;
    compressed_y_plane_desc.mem_object   = compressed_tp10_image;

    cl_mem compressed_y_plane = clCreateImage(
            context,
            CL_MEM_READ_WRITE,
            &compressed_y_plane_format,
            &compressed_y_plane_desc,
            NULL,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for compressed image y plane." << "\n";
        std::exit(err);
    }

    cl_image_format compressed_uv_plane_format;
    compressed_uv_plane_format.image_channel_order     = CL_QCOM_COMPRESSED_TP10_UV;
    compressed_uv_plane_format.image_channel_data_type = CL_QCOM_UNORM_INT10;

    cl_image_desc compressed_uv_plane_desc;
    std::memset(&compressed_uv_plane_desc, 0, sizeof(compressed_uv_plane_desc));
    compressed_uv_plane_desc.image_type   = CL_MEM_OBJECT_IMAGE2D;
    // The image dimensions for the uv-plane derived image must be the same as the parent image, even though the
    // actual dimensions of the uv-plane differ by a factor of 2 in each dimension.
    compressed_uv_plane_desc.image_width  = compressed_tp10_desc.image_width;
    compressed_uv_plane_desc.image_height = compressed_tp10_desc.image_height;
    compressed_uv_plane_desc.mem_object   = compressed_tp10_image;

    cl_mem compressed_uv_plane = clCreateImage(
            context,
            CL_MEM_READ_WRITE,
            &compressed_uv_plane_format,
            &compressed_uv_plane_desc,
            NULL,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateImage for compressed image uv plane." << "\n";
        std::exit(err);
    }
    
    /*
     * Step 3: Copy data to input image planes. Note that for linear TP10 images you must observe row alignment
     * restrictions. (You may also write to the ion buffer directly if you prefer, however using clEnqueueMapImage for
     * a child planar image will return the correct host pointer for the desired plane.)
     */

    cl_command_queue command_queue  = wrapper.get_command_queue();
    const size_t     origin[]       = {0, 0, 0};
    const size_t     src_y_region[] = {src_y_plane_desc.image_width, src_y_plane_desc.image_height, 1};
    size_t           row_pitch      = 0;
    unsigned char   *image_ptr      = static_cast<unsigned char *>(clEnqueueMapImage(
            command_queue,
            src_y_plane,
            CL_TRUE,
            CL_MAP_WRITE,
            origin,
            src_y_region,
            &row_pitch,
            NULL,
            0,
            NULL,
            NULL,
            &err
    ));
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " mapping source image y-plane buffer for writing." << "\n";
        std::exit(err);
    }

    // Copies image data to the ION buffer from the host
    for (uint32_t i = 0; i < src_y_plane_desc.image_height; ++i)
    {
        std::memcpy(
                image_ptr                          + i * row_pitch,
                src_tp10_image_info.y_plane.data() + i * src_y_plane_desc.image_width * 4 / 3,
                src_y_plane_desc.image_width * 4 / 3
        );
    }

    err = clEnqueueUnmapMemObject(command_queue, src_y_plane, image_ptr, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " unmapping source image y-plane data buffer." << "\n";
        std::exit(err);
    }

    // Note the discrepancy between the child plane image descriptor and the size required by clEnqueueMapImage.
    const size_t src_uv_region[] = {src_uv_plane_desc.image_width / 2, src_uv_plane_desc.image_height / 2, 1};
    row_pitch                    = 0;
    image_ptr = static_cast<unsigned char *>(clEnqueueMapImage(
            command_queue,
            src_uv_plane,
            CL_TRUE,
            CL_MAP_WRITE,
            origin,
            src_uv_region,
            &row_pitch,
            NULL,
            0,
            NULL,
            NULL,
            &err
    ));
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " mapping source image uv-plane buffer for writing." << "\n";
        std::exit(err);
    }

    // Copies image data to the ION buffer from the host
    for (uint32_t i = 0; i < src_uv_plane_desc.image_height / 2; ++i)
    {
        std::memcpy(
                image_ptr                           + i * row_pitch,
                src_tp10_image_info.uv_plane.data() + i * src_uv_plane_desc.image_width * 4 / 3,
                src_uv_plane_desc.image_width * 4 / 3
        );
    }

    err = clEnqueueUnmapMemObject(command_queue, src_uv_plane, image_ptr, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " unmapping source image uv-plane data buffer." << "\n";
        std::exit(err);
    }

    /*
     * Step 4: Set up other kernel arguments
     */

    cl_sampler sampler = clCreateSampler(
            context,
            CL_FALSE,
            CL_ADDRESS_CLAMP_TO_EDGE,
            CL_FILTER_NEAREST,
            &err
    );
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error " << err << " with clCreateSampler." << "\n";
        std::exit(err);
    }

    /*
     * Step 5: Run the kernel separately for y- and uv-planes
     */

    enum class comp_t
    {
        Y_ONLY,
        UV_ONLY,
    };

    // Impromptu data structure to cut down on code duplication to enqueue kernels.
    struct kernel_exec_params_t
    {
        cl_kernel copy_kernel;
        cl_mem    src_plane;
        cl_mem    dst_plane;
        size_t    width;
        size_t    height;
        comp_t    comp_type;
    };

    std::array<kernel_exec_params_t, copy_kernels_size> kernel_execution_params{
            /*                   kernel,          source,         destination,         work_size[0],                              work_size[1] */
            kernel_exec_params_t{copy_kernels[0], src_tp10_image, compressed_y_plane,  src_tp10_desc.image_width / 3,             src_tp10_desc.image_height,                comp_t::Y_ONLY},
            kernel_exec_params_t{copy_kernels[1], src_tp10_image, compressed_uv_plane, src_tp10_desc.image_width / 6,             src_tp10_desc.image_height / 2,            comp_t::UV_ONLY},
            kernel_exec_params_t{copy_kernels[2], src_tp10_image, compressed_y_plane,  src_tp10_desc.image_width / 6,             src_tp10_desc.image_height / 2,            comp_t::Y_ONLY},
            kernel_exec_params_t{copy_kernels[3], src_tp10_image, compressed_uv_plane, work_units(src_tp10_desc.image_width, 12), src_tp10_desc.image_height / 2,            comp_t::UV_ONLY},
            kernel_exec_params_t{copy_kernels[4], src_tp10_image, compressed_y_plane,  work_units(src_tp10_desc.image_width, 12), src_tp10_desc.image_height,                comp_t::Y_ONLY},
            kernel_exec_params_t{copy_kernels[5], src_tp10_image, compressed_uv_plane, work_units(src_tp10_desc.image_width, 12), work_units(src_tp10_desc.image_height, 4), comp_t::UV_ONLY},
    };

    for (size_t i = 0; i < kernel_execution_params.size(); ++i)
    {
        cl_kernel copy_kernel      = kernel_execution_params[i].copy_kernel;
        cl_mem    src_plane        = kernel_execution_params[i].src_plane;
        cl_mem    compressed_plane = kernel_execution_params[i].dst_plane;

        err = clSetKernelArg(copy_kernel, 0, sizeof(src_plane), &src_plane);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clSetKernelArg for argument 0 for copy kernel." << "\n";
            std::exit(err);
        }

        err = clSetKernelArg(copy_kernel, 1, sizeof(compressed_plane), &compressed_plane);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clSetKernelArg for argument 1 for copy kernel." << "\n";
            std::exit(err);
        }

        err = clSetKernelArg(copy_kernel, 2, sizeof(sampler), &sampler);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clSetKernelArg for argument 2 for copy kernel." << "\n";
            std::exit(err);
        }

        const size_t work_size[] = {kernel_execution_params[i].width, kernel_execution_params[i].height};
        err = clEnqueueNDRangeKernel(
                command_queue,
                copy_kernel,
                2,
                NULL,
                work_size,
                NULL,
                0,
                NULL,
                NULL
        );
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clEnqueueNDRangeKernel for copy kernel." << "\n";
            std::exit(err);
        }

        size_t comparison_map_region[] = {0, 0, 1};
        size_t comparison_global_work_size[] = {0, 0};
        cl_mem comparison_plane, dst_plane;
        size_t comparison_width  = 0;
        size_t comparison_height = 0;
        cl_kernel conversion_kernel;

        switch(kernel_execution_params[i].comp_type)
        {
            case comp_t::Y_ONLY:
            {
                comparison_map_region[0]       = src_tp10_desc.image_width;
                comparison_map_region[1]       = src_tp10_desc.image_height;
                comparison_width               = src_tp10_desc.image_width * 4 / 3;
                comparison_height              = src_tp10_desc.image_height;
                comparison_global_work_size[0] = src_tp10_desc.image_width / 3;
                comparison_global_work_size[1] = src_tp10_desc.image_height;
                comparison_plane               = src_y_plane;
                dst_plane                      = out_y_plane;
                conversion_kernel = conversion_kernel_y;
                break;
            }
            case comp_t::UV_ONLY:
            {
                comparison_map_region[0]       = src_tp10_desc.image_width / 2;
                comparison_map_region[1]       = src_tp10_desc.image_height / 2;
                comparison_width               = src_tp10_desc.image_width * 4 / 3;
                comparison_height              = src_tp10_desc.image_height / 2;
                comparison_global_work_size[0] = work_units(src_tp10_desc.image_width, 6);
                comparison_global_work_size[1] = src_tp10_desc.image_height / 2;
                comparison_plane               = src_uv_plane;
                dst_plane                      = out_uv_plane;
                conversion_kernel = conversion_kernel_uv;
                break;
            }
            default:
            {
                std::cerr << "On iteration " << i << ":\n";
                std::cerr << "\tUnknown comparison type\n";
                std::exit(EXIT_FAILURE);
            }
        }

        err = clSetKernelArg(conversion_kernel, 0, sizeof(compressed_tp10_image), &compressed_tp10_image);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clSetKernelArg for argument 0 for conversion kernel." << "\n";
            std::exit(err);
        }

        err = clSetKernelArg(conversion_kernel, 1, sizeof(dst_plane), &dst_plane);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clSetKernelArg for argument 1 for conversion kernel." << "\n";
            std::exit(err);
        }

        err = clSetKernelArg(conversion_kernel, 2, sizeof(sampler), &sampler);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clSetKernelArg for argument 2 for conversion kernel." << "\n";
            std::exit(err);
        }

        err = clEnqueueNDRangeKernel(
                command_queue,
                conversion_kernel,
                2,
                NULL,
                comparison_global_work_size,
                NULL,
                0,
                NULL,
                NULL
        );
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " with clEnqueueNDRangeKernel for conversion kernel." << "\n";
            std::exit(err);
        }
        
        size_t         src_row_pitch = 0;
        unsigned char *src_image_ptr = static_cast<unsigned char *>(clEnqueueMapImage(
                command_queue,
                comparison_plane,
                CL_TRUE,
                CL_MAP_READ,
                origin,
                comparison_map_region,
                &src_row_pitch,
                NULL,
                0,
                NULL,
                NULL,
                &err
        ));
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " mapping src image plane for validation." << "\n";
            std::exit(err);
        }

        size_t         dst_row_pitch = 0;
        unsigned char *dst_image_ptr = static_cast<unsigned char *>(clEnqueueMapImage(
                command_queue,
                dst_plane,
                CL_TRUE,
                CL_MAP_READ | CL_MAP_WRITE,
                origin,
                comparison_map_region,
                &dst_row_pitch,
                NULL,
                0,
                NULL,
                NULL,
                &err
        ));
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " mapping dst image plane for validation." << "\n";
            std::exit(err);
        }

        if (src_row_pitch != dst_row_pitch)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tRow pitches do not match, so unable to compare images.\n";
            std::exit(EXIT_FAILURE);
        }

        // Compare images and overwrite the destination buffer for the next iteration
        for (size_t j = 0; j < comparison_height; ++j)
        {
            const int res = std::memcmp(src_image_ptr + j * src_row_pitch,
                                        dst_image_ptr + j * dst_row_pitch,
                                        comparison_width);
            if(res != 0)
            {
                std::cerr << "On iteration " << i << ":\n";
                std::cerr << "\tImages were not equal!\n";
                std::exit(EXIT_FAILURE);
            }

            std::memset(dst_image_ptr + j * dst_row_pitch, 0, comparison_width);
        }

        err = clEnqueueUnmapMemObject(command_queue, comparison_plane, src_image_ptr, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " unmapping src image plane." << "\n";
            std::exit(err);
        }

        err = clEnqueueUnmapMemObject(command_queue, dst_plane, dst_image_ptr, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            std::cerr << "On iteration " << i << ":\n";
            std::cerr << "\tError " << err << " unmapping dst image plane." << "\n";
            std::exit(err);
        }
    }

    clFinish(command_queue); // Finish up the unmaps above.

    // Clean up cl resources that aren't automatically handled by cl_wrapper
    clReleaseSampler(sampler);
    clReleaseMemObject(src_uv_plane);
    clReleaseMemObject(src_y_plane);
    clReleaseMemObject(src_tp10_image);
    clReleaseMemObject(out_uv_plane);
    clReleaseMemObject(out_y_plane);
    clReleaseMemObject(out_tp10_image);
    clReleaseMemObject(compressed_uv_plane);
    clReleaseMemObject(compressed_y_plane);
    clReleaseMemObject(compressed_tp10_image);

    return 0;
}
