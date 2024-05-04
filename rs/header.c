/**
 *  @file header.c
 *  @brief drumlogue SDK unit header
 *
 *  Copyright (c) 2020-2022 KORG Inc. All rights reserved.
 *
 */

#include "unit_osc.h"   // Note: Include base definitions for osc units

// ---- Unit header definition  --------------------------------------------------------------------

const __unit_header unit_header_t unit_header = {
    .header_size = sizeof(unit_header_t),                  // Size of this header. Leave as is.
    .target = UNIT_TARGET_PLATFORM | k_unit_module_osc,    // Tagret platform and module pair for this unit
    .api = UNIT_API_VERSION,                               // API version for which unit was built. See runtime.h
    .dev_id = 0x42636877U,  // "Bchw"
    .unit_id = 0x09010000,  // Product number(01),Unit type(01=OSC),reserved
    .version = 0x00010000U,
    .name = "LiRS",
    .num_params = 9,
    .params = {
        // Format: min, max, center, default, type, fractional, frac. type, <reserved>, name
        // See common/runtime.h for type enum and unit_param_t structure

        {-256, 255, 0, 0, k_unit_param_type_none, 0, 0, 0, {"Tmbr"}},
        {-256, 255, 0, 0, k_unit_param_type_none, 0, 0, 0, {"Colr"}},
        // 8 Edit menu parameters
        {28, 36, 28, 28, k_unit_param_type_strings, 0, 0, 0, {"Shap"}},
        {0, 2, 0, 0, k_unit_param_type_strings, 0, 0, 0, {"mTgt"}},
        {0, 31, 0, 0, k_unit_param_type_none, 0, 0, 0, {"mDly"}},
        {-127, 127, 0, 0, k_unit_param_type_none, 0, 0, 0, {"FMLv"}},
        {-127, 127, 0, 0, k_unit_param_type_none, 0, 0, 0, {"Ptch"}},
        {0, 6, 6, 6, k_unit_param_type_strings, 0, 0, 0, {"Bits"}},
        {0, 5, 5, 5, k_unit_param_type_strings, 0, 0, 0, {"Rate"}},

        {0, 0, 0, 0, k_unit_param_type_none, 0, 0, 0, {""}}},
};
