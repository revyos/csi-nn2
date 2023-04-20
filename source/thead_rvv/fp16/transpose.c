/*
 * Copyright (C) 2016-2023 T-Head Semiconductor Co., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* SHL version 2.1.x */

#include "shl_thead_rvv.h"

int shl_rvv_transpose_fp16(struct csinn_tensor *input, struct csinn_tensor *output,
                           struct csinn_transpose_params *params)
{
    if (params->permute_num == 4 && params->permute[0] == 0 && params->permute[1] == 1 &&
        params->permute[2] == 2 && params->permute[3] == 3) {
        __fp16 *input_data = (__fp16 *)input->data;
        __fp16 *output_data = (__fp16 *)output->data;
        int sizeb = csinn_tensor_byte_size(input);
        memcpy(output_data, input_data, sizeb);
        return CSINN_TRUE;
    } else if (params->permute_num == 4 && params->permute[0] == 0 && params->permute[1] == 2 &&
               params->permute[2] == 3 && params->permute[3] == 1) {
        __fp16 *input_data = (__fp16 *)input->data;
        __fp16 *output_data = (__fp16 *)output->data;

        int batch = input->dim[0];
        int inner_size = input->dim[2] * input->dim[3];
        int outer_size = input->dim[1];
        for (int b = 0; b < batch; b++) {
            for (int i = 0; i < outer_size; i++) {
                int size = inner_size;
                __fp16 *out_ptr = output_data + i;
                while (size > 0) {
                    int vl = vsetvl_e16m1(size);
                    vfloat16m1_t _input = vle16_v_f16m1(input_data, vl);
                    input_data += vl;
                    vsse16_v_f16m1(out_ptr, outer_size * sizeof(__fp16), _input, vl);
                    out_ptr += vl * outer_size;
                    size -= vl;
                }
            }
            output_data += inner_size * outer_size;
        }
        return CSINN_TRUE;
    } else if (params->permute_num == 4 && params->permute[0] == 0 && params->permute[1] == 2 &&
               params->permute[2] == 1 && params->permute[3] == 3) {
        __fp16 *input_data = (__fp16 *)input->data;
        __fp16 *output_data = (__fp16 *)output->data;

        int batch = input->dim[0];
        int outer_size = input->dim[2];
        int inner_size = input->dim[1];
        for (int b = 0; b < batch; b++) {
            for (int i = 0; i < outer_size; i++) {
                for (int j = 0; j < inner_size; j++) {
                    __fp16 *in_ptr = input_data + (j * outer_size + i) * input->dim[3];
                    int size = input->dim[3];
                    while (size > 0) {
                        int vl = vsetvl_e16m1(size);
                        vfloat16m1_t _input = vle16_v_f16m1(in_ptr, vl);
                        in_ptr += vl;
                        vse16_v_f16m1(output_data, _input, vl);
                        output_data += vl;
                        size -= vl;
                    }
                }
            }
            input_data += input->dim[1] * input->dim[2] * input->dim[3];
        }

        return CSINN_TRUE;
    } else if (params->permute_num == 3 && params->permute[0] == 0 && params->permute[1] == 2 &&
               params->permute[2] == 1) {
        __fp16 *input_data = (__fp16 *)input->data;
        __fp16 *output_data = (__fp16 *)output->data;

        int batch = input->dim[0];
        int outer_size = input->dim[2];
        int inner_size = input->dim[1];
        for (int b = 0; b < batch; b++) {
            for (int i = 0; i < outer_size; i++) {
                int size = inner_size;
                __fp16 *in_ptr = input_data + i;
                while (size > 0) {
                    int vl = vsetvl_e16m1(size);
                    vfloat16m1_t _input = vlse16_v_f16m1(in_ptr, outer_size * sizeof(__fp16), vl);
                    in_ptr += vl * outer_size;
                    vse16_v_f16m1(output_data, _input, vl);
                    output_data += vl;
                    size -= vl;
                }
            }
            input_data += inner_size * outer_size;
        }

        return CSINN_TRUE;
    }
    return shl_ref_siso_callback_base(input, output, params, shl_ref_transpose);
}
