#include "ColorTransferTools.h"
/**
@brief ColorTransferTools.cpp
class for ColorTransferTools
@author zhu-ty
@date Apr 25, 2018
*/

#include "ColorTransferTools.h"
// math
#include <math.h>

//eigen
#include <Eigen/Eigen>
#include <glm/gtc/matrix_transform.hpp>

//Eigen : Col first order!
class ColorTransfer
{
public:
	static int cov(const Eigen::MatrixXd &m, Eigen::MatrixXd &dst);
	//Also turns 255 into 1.0f
	static int reshape_CV8UC3(const cv::Mat &src, Eigen::MatrixXd &dst);
	static int reshape_MatrixXd(const Eigen::MatrixXd &src, cv::Mat &dst, int width, int height);
	static int matlab_eig(const Eigen::MatrixXd &src, Eigen::MatrixXd &vec, Eigen::MatrixXd &val);
	static int MKL(const Eigen::MatrixXd &A, const Eigen::MatrixXd &B, Eigen::MatrixXd &T);
};


int ColorTransferInterface::MKL_transfer(cv::Mat & src, const cv::Mat & target, glm::mat4 &color_correct, glm::vec4 &color_append, bool resize_mat)
{
	cv::Mat _src = src.clone();
	cv::Mat _target = target.clone();
	if (resize_mat)
	{
		cv::resize(_src, _src, cv::Size(MKL_SAMPLED_WIDTH, MKL_SAMPLED_HEIGHT));
		cv::resize(_target, _target, cv::Size(MKL_SAMPLED_WIDTH, MKL_SAMPLED_HEIGHT));
	}
	Eigen::MatrixXd X0, X1;
	ColorTransfer::reshape_CV8UC3(_src, X0);
	ColorTransfer::reshape_CV8UC3(_target, X1);

	Eigen::MatrixXd A, B, T;
	ColorTransfer::cov(X0, A);
	ColorTransfer::cov(X1, B);
	ColorTransfer::MKL(A, B, T);
#ifdef MODIFY_SRC
	ColorTransfer::reshape_CV8UC3(src, X0);
	cv::resize(_target, _target, cv::Size(src.cols, src.rows));
	ColorTransfer::reshape_CV8UC3(_target, X1);
	Eigen::MatrixXd mX0(X0.rows(), 3);
	Eigen::MatrixXd mX1(X1.rows(), 3);
	for (int i = 0; i < 3; i++)
	{
		mX0.col(i).setConstant(X0.col(i).mean());
		mX1.col(i).setConstant(X1.col(i).mean());
	}
	Eigen::MatrixXd XR = (X0 - mX0) * T + mX1;
	reshape_MatrixXd(XR, src, src.cols, src.rows);
#endif
	color_correct = glm::mat4();
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			color_correct[j][i] = (float)T(j, i);
	color_append = glm::vec4();
	color_append[3] = 0.0f;
	Eigen::Vector3d X0mean;
	for (int i = 0; i < 3; i++)
		X0mean(i) = X0.col(i).mean();
	for (int i = 0; i < 3; i++)
		color_append[i] = X1.col(i).mean() - X0mean(0) * T(0, i) - X0mean(1) * T(1, i) - X0mean(2) * T(2, i);
	return 0;
}

int ColorTransfer::cov(const Eigen::MatrixXd &m, Eigen::MatrixXd &dst)
{
	Eigen::MatrixXd centered = m.rowwise() - m.colwise().mean();
	dst = (centered.adjoint() * centered) / double(m.rows() - 1);
	//std::cout << dst << std::endl;
	return 0;
}

int ColorTransfer::reshape_CV8UC3(const cv::Mat & src, Eigen::MatrixXd & dst)
{
	cv::Mat _src = src;
	if (src.isContinuous() == false)
		_src = src.clone();
	Eigen::MatrixXd _dst(_src.cols*_src.rows, 3);
	
	for (int i = 0; i < _src.cols*_src.rows; i++)
	{
		_dst(i, 0) = src.at<cv::Vec3b>(i)[0] / 255.0;
		_dst(i, 1) = src.at<cv::Vec3b>(i)[1] / 255.0;
		_dst(i, 2) = src.at<cv::Vec3b>(i)[2] / 255.0;
	}
	dst = _dst;
	//std::cout << dst <<std::endl;
	return 0;
}

int ColorTransfer::reshape_MatrixXd(const Eigen::MatrixXd & src, cv::Mat & dst, int width, int height)
{
	dst.create(height, width, CV_8UC3);
	for (int i = 0; i < height*width; i++)
	{
		dst.at<cv::Vec3b>(i)[0] = cv::saturate_cast<uchar>(round(src(i, 0) * 255));
		dst.at<cv::Vec3b>(i)[1] = cv::saturate_cast<uchar>(round(src(i, 1) * 255));
		dst.at<cv::Vec3b>(i)[2] = cv::saturate_cast<uchar>(round(src(i, 2) * 255));
	}
	return 0;
}

int ColorTransfer::matlab_eig(const Eigen::MatrixXd & src, Eigen::MatrixXd & vec, Eigen::MatrixXd & val)
{
	Eigen::EigenSolver<Eigen::MatrixXd> ES(src);
	vec = ES.eigenvectors().real();
	val = ES.eigenvalues().real();
	for (int i = 0; i < val.rows(); i++)
	{
		int max_val = i;
		for (int j = i; j < val.rows(); j++)
		{
			if (abs(val(j, 0)) > abs(val(max_val, 0)))
				max_val = j;
		}
		val.row(i).swap(val.row(max_val));
		vec.col(i).swap(vec.col(max_val));
	}
	return 0;
}

int ColorTransfer::MKL(const Eigen::MatrixXd & A, const Eigen::MatrixXd & B, Eigen::MatrixXd & T)
{
	int N = 3;
	Eigen::MatrixXd Ua, Da2;
	matlab_eig(A, Ua, Da2);
	//std::cout << Ua << std::endl << std::endl;
	//std::cout << Da2 << std::endl << std::endl;
	Eigen::MatrixXd Da(3, 3);
	Da.setZero();
	for (int i = 0; i < 3; i++)
	{
		if (Da2(i) < 0.0)
			Da2(i) = 0.0;
		Da(i,i) = sqrt(Da2(i) + std::numeric_limits<double>::epsilon());
	}
	//std::cout << Da << std::endl << std::endl;
	Eigen::MatrixXd C = Da * Ua.transpose() * B * Ua * Da;
	Eigen::MatrixXd Uc, Dc2;
	matlab_eig(C, Uc, Dc2);
	Eigen::MatrixXd Dc(3, 3), Da_inv(3,3);
	Dc.setZero();
	Da_inv.setZero();
	for (int i = 0; i < 3; i++)
	{
		if (Dc2(i) < 0.0)
			Dc2(i) = 0.0;
		Dc(i, i) = sqrt(Dc2(i) + std::numeric_limits<double>::epsilon());
		Da_inv(i, i) = 1.0 / Da(i, i);
	}
	T = Ua*Da_inv*Uc*Dc*Uc.transpose()*Da_inv*Ua.transpose();
	return 0;
}
