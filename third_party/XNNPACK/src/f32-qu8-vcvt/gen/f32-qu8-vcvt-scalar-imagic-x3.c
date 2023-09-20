// Auto-generated file. Do not edit!
//   Template: src/f32-qs8-vcvt/scalar-imagic.c.in
//   Generator: tools/xngen
//
// Copyright 2021 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>

#include <xnnpack/common.h>
#include <xnnpack/math.h>
#include <xnnpack/vcvt.h>


void xnn_f32_qu8_vcvt_ukernel__scalar_imagic_x3(
    size_t batch,
    const float* input,
    uint8_t* output,
    const union xnn_f32_qu8_cvt_params params[restrict XNN_MIN_ELEMENTS(1)])
{
  assert(batch != 0);
  assert(batch % sizeof(float) == 0);
  assert(input != NULL);
  assert(output != NULL);

  const float vscale = params->scalar_imagic.scale;
  const float vmagic_bias = params->scalar_imagic.magic_bias;
  const int32_t vmagic_min = params->scalar_imagic.magic_min;
  const int32_t vmagic_max = params->scalar_imagic.magic_max;
  const int32_t vmagic_bias_less_zero_point = params->scalar_imagic.magic_bias_less_zero_point;

  for (; batch >= 3 * sizeof(float); batch -= 3 * sizeof(float)) {
    float vx0 = input[0];
    float vx1 = input[1];
    float vx2 = input[2];
    input += 3;

    vx0 *= vscale;
    vx1 *= vscale;
    vx2 *= vscale;

    vx0 += vmagic_bias;
    vx1 += vmagic_bias;
    vx2 += vmagic_bias;

    int32_t vy0 = (int32_t) float_as_uint32(vx0);
    int32_t vy1 = (int32_t) float_as_uint32(vx1);
    int32_t vy2 = (int32_t) float_as_uint32(vx2);

    vy0 = math_max_s32(vy0, vmagic_min);
    vy1 = math_max_s32(vy1, vmagic_min);
    vy2 = math_max_s32(vy2, vmagic_min);

    vy0 = math_min_s32(vy0, vmagic_max);
    vy1 = math_min_s32(vy1, vmagic_max);
    vy2 = math_min_s32(vy2, vmagic_max);

    vy0 -= vmagic_bias_less_zero_point;
    vy1 -= vmagic_bias_less_zero_point;
    vy2 -= vmagic_bias_less_zero_point;

    output[0] = (uint8_t) vy0;
    output[1] = (uint8_t) vy1;
    output[2] = (uint8_t) vy2;
    output += 3;
  }
  if XNN_UNLIKELY(batch != 0) {
    do {
      float vx = *input++;
      vx *= vscale;
      vx += vmagic_bias;

      int32_t vy = (int32_t) float_as_uint32(vx);
      vy = math_max_s32(vy, vmagic_min);
      vy = math_min_s32(vy, vmagic_max);
      vy -= vmagic_bias_less_zero_point;

      *output++ = (uint8_t) vy;

      batch -= sizeof(float);
    } while (batch != 0);
  }
}
