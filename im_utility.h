#pragma once

#include <string>
#include <vector>
#include <map>

namespace iu
{
	struct key_point
	{
		double x, y, size, angle;
	};

	struct match
	{
		key_point pt1;
		key_point pt2;
		double distance;
	};

	enum parameter_key
	{
		key_unknown
		, key_hessian_threshold
		, key_octaves
		, key_octave_layers
		, key_match_threshold
		, key_speedup
		, key_match_method
	};

	enum speedup_method
	{
		speedup_default 
		, speedup_use_ocl
		, speedup_use_cuda
	};

	enum match_method
	{
		match_brute_force
		, match_flann
	};

	typedef double parameter_value;
	typedef std::vector < key_point > key_points;
	typedef std::vector < match > matches;
	typedef std::map < parameter_key, parameter_value > parameters;

	class im_utility
	{
	public:
		im_utility();
		~im_utility();

	public:
		matches diff(const std::string &im_file1, const std::string &im_file2, const parameters &params, key_points *im1_kps = nullptr, key_points *im2_kps = nullptr);
		void count_objs();
		void extract_text();

	private:

	};

}