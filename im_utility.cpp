#include "im_utility.h"

#include <opencv2\opencv_modules.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\nonfree\nonfree.hpp>
#include <opencv2\gpu\gpu.hpp>
#include <opencv2\nonfree\gpu.hpp>
#include <opencv2\ocl\ocl.hpp>
#include <opencv2\nonfree\ocl.hpp>

//#include <string>
#include <iostream>

#define OPENCV_VER "249"

#ifdef _DEBUG
#define OPENCV_LIB_0(name, ver) \
	"opencv_"#name##ver##"d"
#else
#define OPENCV_LIB_0(name, ver) \
	"opencv_"#name##ver
#endif

#define OPENCV_LIB(name) OPENCV_LIB_0(name, OPENCV_VER)

#pragma comment(lib, OPENCV_LIB(core))
#pragma comment(lib, OPENCV_LIB(highgui))
#pragma comment(lib, OPENCV_LIB(features2d))
#pragma comment(lib, OPENCV_LIB(flann))
#pragma comment(lib, OPENCV_LIB(nonfree))
#pragma comment(lib, OPENCV_LIB(gpu))
#pragma comment(lib, OPENCV_LIB(ocl))

namespace iu
{
	im_utility::im_utility()
	{
	}


	im_utility::~im_utility()
	{
	}

	matches im_utility::diff(const std::string &im_file1, const std::string &im_file2, const parameters &params, key_points *im1_kps, key_points *im2_kps)
	{
		using namespace cv;
		matches mt;
		std::vector<KeyPoint> im1_key_points, im2_key_points;
		std::vector<DMatch> matches;
		std::vector<DMatch> good_matches;

		double min_hessian = 400.0;
		if (params.find(key_hessian_threshold) != params.end()) min_hessian = params.at(key_hessian_threshold);
		double sigma = 2.0;
		if (params.find(key_match_threshold) != params.end()) sigma = params.at(key_match_threshold);
		int speedup = speedup_default;
		if (params.find(key_speedup) != params.end())
			speedup = params.at(key_speedup);

		//auto info = getBuildInformation();
		
		if (speedup == speedup_use_cuda)
		{
			using namespace gpu;
			DeviceInfo dev;
			auto name = dev.name();
			GpuMat img1, img2;

			// upload data
			img1.upload(imread(im_file1, CV_LOAD_IMAGE_GRAYSCALE));
			img2.upload(imread(im_file2, CV_LOAD_IMAGE_GRAYSCALE));

			// detect keypoints & computing descriptors
			SURF_GPU surf;
			GpuMat kps_gpu1, kps_gpu2;
			GpuMat desc_gpu1, desc_gpu2;

			surf(img1, GpuMat(), kps_gpu1, desc_gpu1);
			surf(img2, GpuMat(), kps_gpu2, desc_gpu2);

			// matching descriptors
			BFMatcher_GPU matcher_gpu(NORM_L2);
			GpuMat trainIdx, distance;
			matcher_gpu.matchSingle(desc_gpu1, desc_gpu2, trainIdx, distance);

			// download results
			std::vector<float> desc1, desc2;
			surf.downloadKeypoints(kps_gpu1, im1_key_points);
			surf.downloadKeypoints(kps_gpu2, im2_key_points);
			surf.downloadDescriptors(desc_gpu1, desc1);
			surf.downloadDescriptors(desc_gpu2, desc2);
			BFMatcher_GPU::matchDownload(trainIdx, distance, matches);

			good_matches = matches;
		}
		else if (speedup == speedup_use_ocl)
		{
			using namespace ocl;
			DevicesInfo devs;
			getOpenCLDevices(devs);

			SURF_OCL surf;
			BFMatcher_OCL matcher;
			oclMat desc1, desc2;
			oclMat im1, im2;
			im1 = imread(im_file1, CV_LOAD_IMAGE_GRAYSCALE);
			im2 = imread(im_file2, CV_LOAD_IMAGE_GRAYSCALE);
			surf(im1, oclMat(), im1_key_points, desc1);
			surf(im2, oclMat(), im2_key_points, desc2);
			matcher.match(desc1, desc2, matches);

			double max_dist = 0;
			double min_dist = 100;
			for (int i = 0; i < desc1.rows; i++)
			{
				double dist = matches[i].distance;
				if (dist < min_dist) min_dist = dist;
				if (dist > max_dist) max_dist = dist;
			}

			for (int i = 0; i < desc1.rows; i++)
			{
				if (matches[i].distance <= max(sigma * min_dist, 0.02))
				{
					good_matches.push_back(matches[i]);
				}
			}
		}
		else
		{
			auto get_file_desc = [](const std::string &filename, double min_hessian, std::vector<cv::KeyPoint> *out_key_points)
			{
				using namespace cv;
				Mat desc;
				Mat img = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
				if (!img.data) return desc;

				SurfFeatureDetector detecter(min_hessian);

				std::vector<KeyPoint> key_points;
				detecter.detect(img, key_points);

				if (out_key_points) *out_key_points = key_points;

				if (key_points.empty())
				{
					std::cout << "no feature detected: " << filename << std::endl;
					return desc;
				}

				SurfDescriptorExtractor extractor;
				extractor.compute(img, key_points, desc);

				return desc;
			};

			auto im1_desc = get_file_desc(im_file1, min_hessian, &im1_key_points);
			auto im2_desc = get_file_desc(im_file2, min_hessian, &im2_key_points);
			
			FlannBasedMatcher matcher;

			matcher.match(im1_desc, im2_desc, matches);

			double max_dist = 0;
			double min_dist = 100;
			for (int i = 0; i < im1_desc.rows; i++)
			{
				double dist = matches[i].distance;
				if (dist < min_dist) min_dist = dist;
				if (dist > max_dist) max_dist = dist;
			}

			for (int i = 0; i < im1_desc.rows; i++)
			{
				if (matches[i].distance <= max(sigma * min_dist, 0.02))
				{
					good_matches.push_back(matches[i]);
				}
			}
		}

		auto conv_keypoint = [](const std::vector<KeyPoint> &kps)
		{
			key_points out;
			std::transform(kps.begin(), kps.end(), std::back_inserter(out), [](const KeyPoint &kp){
				//return key_point{ kp.pt.x, kp.pt.y, kp.size, kp.angle };
				key_point p;
				p.x = kp.pt.x;
				p.y = kp.pt.y;
				p.size = kp.size;
				p.angle = kp.angle;
				return p;
			});
			return out;
		};

		auto kps1 = conv_keypoint(im1_key_points);
		auto kps2 = conv_keypoint(im2_key_points);

		if (im1_kps)
			*im1_kps = kps1;
		if (im2_kps)
			*im2_kps = kps2;

		std::transform(good_matches.begin(), good_matches.end(), std::back_inserter(mt), [&](const DMatch &m)
		{
			//return match{ kps1[m.queryIdx] , kps2[m.trainIdx] , m.distance };
			match ma;
			ma.pt1 = kps1[m.queryIdx];
			ma.pt2 = kps2[m.trainIdx];
			ma.distance = m.distance;
			return ma;
		});

		return mt;
	}

}