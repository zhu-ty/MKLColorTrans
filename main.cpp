
#include "SKCommon.hpp"
#include "SKEncoder.h"
#include "ColorTransferTools.h"
#include <npp.h>
#include <nppi.h>

void printUsage()
{
	SKCommon::infoOutput("Usage:");
	SKCommon::infoOutput("(Calculate Mode)./MKLColorTrans C source.jpg(mp4) target.jpg(mp4) (mask.png)");
	SKCommon::infoOutput("(Apply Mode)    ./MKLColorTrans A source.jpg(mp4)");
}

cv::Mat readFirst(std::string name)
{
	cv::Mat ret;
	if (SKCommon::getFileExtention(name) == "mp4" ||
		SKCommon::getFileExtention(name) == "avi")
	{
		cv::VideoCapture v(name);
		v >> ret;
		v.release();
	}
	else
	{
		ret = cv::imread(name);
	}
	return ret;
}

int applyRGB(cv::Mat & color_RGB, glm::mat4 c_mat_RGB, glm::vec4 c_vec_RGB)
{
	cv::cuda::GpuMat g(color_RGB);
	float Twist[3][4] = {//RGB twist
		{ c_mat_RGB[0][0], c_mat_RGB[1][0], c_mat_RGB[2][0], c_vec_RGB[0] * 255.0 },
		{ c_mat_RGB[0][1], c_mat_RGB[1][1], c_mat_RGB[2][1], c_vec_RGB[1] * 255.0 },
		{ c_mat_RGB[0][2], c_mat_RGB[1][2], c_mat_RGB[2][2], c_vec_RGB[2] * 255.0 }
	};
	NppiSize osize;
	osize.width = g.cols;
	osize.height = g.rows;
	nppiColorTwist32f_8u_C3IR(g.data, g.step, osize, Twist);
	g.download(color_RGB);
	return 0;
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printUsage();
		return 0;
	}
	std::string mode = SKCommon::toLower(argv[1]);
	std::sort(mode.begin(), mode.end());
	std::reverse(mode.begin(), mode.end());
	std::string srcName(argv[2]);
	for (int mi = 0; mi < mode.length(); mi++)
	{
		char modei = mode[mi];
		if (modei == 'c')
		{
			cv::Mat msk;
			std::string tarName(argv[3]);
			if (argc > 4)
				msk = cv::imread(argv[4], cv::IMREAD_UNCHANGED);
			cv::Mat src = readFirst(srcName),
				tar = readFirst(tarName);
			glm::mat4 cor;
			glm::vec4 apr;
			cv::cvtColor(src, src, cv::COLOR_BGR2RGB);
			cv::cvtColor(tar, tar, cv::COLOR_BGR2RGB);
			ColorTransferInterface::MKL_transfer(src, tar, cor, apr, msk);
			cv::Mat corC(4, 4, CV_32F, &cor);
			cv::Mat aprC(1, 4, CV_32F, &apr);
			cv::FileStorage fs("MCTParam.xml", cv::FileStorage::WRITE);
			fs << "Correct" << corC;
			fs << "Append" << aprC;
			fs.release();
		}
		else if (modei == 'a')
		{
			if (!SKCommon::existFile("MCTParam.xml"))
			{
				SKCommon::errorOutput("mode = A but MCTParam.xml not found.");
				return -1;
			}
			cv::Mat corC(4, 4, CV_32F);
			cv::Mat aprC(1, 4, CV_32F);
			cv::FileStorage fs("MCTParam.xml", cv::FileStorage::READ);
			fs["Correct"] >> corC;
			fs["Append"] >> aprC;
			fs.release();
			glm::mat4 cor;
			glm::vec4 apr;
			memcpy(&cor, corC.data, sizeof(float) * 4 * 4);
			memcpy(&apr, aprC.data, sizeof(float) * 1 * 4);
			if (SKCommon::getFileExtention(srcName) == "mp4" ||
				SKCommon::getFileExtention(srcName) == "avi")
			{

			}
			else
			{
				cv::Mat src = cv::imread(srcName);
				cv::cvtColor(src, src, cv::COLOR_BGR2RGB);
				applyRGB(src, cor, apr);
				cv::cvtColor(src, src, cv::COLOR_RGB2BGR);
				cv::imwrite(srcName + ".cor." + SKCommon::getFileExtention(srcName), src);
			}
		}
		else
		{
			SKCommon::warningOutput("mode not recognized, mode = %s", mode.c_str());
			printUsage();
		}
	}
	return 0;
}