/* GIMP Plug-in Template
 * Copyright (C) 2000  Michael Natterer <mitch@gimp.org> (the "Author").
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the Author of the
 * Software shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the Author.
 */

#include "config.h"

#include <assert.h>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>

#include "main.h"
#include "render.h"

#include "plugin-intl.h"

static inline gint to_1d_index (gint x, gint y, gint channel, gint width, gint num_channels);
static inline gdouble out_to_in_coord (gint out, gdouble factor);
static gdouble biakima (gdouble bc[], gdouble by, gdouble bx, gint c, gint channels);
static gdouble akima (gdouble z[], gdouble s);

void render(gint32 image_ID,
            GimpDrawable* drawable,
            PlugInVals* vals,
            PlugInImageVals* image_vals,
            PlugInDrawableVals* drawable_vals)
{
    gint x1, y1, x2, y2;
    gimp_drawable_mask_bounds(drawable->drawable_id, &x1, &y1, &x2, &y2);
    gint channels = gimp_drawable_bpp(drawable->drawable_id);
    gint in_width = x2 - x1;
    gint in_height = y2 - y1;

    GimpPixelRgn rgn_in, rgn_out;
    gimp_pixel_rgn_init(&rgn_in, drawable, x1, y1, in_width, in_height, FALSE, FALSE);
    gimp_pixel_rgn_init(&rgn_out, drawable, x1, y1, in_width, in_height, TRUE, TRUE);

    guchar* in_img_array = g_new(guchar, in_width * in_height * channels);
    gimp_pixel_rgn_get_rect(&rgn_in, in_img_array, x1, y1, in_width, in_height);

    gint out_width = vals->x_size_out;
    gint out_height = vals->y_size_out;
    gdouble x_fact = (gdouble)out_width / in_width;
    gdouble y_fact = (gdouble)out_height / in_height;

    gimp_image_undo_group_start(gimp_item_get_image(drawable->drawable_id));

    if (gimp_image_resize(gimp_item_get_image(drawable->drawable_id), out_width, out_height, 0, 0))
    {
        gimp_layer_resize_to_image_size(
            gimp_image_get_active_layer(gimp_item_get_image(drawable->drawable_id)));

        GimpDrawable* resized_drawable = gimp_drawable_get(
            gimp_image_get_active_drawable(gimp_item_get_image(drawable->drawable_id)));

        GimpPixelRgn dest_rgn;
        gimp_pixel_rgn_init(&dest_rgn, resized_drawable, 0, 0, out_width, out_height, TRUE, TRUE);

        const int out_img_array_size = out_width * out_height * channels;
        guchar* out_img_array = g_new(guchar, out_img_array_size);

        for (int iy = 0; iy < out_height; iy++)
        {
            gdouble in_y = out_to_in_coord(iy, y_fact);
            gint in_y0 = (gint)in_y;
            gdouble by = in_y - in_y0;
            for (int ix = 0; ix < out_width; ix++)
            {
                gdouble in_x = out_to_in_coord(ix, x_fact);
                gint in_x0 = (gint)in_x;
                gdouble bx = in_x - in_x0;

                gdouble bc[6 * 6 * channels];
                gint k = 0;
                for (gint dy = -2; dy < 4; dy++)
                {
                    gint ty = CLAMP(in_y0 + dy, 0, in_height - 1);
                    for (gint dx = -2; dx < 4; dx++)
                    {
                        gint tx = CLAMP(in_x0 + dx, 0, in_width - 1);

                        for (gint c = 0; c < channels; ++c)
                        {
                            bc[k] = (gdouble)in_img_array[to_1d_index(tx, ty, c, in_width, channels)];
                            k++;
                        }
                    }
                }
                for (gint c = 0; c < channels; c++)
                {
                    gint in_value = roundf(CLAMP(biakima (bc, by, bx, c, channels), 0.0, 255.0));
                    out_img_array[to_1d_index(ix, iy, c, out_width, channels)] = in_value;
                }

                if (iy % 10 == 0)
                    gimp_progress_update((gdouble)(iy) / (gdouble)(out_height));
            }
        }

        gimp_pixel_rgn_set_rect(&dest_rgn, (guchar*)out_img_array, 0, 0, out_width, out_height);

        gimp_drawable_flush(resized_drawable);
        gimp_drawable_merge_shadow(resized_drawable->drawable_id, TRUE);
        gimp_drawable_update(resized_drawable->drawable_id, x1, y1, out_width, out_height);

        gimp_drawable_detach(resized_drawable);

        g_free(out_img_array);
    }

    g_free(in_img_array);
    gimp_image_undo_group_end(gimp_item_get_image(drawable->drawable_id));
}

static gdouble biakima (gdouble bc[], gdouble by, gdouble bx, gint c, gint channels)
{
    gint i, k;
    gdouble z[6], zz[6], zzz;

    k = c;
    for (gint iy = 0; iy < 6; iy++)
    {
        for (gint ix = 0; ix < 6; ix++)
        {
            z[ix] = bc[k];
            k += channels;
        }
        zz[iy] = akima (z, bx);
    }
    zzz = akima (zz, by);
    return zzz;
}

static gdouble akima (gdouble z[], gdouble s)
{
    gint i;
    gdouble a, b, f, v, m[5], t[2];

    for(i = 0; i < 5; i++)
    {
        m[i] = z[i + 1] - z[i];
    }
    for(i = 0; i < 2; i++)
    {
        a = (m[i + 2] > m[i + 3]) ? (m[i + 2] - m[i + 3]) : (m[i + 3] - m[i + 2]);
        b = (m[i] > m[i + 1]) ? (m[i] - m[i + 1]) : (m[i + 1] - m[i]);
        t[i] = ((a + b) > 0) ? ((a * m[i + 1] + b * m[i + 2]) / (a + b)) : (0.5 * (m[i + 1] + m[i + 2]));
    }
    v = z[2] + (t[0] + ((m[2] + m[2] + m[2] - t[0] - t[0] - t[1]) + (t[0] + t[1] - m[2] - m[2]) * s) * s) * s;

    return v;
}

static inline gdouble out_to_in_coord(gint out, gdouble factor)
{
    gdouble in;
    in  = ( 0.5 + out) * (1.0 / factor) - 0.5;
    return in;
}

static inline gint to_1d_index(gint x, gint y, gint channel, gint width, gint num_channels)
{
    assert(x >= 0 && y >= 0 && channel >= 0 && channel < num_channels && x < width);
    return (y * width * num_channels) + x * num_channels + channel;
}
