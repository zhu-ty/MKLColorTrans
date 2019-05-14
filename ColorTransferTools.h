/**
@brief ColorTransferTools.h
Color Transfer Static Class
@author zhu-ty
@date Apr 25, 2018
*/


#ifndef __PANO_RANDER_COLOR_TRANSFER_TOOLS__
#define __PANO_RANDER_COLOR_TRANSFER_TOOLS__

// include stl
#include <memory>
#include <cstdlib>
#include <vector>
// opencv
#include <opencv2/opencv.hpp>

// include GLM
#include <glm/glm.hpp>

//#define MODIFY_SRC
#define MKL_SAMPLED_WIDTH 400
#define MKL_SAMPLED_HEIGHT 300

//Eigen : Col first order!
class ColorTransferInterface
{
public:
	/**
	@brief use MKL method to do the color transfer
	@param cv::Mat &src: 
	Img that is about to change into another "color", define MODIFY_SRC will change src for debug
	To make sure you get the right color_correct, use RGB format instead of BGR in both src and target
	@param const cv::Mat &target: target "color" image
	@param glm::mat4 &color_correct: mat4 color correction matrix for OpenGL
	@param glm::vec4 &color_append: vec4 color append for OpenGL
	@param  bool resize_mat: (defalt:true) resize mat to SAMPLED_WIDTH*SAMPLED_HEIGHT
	@return int(0)
	*/
	static int MKL_transfer(cv::Mat &src, const cv::Mat &target, glm::mat4 &color_correct, glm::vec4 &color_append, bool resize_mat = true);
};

#endif //!__PANO_RANDER_COLOR_TRANSFER_TOOLS__