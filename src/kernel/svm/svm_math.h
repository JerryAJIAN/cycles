/*
 * Copyright 2011-2013 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

CCL_NAMESPACE_BEGIN

ccl_device void svm_node_math(KernelGlobals *kg,
                              ShaderData *sd,
                              float *stack,
                              uint type,
                              uint inputs_stack_offsets,
                              uint result_stack_offset,
                              int *offset)
{
  uint a_stack_offset, b_stack_offset;
  svm_unpack_node_uchar2(inputs_stack_offsets, &a_stack_offset, &b_stack_offset);

  float a = stack_load_float(stack, a_stack_offset);
  float b = stack_load_float(stack, b_stack_offset);
  float result = svm_math((NodeMathType)type, a, b);

  stack_store_float(stack, result_stack_offset, result);
}

ccl_device void svm_node_vector_math(KernelGlobals *kg,
                                     ShaderData *sd,
                                     float *stack,
                                     uint type,
                                     uint inputs_stack_offsets,
                                     uint outputs_stack_offsets,
                                     int *offset)
{
  uint value_stack_offset, vector_stack_offset;
  uint a_stack_offset, b_stack_offset, scale_stack_offset;
  svm_unpack_node_uchar3(
      inputs_stack_offsets, &a_stack_offset, &b_stack_offset, &scale_stack_offset);
  svm_unpack_node_uchar2(outputs_stack_offsets, &value_stack_offset, &vector_stack_offset);

  float3 a = stack_load_float3(stack, a_stack_offset);
  float3 b = stack_load_float3(stack, b_stack_offset);
  float scale = stack_load_float(stack, scale_stack_offset);

  float value;
  float3 vector;
  svm_vector_math(&value, &vector, (NodeVectorMathType)type, a, b, scale);

  if (stack_valid(value_stack_offset))
    stack_store_float(stack, value_stack_offset, value);
  if (stack_valid(vector_stack_offset))
    stack_store_float3(stack, vector_stack_offset, vector);
}

ccl_device void svm_node_matrix_math(KernelGlobals *kg, ShaderData *sd, float *stack, uint itype, uint vec_offset, uint out_offset, int *offset)
{
  NodeMatrixMath type = (NodeMatrixMath)itype;
  float3 v = stack_load_float3(stack, vec_offset);

  Transform tfm;
  tfm.x = read_node_float(kg, offset);
  tfm.y = read_node_float(kg, offset);
  tfm.z = read_node_float(kg, offset);

  float3 r;
  switch (type) {
    case NODE_MATRIX_MATH_DIRECTION:
    {
      r = transform_direction(&tfm, v);
      break;
    }
    case NODE_MATRIX_MATH_PERSPECTIVE:
    {
#ifndef __KERNEL_GPU__
      ProjectionTransform pt(tfm);
      r = transform_perspective(&pt, v);
#endif
      break;
    }
    case NODE_MATRIX_MATH_DIR_TRANSPOSED:
    {
      r = transform_direction_transposed(&tfm, v);
      break;
    }
    default:
      r = transform_point(&tfm, v);
  }
  stack_store_float3(stack, out_offset, r);
}

CCL_NAMESPACE_END
