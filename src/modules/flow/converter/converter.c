/*
 * This file is part of the Soletta Project
 *
 * Copyright (C) 2015 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "converter-gen.h"

#include "sol-flow-internal.h"
#include "sol-mainloop.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

struct sol_converter_byte {
    unsigned char min;
    unsigned char max;
};

struct sol_converter_boolean {
    bool output_value;
};

struct sol_converter_bits {
    unsigned char last;
    unsigned char output_initialized;
    unsigned char connected_ports;
};

struct sol_converter_string {
    char *string;
};

struct sol_converter_rgb {
    struct sol_rgb output_value;
    bool output_initialized[3];
};

struct sol_converter_irange_compose {
    unsigned int output_value;
    unsigned char connected_ports : 4;
    unsigned char port_has_value : 4;
};

struct sol_converter_boolean_string {
    char *false_value;
    char *true_value;
};

static int
irange_min_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    int32_t value;
    int r;

    r = sol_flow_packet_get_irange_value(packet, &value);
    SOL_INT_CHECK(r, < 0, r);
    mdata->min = value;

    return 0;
}

static int
irange_max_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    int32_t value;
    int r;

    r = sol_flow_packet_get_irange_value(packet, &value);
    SOL_INT_CHECK(r, < 0, r);
    mdata->max = value;

    return 0;
}

static int
irange_true_range_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    int r = sol_flow_packet_get_irange(packet, mdata);

    SOL_INT_CHECK(r, < 0, r);
    return 0;
}

static int
drange_min_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange *mdata = data;
    double value;
    int r;

    r = sol_flow_packet_get_drange_value(packet, &value);
    SOL_INT_CHECK(r, < 0, r);
    mdata->min = value;

    return 0;
}

static int
drange_max_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    double value;
    int r;

    r = sol_flow_packet_get_drange_value(packet, &value);
    SOL_INT_CHECK(r, < 0, r);
    mdata->max = value;

    return 0;
}

static int
drange_true_range_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange *mdata = data;
    int r = sol_flow_packet_get_drange(packet, mdata);

    SOL_INT_CHECK(r, < 0, r);
    return 0;
}

static int
byte_min_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;
    int r;
    unsigned char value;

    r = sol_flow_packet_get_byte(packet, &value);
    SOL_INT_CHECK(r, < 0, r);
    mdata->min = value;

    return 0;
}

static int
byte_max_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;
    int r;
    unsigned char value;

    r = sol_flow_packet_get_byte(packet, &value);
    SOL_INT_CHECK(r, < 0, r);
    mdata->max = value;

    return 0;
}

static int
boolean_to_irange_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_irange *mdata = data;
    const struct sol_flow_node_type_converter_boolean_to_int_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_INT_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_boolean_to_int_options *)options;

    mdata->min = opts->false_value.val;
    mdata->max = opts->true_value.val;

    return 0;
}

static int
boolean_to_irange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    int r;
    bool in_value;

    r = sol_flow_packet_get_boolean(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    return sol_flow_send_irange_value_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_INT__OUT__OUT,
        in_value ? mdata->max : mdata->min);
}

static int
irange_to_boolean_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_irange *mdata = data;
    const struct sol_flow_node_type_converter_int_to_boolean_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_BOOLEAN_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_int_to_boolean_options *)options;

    if (opts->true_range.max >= opts->true_range.min) {
        mdata->min = opts->true_range.min;
        mdata->max = opts->true_range.max;
    } else {
        SOL_WRN("min (%" PRId32 ") should be smaller than max (%" PRId32 ").",
            opts->true_range.min, opts->true_range.max);
        mdata->min = opts->true_range.max;
        mdata->max = opts->true_range.min;
    }

    return 0;
}

static int
irange_to_boolean_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    int32_t in_value;
    int r;
    bool out_value;

    r = sol_flow_packet_get_irange_value(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if ((in_value < mdata->min) || (in_value > mdata->max))
        out_value = false;
    else
        out_value = true;

    return sol_flow_send_boolean_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_BOOLEAN__OUT__OUT,
        out_value);
}

static int
boolean_to_drange_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_drange *mdata = data;
    const struct sol_flow_node_type_converter_boolean_to_float_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_FLOAT_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_boolean_to_float_options *)options;

    mdata->min = opts->false_value.val;
    mdata->max = opts->true_value.val;

    return 0;
}

static int
boolean_to_drange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange *mdata = data;
    int r;
    bool in_value;

    r = sol_flow_packet_get_boolean(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    return sol_flow_send_drange_value_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_FLOAT__OUT__OUT,
        in_value ? mdata->max : mdata->min);
}

static int
drange_to_boolean_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_drange *mdata = data;
    const struct sol_flow_node_type_converter_float_to_boolean_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_BOOLEAN_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_float_to_boolean_options *)options;

    if (opts->true_range.max >= opts->true_range.min) {
        mdata->min = opts->true_range.min;
        mdata->max = opts->true_range.max;
    } else {
        SOL_WRN("min %f should be smaller than max %f.",
            opts->true_range.min, opts->true_range.max);
        mdata->min = opts->true_range.max;
        mdata->max = opts->true_range.min;
    }

    return 0;
}

static int
drange_to_boolean_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    double in_value;
    struct sol_drange *mdata = data;
    int r;
    bool out_value;

    r = sol_flow_packet_get_drange_value(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if ((in_value < mdata->min) || (in_value > mdata->max))
        out_value = false;
    else
        out_value = true;

    return sol_flow_send_boolean_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_BOOLEAN__OUT__OUT,
        out_value);
}

static int
boolean_to_byte_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_byte *mdata = data;
    const struct sol_flow_node_type_converter_boolean_to_byte_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_BYTE_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_boolean_to_byte_options *)options;

    mdata->min = opts->false_value;
    mdata->max = opts->true_value;

    return 0;
}

static int
boolean_to_byte_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;
    int r;
    bool in_value;

    r = sol_flow_packet_get_boolean(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    return sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_BYTE__OUT__OUT,
        in_value ? mdata->max : mdata->min);
}

static int
byte_to_boolean_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_byte *mdata = data;
    const struct sol_flow_node_type_converter_byte_to_boolean_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_BYTE_TO_BOOLEAN_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_byte_to_boolean_options *)options;

    if (opts->true_max >= opts->true_min) {
        mdata->min = opts->true_min;
        mdata->max = opts->true_max;
    } else {
        SOL_WRN("min %02x should be smaller than max %02x.",
            opts->true_min, opts->true_max);
        mdata->min = opts->true_max;
        mdata->max = opts->true_min;
    }

    return 0;
}

static int
byte_to_boolean_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;
    int r;
    unsigned char in_value;
    bool out_value;

    r = sol_flow_packet_get_byte(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if ((in_value < mdata->min) || (in_value > mdata->max))
        out_value = false;
    else
        out_value = true;

    return sol_flow_send_boolean_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BYTE_TO_BOOLEAN__OUT__OUT,
        out_value);
}

static int
byte_to_irange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange out_value = { 0, 0, 255, 1 };
    int r;
    unsigned char in_value;

    r = sol_flow_packet_get_byte(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    out_value.val = (int32_t)in_value;

    return sol_flow_send_irange_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BYTE_TO_INT__OUT__OUT,
        &out_value);
}

static int
byte_to_drange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange out_value = { 0, 0, 255, 0 };
    int r;
    unsigned char in_value;

    r = sol_flow_packet_get_byte(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    out_value.val = (double)in_value;

    return sol_flow_send_drange_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BYTE_TO_FLOAT__OUT__OUT,
        &out_value);
}

static int
irange_to_byte_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange in_value;
    int r;
    unsigned char out_value;

    r = sol_flow_packet_get_irange(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if (in_value.val < 0)
        out_value = 0;
    else if (in_value.val > 255)
        out_value = 255;
    else
        out_value = (char)in_value.val;

    return sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_BYTE__OUT__OUT,
        out_value);
}

static int
drange_to_byte_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange in_value;
    int r;
    unsigned char out_value;

    r = sol_flow_packet_get_drange(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if (in_value.val < 0)
        out_value = 0;
    else if (in_value.val > 255)
        out_value = 255;
    else
        out_value = (char)in_value.val;

    return sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_BYTE__OUT__OUT,
        out_value);
}

static int
irange_to_drange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange in_value;
    struct sol_drange out_value;
    int r;

    r = sol_flow_packet_get_irange(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    out_value.val = (double)in_value.val;
    out_value.min = (double)in_value.min;
    out_value.max = (double)in_value.max;
    out_value.step = (double)in_value.step;

    return sol_flow_send_drange_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_FLOAT__OUT__OUT,
        &out_value);
}

static int
drange_to_irange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange in_value;
    struct sol_irange out_value;
    int r;

    r = sol_flow_packet_get_drange(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    out_value.val = (int32_t)in_value.val;
    out_value.min = (int32_t)in_value.min;
    out_value.max = (int32_t)in_value.max;
    out_value.step = (int32_t)in_value.step;

    return sol_flow_send_irange_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_INT__OUT__OUT,
        &out_value);
}

static int
empty_to_boolean_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_boolean *mdata = data;
    const struct sol_flow_node_type_converter_empty_to_boolean_options *opts =
        (const struct sol_flow_node_type_converter_empty_to_boolean_options *)
        options;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_BOOLEAN_OPTIONS_API_VERSION,
        -EINVAL);

    mdata->output_value = opts->output_value;
    return 0;
}

static int
empty_to_byte_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_byte *mdata = data;
    const struct sol_flow_node_type_converter_empty_to_byte_options *opts =
        (const struct sol_flow_node_type_converter_empty_to_byte_options *)
        options;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_BYTE_OPTIONS_API_VERSION,
        -EINVAL);

    mdata->min = opts->output_value;
    return 0;
}

static int
empty_to_drange_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_drange *mdata = data;
    const struct sol_flow_node_type_converter_empty_to_float_options *opts =
        (const struct sol_flow_node_type_converter_empty_to_float_options *)
        options;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_FLOAT_OPTIONS_API_VERSION,
        -EINVAL);

    *mdata = opts->output_value;
    return 0;
}

static int
empty_to_irange_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_irange *mdata = data;
    const struct sol_flow_node_type_converter_empty_to_int_options *opts =
        (const struct sol_flow_node_type_converter_empty_to_int_options *)
        options;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_INT_OPTIONS_API_VERSION,
        -EINVAL);

    *mdata = opts->output_value;
    return 0;
}

static int
byte_to_empty_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_byte *mdata = data;
    const struct sol_flow_node_type_converter_byte_to_empty_options *opts =
        (const struct sol_flow_node_type_converter_byte_to_empty_options *)
        options;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_BYTE_TO_EMPTY_OPTIONS_API_VERSION,
        -EINVAL);

    mdata->min = opts->range_min;
    mdata->max = opts->range_max;
    return 0;
}

static int
drange_to_empty_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_drange *mdata = data;
    const struct sol_flow_node_type_converter_float_to_empty_options *opts =
        (const struct sol_flow_node_type_converter_float_to_empty_options *)
        options;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_EMPTY_OPTIONS_API_VERSION,
        -EINVAL);

    *mdata = opts->range;
    return 0;
}

static int
irange_to_empty_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_irange *mdata = data;
    const struct sol_flow_node_type_converter_int_to_empty_options *opts =
        (const struct sol_flow_node_type_converter_int_to_empty_options *)
        options;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_EMPTY_OPTIONS_API_VERSION,
        -EINVAL);

    *mdata = opts->range;
    return 0;
}

static int
empty_to_boolean_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_boolean *mdata = data;

    return sol_flow_send_boolean_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_BOOLEAN__OUT__OUT,
        mdata->output_value);
}

static int
empty_to_byte_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;

    return sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_BYTE__OUT__OUT,
        mdata->min);
}

static int
empty_to_drange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange *mdata = data;

    return sol_flow_send_drange_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_FLOAT__OUT__OUT,
        mdata);
}

static int
empty_to_irange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;

    return sol_flow_send_irange_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_INT__OUT__OUT,
        mdata);
}

static int
send_empty_packet(struct sol_flow_node *node, uint16_t port)
{
    return sol_flow_send_empty_packet(node, port);
}

static int
pulse_if_true(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    bool in_value;
    int r;

    r = sol_flow_packet_get_boolean(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if (!in_value)
        return 0;

    return send_empty_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_EMPTY__OUT__OUT);
}

static int
pulse_if_false(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    bool in_value;
    int r;

    r = sol_flow_packet_get_boolean(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if (in_value)
        return 0;

    return send_empty_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_EMPTY__OUT__OUT);
}

static int
byte_to_empty_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;
    unsigned char in_value;
    int r;

    r = sol_flow_packet_get_byte(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if ((in_value < mdata->min) || (in_value > mdata->max))
        return 0;

    return send_empty_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BYTE_TO_EMPTY__OUT__OUT);
}

static int
drange_to_empty_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange *mdata = data;
    double in_value;
    int r;

    r = sol_flow_packet_get_drange_value(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if ((in_value < mdata->min) || (in_value > mdata->max))
        return 0;

    return send_empty_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_EMPTY__OUT__OUT);
}

static int
irange_to_empty_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    int32_t in_value;
    int r;

    r = sol_flow_packet_get_irange_value(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if ((in_value < mdata->min) || (in_value > mdata->max))
        return 0;

    return send_empty_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_EMPTY__OUT__OUT);
}

#define STR_SIZE (32)

static int
drange_to_string_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    double in_value;
    int r;
    char out_value[STR_SIZE];

    r = sol_flow_packet_get_drange_value(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    snprintf(out_value, STR_SIZE, "%f", in_value);

    return sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_STRING__OUT__OUT,
        out_value);
}

static int
irange_to_string_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    int32_t in_value;
    int r;
    char out_value[STR_SIZE];

    r = sol_flow_packet_get_irange_value(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    snprintf(out_value, STR_SIZE, "%d", in_value);

    return sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_STRING__OUT__OUT,
        out_value);
}

#undef STR_SIZE

static int
boolean_to_string_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_boolean_string *mdata = data;
    const struct sol_flow_node_type_converter_boolean_to_string_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_STRING_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_boolean_to_string_options *)options;

    if (!opts->false_value) {
        SOL_WRN("A valid string is required as 'false_value'");
        return -EINVAL;
    }

    if (!opts->true_value) {
        SOL_WRN("A valid string is required as 'true_value'");
        return -EINVAL;
    }

    mdata->false_value = strdup(opts->false_value);
    SOL_NULL_CHECK(mdata->false_value, -ENOMEM);

    mdata->true_value = strdup(opts->true_value);
    SOL_NULL_CHECK_GOTO(mdata->true_value, string_error);

    return 0;

string_error:
    free(mdata->false_value);
    return -ENOMEM;
}

static void
boolean_to_string_close(struct sol_flow_node *node, void *data)
{
    struct sol_converter_boolean_string *mdata = data;

    free(mdata->false_value);
    free(mdata->true_value);
}

static int
set_string(const struct sol_flow_packet *packet, char **string)
{
    const char *in_value;
    int r;

    r = sol_flow_packet_get_string(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    free(*string);
    *string = strdup(in_value);

    if (!(*string))
        return -ENOMEM;

    return 0;
}

static int
string_false_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_boolean_string *mdata = data;

    return set_string(packet, &mdata->false_value);
}

static int
string_true_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_boolean_string *mdata = data;

    return set_string(packet, &mdata->true_value);
}

static int
boolean_to_string_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_boolean_string *mdata = data;
    int r;
    bool in_value;

    r = sol_flow_packet_get_boolean(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    return sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BOOLEAN_TO_STRING__OUT__OUT,
        in_value ? mdata->true_value : mdata->false_value);
}

#define BYTE_STR_SIZE (5)

static int
byte_to_string_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    unsigned char in_value;
    int r;
    char out_value[BYTE_STR_SIZE];

    r = sol_flow_packet_get_byte(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    snprintf(out_value, BYTE_STR_SIZE, "0x%02x", in_value);

    return sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BYTE_TO_STRING__OUT__OUT,
        out_value);
}

#undef BYTE_STR_SIZE

static int
empty_to_string_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_string *mdata = data;
    const struct sol_flow_node_type_converter_empty_to_string_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_STRING_OPTIONS_API_VERSION,
        -EINVAL);
    opts = (const struct sol_flow_node_type_converter_empty_to_string_options *)
           options;

    mdata->string = strdup(opts->output_value);
    if (!mdata->string)
        return -ENOMEM;

    return 0;
}

static void
empty_to_string_close(struct sol_flow_node *node, void *data)
{
    struct sol_converter_string *mdata = data;

    free(mdata->string);
}

static int
empty_string_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_string *mdata = data;

    return set_string(packet, &mdata->string);
}

static int
empty_to_string_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_string *mdata = data;

    return sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_EMPTY_TO_STRING__OUT__OUT,
        mdata->string);
}

static int
empty_boolean_output_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_boolean *mdata = data;
    int r = sol_flow_packet_get_boolean(packet, &mdata->output_value);

    SOL_INT_CHECK(r, < 0, r);
    return 0;
}

static int
empty_byte_min_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;
    int r = sol_flow_packet_get_byte(packet, &mdata->min);

    SOL_INT_CHECK(r, < 0, r);
    return 0;
}

static int
empty_byte_max_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_byte *mdata = data;
    int r = sol_flow_packet_get_byte(packet, &mdata->max);

    SOL_INT_CHECK(r, < 0, r);
    return 0;
}

static int
empty_drange_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange *mdata = data;
    int r = sol_flow_packet_get_drange(packet, mdata);

    SOL_INT_CHECK(r, < 0, r);
    return 0;
}

static int
empty_irange_value_set(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange *mdata = data;
    int r = sol_flow_packet_get_irange(packet, mdata);

    SOL_INT_CHECK(r, < 0, r);
    return 0;
}

static int
byte_to_bits_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_bits *mdata = data;
    int i;
    unsigned char in_val, last_bit, next_bit;

    sol_flow_packet_get_byte(packet, &in_val);

    for (i = 0; i <= 7; i++) {
        last_bit = (mdata->last >> i) & 1;
        next_bit = (in_val >> i) & 1;

        if (mdata->output_initialized && last_bit == next_bit)
            continue;

        sol_flow_send_boolean_packet(node, i, next_bit);
    }

    mdata->last = in_val;
    mdata->output_initialized = 1;

    return 0;
}

static int
string_to_boolean_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    const char *in_value;
    int r;
    bool out_value = false;

    r = sol_flow_packet_get_string(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    if (!strcasecmp(in_value, "true"))
        out_value = true;
    else if (strcasecmp(in_value, "false")) {
        SOL_WRN("String %s isn't a valid boolean", in_value);
        return -EINVAL;
    }

    return sol_flow_send_boolean_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_STRING_TO_BOOLEAN__OUT__OUT,
        out_value);
}

static int
string_to_byte_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    const char *in_value;
    char *endptr;
    int r;
    unsigned char out_value;

    r = sol_flow_packet_get_string(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    errno = 0;
    out_value = strtol(in_value, &endptr, 16);
    if (errno) {
        SOL_WRN("Failed to convert string to byte %s: %d", in_value, errno);
        return -errno;
    }

    // no error, but no conversion done
    if (in_value == endptr) {
        SOL_WRN("Failed to convert string to byte %s", in_value);
        return -EINVAL;
    }

    return sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_STRING_TO_BYTE__OUT__OUT,
        out_value);
}

static int
string_to_drange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    double out_value;
    const char *in_value;
    char *endptr;
    int r;

    r = sol_flow_packet_get_string(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    errno = 0;
    out_value = strtod(in_value, &endptr);
    if (errno) {
        SOL_WRN("Failed to convert string to float %s: %d", in_value, errno);
        return -errno;
    }

    if (in_value == endptr) {
        SOL_WRN("Failed to convert string to float %s", in_value);
        return -EINVAL;
    }

    return sol_flow_send_drange_value_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_STRING_TO_FLOAT__OUT__OUT,
        out_value);
}

static int
string_to_empty_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    return sol_flow_send_empty_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_STRING_TO_EMPTY__OUT__OUT);
}

static int
string_to_irange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    const char *in_value;
    char *endptr;
    uint32_t out_value;
    int r;

    r = sol_flow_packet_get_string(packet, &in_value);
    SOL_INT_CHECK(r, < 0, r);

    errno = 0;
    out_value = strtol(in_value, &endptr, 10);
    if (errno) {
        SOL_WRN("Failed to convert string to int %s: %d", in_value, errno);
        return -errno;
    }

    if (in_value == endptr) {
        SOL_WRN("Failed to convert string to int %s", in_value);
        return -EINVAL;
    }

    return sol_flow_send_irange_value_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_STRING_TO_INT__OUT__OUT,
        out_value);
}

static int
byte_to_rgb_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_rgb *mdata = data;

    mdata->output_value.red_max = 255;
    mdata->output_value.green_max = 255;
    mdata->output_value.blue_max = 255;

    return 0;
}

static int
irange_to_rgb_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_rgb *mdata = data;
    const struct sol_flow_node_type_converter_int_to_rgb_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_INT_TO_RGB_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_int_to_rgb_options *)options;

    mdata->output_value.red_max = opts->red_max.val;
    mdata->output_value.green_max = opts->green_max.val;
    mdata->output_value.blue_max = opts->blue_max.val;

    return 0;
}

static int
drange_to_rgb_open(struct sol_flow_node *node, void *data, const struct sol_flow_node_options *options)
{
    struct sol_converter_rgb *mdata = data;
    const struct sol_flow_node_type_converter_float_to_rgb_options *opts;

    SOL_FLOW_NODE_OPTIONS_SUB_API_CHECK(options,
        SOL_FLOW_NODE_TYPE_CONVERTER_FLOAT_TO_RGB_OPTIONS_API_VERSION,
        -EINVAL);

    opts = (const struct sol_flow_node_type_converter_float_to_rgb_options *)options;

    mdata->output_value.red_max = opts->red_max.val;
    mdata->output_value.green_max = opts->green_max.val;
    mdata->output_value.blue_max = opts->blue_max.val;

    return 0;
}

static int
rgb_convert(struct sol_flow_node *node, void *data, uint16_t port, uint32_t val)
{
    struct sol_converter_rgb *mdata = data;
    int i;

    mdata->output_initialized[port] = true;
    if (port == 0)
        mdata->output_value.red = val > mdata->output_value.red_max ?
                                  mdata->output_value.red_max : val;
    else if (port == 1)
        mdata->output_value.green = val > mdata->output_value.green_max ?
                                    mdata->output_value.green_max : val;
    else
        mdata->output_value.blue = val > mdata->output_value.blue_max ?
                                   mdata->output_value.blue_max : val;

    for (i = 0; i < 3; i++) {
        if (!mdata->output_initialized[i])
            return 0;
    }

    return sol_flow_send_rgb_packet(node, 0, &mdata->output_value);
}

static int
byte_to_rgb_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    unsigned char in_val;
    int r;

    r = sol_flow_packet_get_byte(packet, &in_val);
    SOL_INT_CHECK(r, < 0, r);

    return rgb_convert(node, data, port, in_val);
}

static inline uint32_t
rgb_get_port_max(void *data, uint16_t port)
{
    struct sol_converter_rgb *mdata = data;

    if (port == 0)
        return mdata->output_value.red_max;
    if (port == 1)
        return mdata->output_value.green_max;
    return mdata->output_value.blue_max;
}

static int
irange_to_rgb_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange in_val;
    uint32_t val;
    int r;

    r = sol_flow_packet_get_irange(packet, &in_val);
    SOL_INT_CHECK(r, < 0, r);

    if (in_val.val < 0) {
        SOL_WRN("Color component must to be a not negative value");
        return -EINVAL;
    }

    if (in_val.max <= 0) {
        SOL_WRN("Max value for color component must to be a positive value");
        return -EINVAL;
    }

    val = in_val.val * rgb_get_port_max(data, port) / in_val.max;

    return rgb_convert(node, data, port, val);
}

static int
drange_to_rgb_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_drange in_val;
    uint32_t val;
    int r;

    r = sol_flow_packet_get_drange(packet, &in_val);
    SOL_INT_CHECK(r, < 0, r);

    if (isless(in_val.val, 0)) {
        SOL_WRN("Color component must to be a not negative value");
        return -EINVAL;
    }

    if (islessequal(in_val.max, 0)) {
        SOL_WRN("Max value for color component must to be a positive value");
        return -EINVAL;
    }

    val = in_val.val * rgb_get_port_max(data, port) / in_val.max;

    return rgb_convert(node, data, port, val);
}

static int
rgb_to_byte_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_rgb rgb;
    int r;

    r = sol_flow_packet_get_rgb(packet, &rgb);
    SOL_INT_CHECK(r, < 0, r);

    r = sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_RGB_TO_BYTE__OUT__RED, rgb.red);
    SOL_INT_CHECK(r, < 0, r);

    r = sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_RGB_TO_BYTE__OUT__GREEN, rgb.green);
    SOL_INT_CHECK(r, < 0, r);

    return sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_RGB_TO_BYTE__OUT__BLUE, rgb.blue);
}

#define RGB_SEND(_type, _type_m, _component, _component_m) \
    do { \
        out.val = rgb._component; \
        out.max = rgb._component ## _max; \
        r = sol_flow_send_ ## _type ## _packet(node, \
            SOL_FLOW_NODE_TYPE_CONVERTER_RGB_TO_ ## _type_m ## __OUT__ ## _component_m, \
            &out); \
        SOL_INT_CHECK(r, < 0, r); \
    } while (0)

static int
rgb_to_irange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_rgb rgb;
    struct sol_irange out;
    int r;

    r = sol_flow_packet_get_rgb(packet, &rgb);
    SOL_INT_CHECK(r, < 0, r);

    out.min = 0;
    out.step = 1;

    RGB_SEND(irange, INT, red, RED);
    RGB_SEND(irange, INT, green, GREEN);
    RGB_SEND(irange, INT, blue, BLUE);

    return r;
}

static int
rgb_to_drange_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_rgb rgb;
    struct sol_drange out;
    int r;

    r = sol_flow_packet_get_rgb(packet, &rgb);
    SOL_INT_CHECK(r, < 0, r);

    out.min = 0;
    out.step = 1;

    RGB_SEND(drange, FLOAT, red, RED);
    RGB_SEND(drange, FLOAT, green, GREEN);
    RGB_SEND(drange, FLOAT, blue, BLUE);

    return r;
}

#undef RGB_SEND

static int
irange_compose_connect(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id)
{
    struct sol_converter_irange_compose *mdata = data;

    mdata->connected_ports |= 1 << port;
    return 0;
}

static int
irange_compose(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_irange_compose *mdata = data;
    struct sol_irange out_val = { .min = INT32_MIN, .max = INT32_MAX, .step = 1 };
    unsigned int tmp;
    unsigned char in_val;
    int r;

    r = sol_flow_packet_get_byte(packet, &in_val);
    SOL_INT_CHECK(r, < 0, r);

    mdata->port_has_value |= 1 << port;

    tmp = mdata->output_value & ~(0xff << (port * CHAR_BIT));
    tmp |= in_val << (port * CHAR_BIT);
    mdata->output_value = tmp;

    if (mdata->port_has_value != mdata->connected_ports)
        return 0;

    out_val.val = (int)mdata->output_value;
    return sol_flow_send_irange_packet(node, SOL_FLOW_NODE_TYPE_CONVERTER_INT_COMPOSE__OUT__OUT, &out_val);
}

static int
irange_decompose(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_irange in;
    unsigned int value;
    int r, i;

    r = sol_flow_packet_get_irange(packet, &in);
    SOL_INT_CHECK(r, < 0, r);

    value = (unsigned)in.val;
    for (i = 0; i < 4; i++) {
        uint16_t out_port[] = {
            SOL_FLOW_NODE_TYPE_CONVERTER_INT_DECOMPOSE__OUT__OUT0,
            SOL_FLOW_NODE_TYPE_CONVERTER_INT_DECOMPOSE__OUT__OUT1,
            SOL_FLOW_NODE_TYPE_CONVERTER_INT_DECOMPOSE__OUT__OUT2,
            SOL_FLOW_NODE_TYPE_CONVERTER_INT_DECOMPOSE__OUT__OUT3
        };
        unsigned char byte;
        byte = value & 0xff;
        value >>= CHAR_BIT;
        r = sol_flow_send_byte_packet(node, out_port[i], byte);
        SOL_INT_CHECK(r, < 0, r);
    }

    return 0;
}

static int
error_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    const char *msg;
    int code, r;

    r = sol_flow_packet_get_error(packet, &code, &msg);
    SOL_INT_CHECK(r, < 0, r);

    r = sol_flow_send_irange_value_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_ERROR__OUT__CODE, code);
    SOL_INT_CHECK(r, < 0, r);

    return sol_flow_send_string_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_ERROR__OUT__MESSAGE, msg);
}

static int
bits_to_byte_connect(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id)
{
    struct sol_converter_bits *mdata = data;

    mdata->connected_ports |= 1 << port;
    return 0;
}

#define SET_BIT(_byte, _bit_index, _bit_value) \
    do { \
        _byte = (~(1 << _bit_index) & _byte) | (_bit_value << _bit_index); \
    } while (0)

static int
bits_to_byte_convert(struct sol_flow_node *node, void *data, uint16_t port, uint16_t conn_id, const struct sol_flow_packet *packet)
{
    struct sol_converter_bits *mdata = data;
    bool in_val;

    sol_flow_packet_get_boolean(packet, &in_val);

    if ((mdata->output_initialized >> port) & 1) {
        if ((mdata->last >> port) == in_val)
            return 0;
    } else
        mdata->output_initialized |= 1 << port;

    SET_BIT(mdata->last, port, in_val);

    if (mdata->output_initialized != mdata->connected_ports)
        return 0;

    return sol_flow_send_byte_packet(node,
        SOL_FLOW_NODE_TYPE_CONVERTER_BITS_TO_BYTE__OUT__OUT, mdata->last);
}

#undef SET_BIT

#include "converter-gen.c"
