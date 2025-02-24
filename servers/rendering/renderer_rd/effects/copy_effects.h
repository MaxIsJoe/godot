/*************************************************************************/
/*  copy_effects.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef COPY_RD_H
#define COPY_RD_H

#include "servers/rendering/renderer_rd/pipeline_cache_rd.h"
#include "servers/rendering/renderer_rd/shaders/effects/blur_raster.glsl.gen.h"
#include "servers/rendering/renderer_rd/shaders/effects/copy.glsl.gen.h"
#include "servers/rendering/renderer_rd/shaders/effects/copy_to_fb.glsl.gen.h"
#include "servers/rendering/renderer_scene_render.h"

#include "servers/rendering_server.h"

namespace RendererRD {

class CopyEffects {
private:
	bool prefer_raster_effects;

	// Blur raster shader

	enum BlurRasterMode {
		BLUR_MIPMAP,

		BLUR_MODE_GAUSSIAN_BLUR,
		BLUR_MODE_GAUSSIAN_GLOW,
		BLUR_MODE_GAUSSIAN_GLOW_AUTO_EXPOSURE,
		BLUR_MODE_COPY,

		BLUR_MODE_MAX
	};

	enum {
		BLUR_FLAG_HORIZONTAL = (1 << 0),
		BLUR_FLAG_USE_ORTHOGONAL_PROJECTION = (1 << 1),
		BLUR_FLAG_GLOW_FIRST_PASS = (1 << 2),
	};

	struct BlurRasterPushConstant {
		float pixel_size[2];
		uint32_t flags;
		uint32_t pad;

		//glow
		float glow_strength;
		float glow_bloom;
		float glow_hdr_threshold;
		float glow_hdr_scale;

		float glow_exposure;
		float glow_white;
		float glow_luminance_cap;
		float glow_auto_exposure_grey;

		float luminance_multiplier;
		float res1;
		float res2;
		float res3;
	};

	struct BlurRaster {
		BlurRasterPushConstant push_constant;
		BlurRasterShaderRD shader;
		RID shader_version;
		PipelineCacheRD pipelines[BLUR_MODE_MAX];
	} blur_raster;

	// Copy shader

	enum CopyMode {
		COPY_MODE_GAUSSIAN_COPY,
		COPY_MODE_GAUSSIAN_COPY_8BIT,
		COPY_MODE_GAUSSIAN_GLOW,
		COPY_MODE_GAUSSIAN_GLOW_AUTO_EXPOSURE,
		COPY_MODE_SIMPLY_COPY,
		COPY_MODE_SIMPLY_COPY_8BIT,
		COPY_MODE_SIMPLY_COPY_DEPTH,
		COPY_MODE_SET_COLOR,
		COPY_MODE_SET_COLOR_8BIT,
		COPY_MODE_MIPMAP,
		COPY_MODE_LINEARIZE_DEPTH,
		COPY_MODE_CUBE_TO_PANORAMA,
		COPY_MODE_CUBE_ARRAY_TO_PANORAMA,
		COPY_MODE_MAX,

	};

	enum {
		COPY_FLAG_HORIZONTAL = (1 << 0),
		COPY_FLAG_USE_COPY_SECTION = (1 << 1),
		COPY_FLAG_USE_ORTHOGONAL_PROJECTION = (1 << 2),
		COPY_FLAG_DOF_NEAR_FIRST_TAP = (1 << 3),
		COPY_FLAG_GLOW_FIRST_PASS = (1 << 4),
		COPY_FLAG_FLIP_Y = (1 << 5),
		COPY_FLAG_FORCE_LUMINANCE = (1 << 6),
		COPY_FLAG_ALL_SOURCE = (1 << 7),
		COPY_FLAG_HIGH_QUALITY_GLOW = (1 << 8),
		COPY_FLAG_ALPHA_TO_ONE = (1 << 9),
	};

	struct CopyPushConstant {
		int32_t section[4];
		int32_t target[2];
		uint32_t flags;
		uint32_t pad;
		// Glow.
		float glow_strength;
		float glow_bloom;
		float glow_hdr_threshold;
		float glow_hdr_scale;

		float glow_exposure;
		float glow_white;
		float glow_luminance_cap;
		float glow_auto_exposure_grey;
		// DOF.
		float camera_z_far;
		float camera_z_near;
		uint32_t pad2[2];
		//SET color
		float set_color[4];
	};

	struct Copy {
		CopyPushConstant push_constant;
		CopyShaderRD shader;
		RID shader_version;
		RID pipelines[COPY_MODE_MAX];

	} copy;

	// Copy to FB shader

	enum CopyToFBMode {
		COPY_TO_FB_COPY,
		COPY_TO_FB_COPY_PANORAMA_TO_DP,
		COPY_TO_FB_COPY2,

		COPY_TO_FB_MULTIVIEW,
		COPY_TO_FB_MULTIVIEW_WITH_DEPTH,
		COPY_TO_FB_MAX,
	};

	struct CopyToFbPushConstant {
		float section[4];
		float pixel_size[2];
		uint32_t flip_y;
		uint32_t use_section;

		uint32_t force_luminance;
		uint32_t alpha_to_zero;
		uint32_t srgb;
		uint32_t pad;
	};

	struct CopyToFb {
		CopyToFbPushConstant push_constant;
		CopyToFbShaderRD shader;
		RID shader_version;
		PipelineCacheRD pipelines[COPY_TO_FB_MAX];

	} copy_to_fb;

	static CopyEffects *singleton;

public:
	static CopyEffects *get_singleton();

	CopyEffects(bool p_prefer_raster_effects);
	~CopyEffects();

	void copy_to_rect(RID p_source_rd_texture, RID p_dest_texture, const Rect2i &p_rect, bool p_flip_y = false, bool p_force_luminance = false, bool p_all_source = false, bool p_8_bit_dst = false, bool p_alpha_to_one = false);
	void copy_cubemap_to_panorama(RID p_source_cube, RID p_dest_panorama, const Size2i &p_panorama_size, float p_lod, bool p_is_array);
	void copy_depth_to_rect(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2i &p_rect, bool p_flip_y = false);
	void copy_depth_to_rect_and_linearize(RID p_source_rd_texture, RID p_dest_texture, const Rect2i &p_rect, bool p_flip_y, float p_z_near, float p_z_far);
	void copy_to_fb_rect(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2i &p_rect, bool p_flip_y = false, bool p_force_luminance = false, bool p_alpha_to_zero = false, bool p_srgb = false, RID p_secondary = RID(), bool p_multiview = false);
	void copy_to_atlas_fb(RID p_source_rd_texture, RID p_dest_framebuffer, const Rect2 &p_uv_rect, RD::DrawListID p_draw_list, bool p_flip_y = false, bool p_panorama = false);
	void copy_raster(RID p_source_texture, RID p_dest_framebuffer);

	void gaussian_blur(RID p_source_rd_texture, RID p_texture, const Rect2i &p_region, bool p_8bit_dst = false);
	void gaussian_glow(RID p_source_rd_texture, RID p_back_texture, const Size2i &p_size, float p_strength = 1.0, bool p_high_quality = false, bool p_first_pass = false, float p_luminance_cap = 16.0, float p_exposure = 1.0, float p_bloom = 0.0, float p_hdr_bleed_threshold = 1.0, float p_hdr_bleed_scale = 1.0, RID p_auto_exposure = RID(), float p_auto_exposure_grey = 1.0);
	void gaussian_glow_raster(RID p_source_rd_texture, float p_luminance_multiplier, RID p_framebuffer_half, RID p_rd_texture_half, RID p_dest_framebuffer, const Size2i &p_size, float p_strength = 1.0, bool p_high_quality = false, bool p_first_pass = false, float p_luminance_cap = 16.0, float p_exposure = 1.0, float p_bloom = 0.0, float p_hdr_bleed_threshold = 1.0, float p_hdr_bleed_scale = 1.0, RID p_auto_exposure = RID(), float p_auto_exposure_grey = 1.0);

	void make_mipmap(RID p_source_rd_texture, RID p_dest_texture, const Size2i &p_size);
	void make_mipmap_raster(RID p_source_rd_texture, RID p_dest_framebuffer, const Size2i &p_size);

	void set_color(RID p_dest_texture, const Color &p_color, const Rect2i &p_region, bool p_8bit_dst = false);
};

} // namespace RendererRD

#endif // !COPY_RD_H
