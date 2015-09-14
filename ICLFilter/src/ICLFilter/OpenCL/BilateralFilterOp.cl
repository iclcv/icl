

//__constant float _13 = 1.0/3.0;

float4 rgb_to_cie_lab(const uchar4 rgb) {
	float m1[3][3] = {	{ 0.412453/255.0, 0.35758/255.0 , 0.180423/255.0},
							{ 0.212671/255.0, 0.71516/255.0 , 0.072169/255.0},
							{ 0.019334/255.0, 0.119193/255.0, 0.950227/255.0}	};
	float r = rgb.x, g = rgb.y, b = rgb.z;
	float x = m1[0][0]*r+m1[0][1]*g+m1[0][2]*b;
	float y = m1[1][0]*r+m1[1][1]*g+m1[1][2]*b;
	float z = m1[2][0]*r+m1[2][1]*g+m1[2][2]*b;
	float wXYZ[3] = {	0.950455, 1.0, 1.088753	};
	float XXn = x/wXYZ[0];
	float YYn = y/wXYZ[1];
	float ZZn = z/wXYZ[2];
	float fX = (XXn > 0.008856) ? cbrt(XXn) : 7.787 * XXn + (16.0 / 116.0);
	float fY = (YYn > 0.008856) ? cbrt(YYn) : 7.787 * YYn + (16.0 / 116.0);
	float fZ = (ZZn > 0.008856) ? cbrt(ZZn) : 7.787 * ZZn + (16.0 / 116.0);
	float4 lab = 0.0;
	lab.x = 116.0*2.55f*fY-(16.0*(2.55f));
	lab.y = 500.0*(fX-fY)+128.0f;
	lab.z = 200.0*(fY-fZ)+128.0f;
	return lab;
}

float4 rgb_to_cie_lab2(const uint4 rgb) {
	float m1[3][3] = {	{ 0.412453/255.0, 0.35758/255.0 , 0.180423/255.0},
							{ 0.212671/255.0, 0.71516/255.0 , 0.072169/255.0},
							{ 0.019334/255.0, 0.119193/255.0, 0.950227/255.0}	};
	float r = rgb.x, g = rgb.y, b = rgb.z;
	float x = m1[0][0]*r+m1[0][1]*g+m1[0][2]*b;
	float y = m1[1][0]*r+m1[1][1]*g+m1[1][2]*b;
	float z = m1[2][0]*r+m1[2][1]*g+m1[2][2]*b;
	float wXYZ[3] = {	0.950455, 1.0, 1.088753	};
	float XXn = x/wXYZ[0];
	float YYn = y/wXYZ[1];
	float ZZn = z/wXYZ[2];
	float fX = (XXn > 0.008856) ? cbrt(XXn) : 7.787 * XXn + (16.0 / 116.0);
	float fY = (YYn > 0.008856) ? cbrt(YYn) : 7.787 * YYn + (16.0 / 116.0);
	float fZ = (ZZn > 0.008856) ? cbrt(ZZn) : 7.787 * ZZn + (16.0 / 116.0);
	float4 lab = 0.0;
	lab.x = 116.0*2.55f*fY-(16.0*(2.55f));
	lab.y = 500.0*(fX-fY)+128.0f;
	lab.z = 200.0*(fY-fZ)+128.0f;
	return lab;
}

uchar4 cie_lab_to_rgb(const float4 lab) {
	float m2[3][3] = {	{ 3.2405*255.f, -1.5372*255.f,-0.4985*255.f},
							{-0.9693*255.f, 1.8760*255.f, 0.0416*255.f},
							{ 0.0556*255.f, -0.2040*255.f, 1.0573*255.f}	};
	float fy = (lab.x+16.0*2.55f)/(116.0*2.55f);
	float fx = fy+(lab.y-128.f)/500.0;
	float fz = fy-(lab.z-128.f)/200.0;
	float wXYZ[3] = {	0.950455, 1.0, 1.088753	};
	float _n_const = 16.0/116.0;
	float X = (fx>0.206893f) ?  wXYZ[0]*pow(fx,3) : wXYZ[0]*(fx-_n_const)/7.787f;
	float Y = (fy>0.206893f) ?  pow(fy,3) : (fy-_n_const)/7.787f;
	float Z = (fz>0.206893f) ?  wXYZ[2]*pow(fz,3) : wXYZ[2]*(fz-_n_const)/7.787f;
	uchar4 rgb = 0;
	rgb.x = m2[0][0] * X + m2[0][1] * Y + m2[0][2] * Z;
	rgb.y = m2[1][0] * X + m2[1][1] * Y + m2[1][2] * Z;
	rgb.z = m2[2][0] * X + m2[2][1] * Y + m2[2][2] * Z;
	return rgb;
}

uint4 cie_lab_to_rgb2(const float4 lab) {
	float m2[3][3] = {	{ 3.2405*255.f, -1.5372*255.f,-0.4985*255.f},
							{-0.9693*255.f, 1.8760*255.f, 0.0416*255.f},
							{ 0.0556*255.f, -0.2040*255.f, 1.0573*255.f}	};
	float fy = (lab.x+16.0*2.55f)/(116.0*2.55f);
	float fx = fy+(lab.y-128.f)/500.0;
	float fz = fy-(lab.z-128.f)/200.0;
	float wXYZ[3] = {	0.950455, 1.0, 1.088753	};
	float _n_const = 16.0/116.0;
	float X = (fx>0.206893f) ?  wXYZ[0]*pow(fx,3) : wXYZ[0]*(fx-_n_const)/7.787f;
	float Y = (fy>0.206893f) ?  pow(fy,3) : (fy-_n_const)/7.787f;
	float Z = (fz>0.206893f) ?  wXYZ[2]*pow(fz,3) : wXYZ[2]*(fz-_n_const)/7.787f;
	uint4 rgb = 0;
	rgb.x = min(m2[0][0] * X + m2[0][1] * Y + m2[0][2] * Z,255.f);
	rgb.y = min(m2[1][0] * X + m2[1][1] * Y + m2[1][2] * Z,255.f);
	rgb.z = min(m2[2][0] * X + m2[2][1] * Y + m2[2][2] * Z,255.f);
	return rgb;
}

/*
__kernel void bilateral_filter_color( __global const uchar4* src,
									  const int width,
									  const int height,
									  const int radius_,
									  const float sigma_s,
									  const float sigma_r,
									  const int use_lab,
									  __local float4* lab_local,
									  __global float4* lab_mem,
									  __global uchar4* dst) {

	const int x = get_global_id(0);
	const int y = get_global_id(1);

	const int l_x = get_local_id(0);
	const int l_y = get_local_id(1);

	const int l_w = get_local_size(0);
	const int l_h = get_local_size(1);

	const float s = sigma_s;
	const float r = sigma_r;

	const int radius = radius_;
	const int w = width;
	const int h = height;
	int idx = w * y + x;

	int tlx = max(x-radius, 0);
	int tly = max(y-radius, 0);
	int brx = min(x+radius, w);
	int bry = min(y+radius, h);

	float3 sum = 0;
	float wp = 0;

	uchar4 src_val = src[idx];
	float src_L = src_val.x, src_a = src_val.y, src_b = src_val.y;
	if (use_lab) {
		float4 lab = rgb_to_cie_lab(src_val);
		int l_idx = l_w*l_y+l_x;
		lab_local[l_idx] = lab;

		lab_mem[idx] = lab;
		src_L = lab.x;
		src_a = lab.y;
		src_b = lab.z;
		barrier(CLK_GLOBAL_MEM_FENCE);
	}

	float s2 = s*s;
	float r2 = r*r;

	for(int i=tlx; i< brx; i++) {
		for(int j=tly; j<bry; j++) {
		// cost:
			float delta_dist = (float)((x - i) * (x - i) + (y - j) * (y - j));

			int idx2 = w * j + i;
			float L = 0.0;
			float a = 0.0;
			float b = 0.0;
			if (use_lab) {
				float4 cur_lab = lab_mem[idx2];//rgb_to_cie_lab(cur);
				L = cur_lab.x;
				a = cur_lab.y;
				b = cur_lab.z;
			} else {
				uchar4 cur = src[idx2];
				L = cur.x;
				a = cur.y;
				b = cur.z;
			}
			float delta = (src_L-L)*(src_L-L) + (src_a-a)*(src_a-a) + (src_b-b)*(src_b-b);
			float weight = native_exp( -(delta_dist / s2 + delta / r2) );
			sum.x += weight * L;
			sum.y += weight * a;
			sum.z += weight * b;
			wp += weight;
		}
	}
	if (use_lab) {
		float4 res = 0;
		res.x = sum.x/wp;
		res.y = sum.y/wp;
		res.z = sum.z/wp;
		dst[idx] = cie_lab_to_rgb(res);
	} else {
		uchar4 res = 0;
		res.x = sum.x/wp;
		res.y = sum.y/wp;
		res.z = sum.z/wp;
		dst[idx] = res;
	}
}*/

__kernel void rgbToLABCIE(	__read_only image2d_t r_in,
							__read_only image2d_t g_in,
							__read_only image2d_t b_in,
							__write_only image2d_t r_out,
							__write_only image2d_t g_out,
							__write_only image2d_t b_out) {

	const sampler_t sampler = CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE;// | CLK_FILTER_NEAREST;

	const int x = get_global_id(0);
	const int y = get_global_id(1);

	int2 coords = (int2)(x,y);

	uint4 r = read_imageui(r_in,sampler,coords);
	uint4 g = read_imageui(g_in,sampler,coords);
	uint4 b = read_imageui(b_in,sampler,coords);

	uint4 rgb = (uint4)(r.x,g.x,b.x,0);

	float4 lab = rgb_to_cie_lab2(rgb);

	uint4 _r = (uint4)(lab.x,0,0,1);
	uint4 _g = (uint4)(lab.y,0,0,1);
	uint4 _b = (uint4)(lab.z,0,0,1);

	write_imageui(r_out,coords,_r);
	write_imageui(g_out,coords,_g);
	write_imageui(b_out,coords,_b);

}

__kernel void bilateral_filter_color(	__read_only image2d_t l_in,
											__read_only image2d_t a_in,
											__read_only image2d_t b_in,
											__write_only image2d_t r_out,
											__write_only image2d_t g_out,
											__write_only image2d_t b_out,
											const int width,
											const int height,
											const int radius_,
											const float sigma_s,
											const float sigma_r,
											const int use_lab) {

	const sampler_t sampler = CLK_FILTER_NEAREST | CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE;// | CLK_FILTER_NEAREST;

	const int x = get_global_id(0);
	const int y = get_global_id(1);
	const float s = sigma_s;
	const float r = sigma_r;
	const int radius = radius_;
	const int w = width;
	const int h = height;
	int idx = w * y + x;

	int tlx = max(x-radius, 0);
	int tly = max(y-radius, 0);
	int brx = min(x+radius, w);
	int bry = min(y+radius, h);

	float3 sum = (float3)0;
	float3 sum2 = (float3)0;
	float wp = 0;

	int2 coords = (int2)(x,y);

	uint4 l = read_imageui(l_in,sampler,coords);
	uint4 A = read_imageui(a_in,sampler,coords);
	uint4 B = read_imageui(b_in,sampler,coords);

	float src_L = l.x, src_a = A.x, src_b = B.x;

	float s2 = s*s;
	float r2 = r*r;

	int count = 0;

	for(int i=tlx; i< brx; i++) {
		for(int j=tly; j<bry; j++) {
			float delta_dist = (float)((x - i) * (x - i) + (y - j) * (y - j));

			int2 coords2 = (int2)(i,j);
			uint4 l_ = read_imageui(l_in,sampler,coords2);
			uint4 a_ = read_imageui(a_in,sampler,coords2);
			uint4 b_ = read_imageui(b_in,sampler,coords2);
			float L = l_.x;
			float a = a_.x;
			float b = b_.x;
			float delta = (src_L-L)*(src_L-L) + (src_a-a)*(src_a-a) + (src_b-b)*(src_b-b);
			float weight = native_exp( -(delta_dist / s2 + delta / r2) );
			sum.x += weight * L;
			sum.y += weight * a;
			sum.z += weight * b;
			wp += weight;

			sum2.x += l_.x;
			sum2.y += a_.x;
			sum2.z += b_.x;
		}
	}

	float4 res = 0;
	res.x = sum.x/wp;
	res.y = sum.y/wp;
	res.z = sum.z/wp;
	uint4 res_r = (uint4)((uchar)res.x,0,0,1);
	uint4 res_g = (uint4)((uchar)res.y,0,0,1);
	uint4 res_b = (uint4)((uchar)res.z,0,0,1);
	if (use_lab) {
		uint4 _rgb = cie_lab_to_rgb2(res);
		res_r = (uint4)(_rgb.x,0,0,1);
		res_g = (uint4)(_rgb.y,0,0,1);
		res_b = (uint4)(_rgb.z,0,0,1);
	}
	write_imageui(r_out,coords,res_r);
	write_imageui(g_out,coords,res_g);
	write_imageui(b_out,coords,res_b);

}

/*
__kernel void bilateral_filter_color2(__read_only image2d_t imageInput,
									  const int width,
									  const int height,
									  const int radius_,
									  const float sigma_s,
									  const float sigma_r,
									  const int use_lab,
									  __write_only image2d_t imageOutput) {

	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

	const int x = get_global_id(0);
	const int y = get_global_id(1);
	const float s = sigma_s;
	const float r = sigma_r;

	const int radius = radius_;
	const int w = width;
	const int h = height;
	int idx = w * y + x;

	int tlx = max(x-radius, 0);
	int tly = max(y-radius, 0);
	int brx = min(x+radius, w);
	int bry = min(y+radius, h);

	float3 sum = 0;
	float wp = 0;

	int2 coords = (x,y);

	uint4 src_val = read_imageui(imageInput,sampler,coords);//src[idx];
	float src_L = src_val.x, src_a = src_val.y, src_b = src_val.y;
	if (use_lab) {
		float4 lab = rgb_to_cie_lab2(src_val);
		src_L = lab.x;
		src_a = lab.y;
		src_b = lab.z;
	}

	float s2 = s*s;
	float r2 = r*r;

	for(int i=tlx; i< brx; i++) {
		for(int j=tly; j<bry; j++) {
		// cost:
			float delta_dist = (float)((x - i) * (x - i) + (y - j) * (y - j));

			int idx2 = w * j + i;
			int2 coords2 = (x-i,y-j);
			uint4 cur = read_imageui(imageInput,sampler,coords2);
			float L = cur.x;
			float a = cur.y;
			float b = cur.z;
			if (use_lab) {
				float4 cur_lab = rgb_to_cie_lab2(cur);
				L = cur_lab.x;
				a = cur_lab.y;
				b = cur_lab.z;
			}
			float delta = (src_L-L)*(src_L-L) + (src_a-a)*(src_a-a) + (src_b-b)*(src_b-b);
			float weight = native_exp( -(delta_dist / s2 + delta / r2) );
			sum.x += weight * L;
			sum.y += weight * a;
			sum.z += weight * b;
			wp += weight;
		}
	}
	if (use_lab) {
		float4 res = 0;
		res.x = sum.x/wp;
		res.y = sum.y/wp;
		res.z = sum.z/wp;
		//dst[idx] = cie_lab_to_rgb(res);
		write_imageui(imageOutput,coords,cie_lab_to_rgb2(res));
	} else {
		uint4 res = 0;
		res.x = sum.x/wp;
		res.y = sum.y/wp;
		res.z = sum.z/wp;
		//dst[idx] = res;
		write_imageui(imageOutput,coords,res);
	}
}*/

__kernel void bilateral_filter_mono(__global const uchar* src,
									const int width,
									const int height,
									const int radius_,
									const float sigma_s,
									const float sigma_r,
									const int use_lab,
									__global uchar* dst) {

	const int x = get_global_id(0);
	const int y = get_global_id(1);
	const float s = sigma_s;
	const float r = sigma_r;

	const int radius = radius_;
	int w = width;
	int h = height;

	int tlx = max(x-radius, 0);
	int tly = max(y-radius, 0);
	int brx = min(x+radius, w);
	int bry = min(y+radius, h);

	int idx = w * y + x;
	float sum = 0;
	float wp = 0;

	float src_depth = src[idx];

	float s2 = s*s;
	float r2 = r*r;

	if(src_depth != 0) {
		for(int i=tlx; i< brx; i++) {
			for(int j=tly; j<bry; j++) {
				// cost:
				float delta_dist = (float)((x - i) * (x - i) + (y - j) * (y - j));

				int idx2 = w * j + i;
				float d = src[idx2]; // cost: 0.013s	// TODO : use shared memory?
				float delta_depth = (src_depth - d) * (src_depth - d);
				float weight = native_exp( -(delta_dist / s2 + delta_depth / r2) ); //cost :
				sum += weight * d;
				wp += weight;
			}
		}
		float res = sum / wp;
		dst[idx] = res;
	} else {
		dst[idx] = NAN;
	}
}

__kernel void bilateral_filter_mono_float(	__global const float* src,
											const int width,
											const int height,
											const int radius_,
											const float sigma_s,
											const float sigma_r,
											const int use_lab,
											__global float* dst) {

	const int x = get_global_id(0);
	const int y = get_global_id(1);
	const float s = sigma_s;
	const float r = sigma_r;

	const int radius = radius_;
	int w = width;	// buffer into shared memory?
	int h = height;

	int tlx = max(x-radius, 0);
	int tly = max(y-radius, 0);
	int brx = min(x+radius, w);
	int bry = min(y+radius, h);

	int idx = w * y + x;
	float sum = 0;
	float wp = 0;	// normalizing constant

	float src_depth = src[idx];

	float s2 = s*s;
	float r2 = r*r;

	if(src_depth != 0) {
		for(int i=tlx; i< brx; i++) {
			for(int j=tly; j<bry; j++) {
				float delta_dist = (float)((x - i) * (x - i) + (y - j) * (y - j));

				int idx2 = w * j + i;
				float d = src[idx2]; // cost: 0.013s	// TODO : use shared memory?
				float delta_depth = (src_depth - d) * (src_depth - d);
				float weight = native_exp( -(delta_dist / s2 + delta_depth / r2) ); //cost
				sum += weight * d;
				wp += weight;
			}
		}
		float res = sum / wp;
		dst[idx] = res;
	} else {
		dst[idx] = 0;
	}
}
