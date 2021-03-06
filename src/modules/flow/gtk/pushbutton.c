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

#include "pushbutton.h"
#include "gtk-gen.h"

static void
on_pushbutton_pressed(GtkButton *button, gpointer data)
{
    struct gtk_common_data *mdata = data;

    sol_flow_send_boolean_packet(mdata->node,
        SOL_FLOW_NODE_TYPE_GTK_PUSHBUTTON__OUT__OUT, true);
}

static void
on_pushbutton_released(GtkButton *button, gpointer data)
{
    struct gtk_common_data *mdata = data;

    sol_flow_send_boolean_packet(mdata->node,
        SOL_FLOW_NODE_TYPE_GTK_PUSHBUTTON__OUT__OUT, false);
}

static int
pushbutton_setup(struct gtk_common_data *mdata,
    const struct sol_flow_node_options *options)
{
    mdata->widget = gtk_button_new_with_label("    ");
    g_signal_connect(mdata->widget, "pressed",
        G_CALLBACK(on_pushbutton_pressed), mdata);
    g_signal_connect(mdata->widget, "released",
        G_CALLBACK(on_pushbutton_released), mdata);
    g_object_set(mdata->widget, "halign", GTK_ALIGN_CENTER, NULL);

    return 0;
}

DEFINE_DEFAULT_OPEN_CLOSE(pushbutton);
