#pragma once

#include "bmp.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <string>

using namespace std;

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char uchar;
constexpr auto PI = 3.1415926535;
#define MIN(x, y) ( (x)<(y) ? (x) : (y) )
#define MAX(x, y) ( (x)>(y) ? (x) : (y) )

class Operate {
private:
	BmpImage bmpImage;

public:
	bool dithering();
	void getDitheringMatrix(int n, vector<vector<uchar>>& D);
	bool orderedDithering();
	BmpImage rgb2gray(string method);
	void Color2gray();//彩色转灰度
	void LosslessPreCoding();//无损预测
	void DCT();//dct变换
	void LogOrExpo();//对数或指数变换
	double C(int n);//dct 变换要用到的c函数
	void computeF(vector<vector<double>> &F, vector<vector<uchar>> &f, int width, int height);//计算F矩阵
	void computeIF(vector<vector<double>> &IF, vector<vector<uchar>> &If, int width, int height);//计算IF矩阵
	void computeFC(vector<vector<double>> &F, vector<vector<uchar>> &f, int width, int height);//f是原始的小块二维数组
	bool singleThreshold();//单阈值法转化黑度图像
	bool histogramEqualication_gray();//灰度图像直方图均衡
	bool histogramEqualication_color();//彩色图像直方图均衡
	bool quantization();//均匀量化
	void pixelRGBtoHSI(uchar R, uchar G, uchar B, float& H, float& S, float& I);
	void pixelHSItoRGB(float H, float S, float I, uchar& R, uchar& G, uchar& B);

	Operate(string path) {

		bool judge_input = bmpImage.LoadImage(path);
		if (judge_input == true) {
			printf("\n******图片读入成功******\n\n");
		}
		//cout << bmpImage.getChannels();
	}
	bool doOperation() {
		if (bmpImage.getChannels() == 1) {
			int choose = 0;
			cout << "请选择您的操作:\n1.single threshold\n2.dithering\n3.ordered dithering\n4.无损预测编码\n5.DCT或反DCT\n6.指数或对数变换\n7.直方图均衡\n8.均匀量化\n9.exit\n";
			cin >> choose;
			getchar();
			switch (choose) {
			case 1:
				singleThreshold();
				break;
			case 2:
				dithering();
				break;
			case 3:
				orderedDithering();
				break;
			case 4:
				LosslessPreCoding();
				break;
			case 5:
				DCT();
				break;
			case 6:
				LogOrExpo();
				break;
			case 7:
				histogramEqualication_gray();
				break;
			case 8:
				quantization();
				break;
			case 9:
				exit(1);
			default:
				cout << "错误输入!\n";
			}
		}
		else if (bmpImage.getChannels() == 3) {
			int choose = 0;
			cout << "请选择您的操作:\n1.Color to gray\n";
			cin >> choose;
			//getchar();
			switch (choose) {
				case 1:
					//clolor to gray
					Color2gray();
					break;
				default:
					cout << "错误输入!\n";
			}

		}
		else {
			cout << "错误!";
		}

	}
};

bool Operate::dithering() {
	bool transDone = false;
	vector<vector<uchar>> D;//dither matrix
	vector<vector<uchar>> I;//matrix after divided.
	vector<vector<uchar>> O;//output matrix
	int n;
	cout << "请输入抖动矩阵：\n(例:2)\n";
	while (true) {
		cin >> n;
		getchar();
		if (n&(n - 1)) {//如果抖动矩阵不是2的倍数
			cout << "请输入抖动矩阵，必须是2的倍数(例:4)" << endl;
		}
		else {
			cout << "等待中..." << endl;
			break;
		}
	}

	float T = round(256.0f / ((float)n*(float)n + 1.0f));
	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		vector<uchar> tempi;
		for (size_t j = 0; j <= bmpImage.data[i].size() - 1; j++) {
			float temp = 0.0f;
			temp = (float)bmpImage.data[i][j] / T;//ppt中的公式I[x][y]=(int)(I[x][y]*(n*n+1)/256.0)
			tempi.push_back((uchar)round(temp));
		}
		I.push_back(tempi);
	}
	
	getDitheringMatrix(n, D);
	for (size_t x = 0; x < bmpImage.getHeight()*n; x++) {//初始化输出矩阵，首先全部置为1
		vector<uchar> temp;
		for (size_t y = 0; y < bmpImage.getHeight()*n; y++) {
			temp.push_back(0xff);
		}
		O.push_back(temp);
	}
	//以下为实现ppt中的伪代码
	for (size_t x = 0; x < bmpImage.getHeight(); x++) {
		for (size_t y = 0; y < bmpImage.getWidth(); y++) {
			int i = x * n;
			int j = y * n;
			for (size_t ii = 0; ii < n; ii++) {
				for (size_t jj = 0; jj < n; jj++) {
					//cout << "height = "<< ii + i << endl;
					//cout << "width = " << jj + j << endl;
					if (D[ii][jj] < I[x][y]) {
						O[i + ii][j + jj] = 0xff;
					}
					else {
						O[i + ii][j + jj] = 0x00;
					}
				}
			}
		}
	}
	//以下计算出处理后图像的分辨率及文件头信息更新
	bmpImage.setHeight(bmpImage.getHeight() * n);
	bmpImage.setWidth(bmpImage.getWidth() * n);
	bmpImage.bmpFileHeader.bfSize = bmpImage.bmpFileHeader.bfOffBits + bmpImage.getWidth() * bmpImage.getHeight();
	bmpImage.data.clear();
	for (size_t i = 0; i < bmpImage.getHeight(); i++) {
		vector<uchar> tempData;
		for (size_t j = 0; j < bmpImage.getWidth(); j++) {
			tempData.push_back(O[i][j]);
		}
		bmpImage.data.push_back(tempData);
	}
	bmpImage.SaveImage();
	transDone = true;
	return transDone;
}

bool Operate::orderedDithering() {
	bool transDone = false;
	vector<vector<uchar>> D;//抖动矩阵
	vector<vector<uchar>> I;//矩阵被分开后

	int n;
	cout << "请输入抖动矩阵：\n(例:2)\n";
	while (true) {
		cin >> n;
		getchar();
		if (n&(n - 1)) {//if n is not the index times of 2
			cout << "请输入抖动矩阵，必须是2的倍数(例:4)" << endl;
		}
		else {
			cout << "等待中..." << endl;
			break;
		}
	}

	float T = round(256.0f / ((float)n*(float)n + 1.0f));
	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		vector<uchar> tempi;
		for (size_t j = 0; j <= bmpImage.data[i].size() - 1; j++) {
			float temp = 0.0f;
			temp = (float)bmpImage.data[i][j] / T;
			tempi.push_back((uchar)round(temp));
		}
		I.push_back(tempi);
	}
	//cout << "end first" << endl;
	getDitheringMatrix(n, D);
	for (size_t x = 0; x < bmpImage.getHeight(); x++) {
		int i = x % n;
		for (size_t y = 0; y < bmpImage.getWidth(); y++) {
			int j = y % n;
			if (I[x][y] > D[i][j]) {
				bmpImage.data[x][y] = 0xff;
			}
			else {
				bmpImage.data[x][y] = 0x00;
			}
		}
	}
	bmpImage.SaveImage();
	transDone = true;
	return transDone;
}


void Operate::getDitheringMatrix(int n, vector<vector<uchar>>& D) {
	D = { {(uchar)0,(uchar)2},{(uchar)3,(uchar)1} };
	//首先给出D[2]，之后根据用户给出的n递归计算D[n]
	int length = 2;
	vector<vector<uchar>> tempD;
	while (length < n) {
		for (size_t x = 0; x < length; x++) {
			vector<uchar> temp;
			for (size_t y = 0; y < length; y++) {
				temp.push_back(D[x][y]);
			}
			tempD.push_back(temp);
		}
		D.clear();
		int x = 0;
		int y = 0;
		for (x = 0; x < length; x++) {
			vector<uchar> temp;
			for (y = 0; y < length; y++) {
				temp.push_back((uchar)(4 * (int)tempD[x][y]));
			}
			for (y = length; y < 2 * length; y++) {
				temp.push_back((uchar)(4 * (int)tempD[x][y - length] + 2));
			}
			D.push_back(temp);
		}
		for (x = length; x < 2 * length; x++) {
			vector<uchar> temp;
			for (y = 0; y < length; y++) {
				temp.push_back((uchar)(4 * (int)tempD[x - length][y] + 3));
			}
			for (y = length; y < 2 * length; y++) {
				temp.push_back((uchar)(4 * (int)tempD[x - length][y - length] + 1));
			}
			D.push_back(temp);
		}
		tempD.clear();

		length = length << 1;
	}

}

//rgb转灰度图
BmpImage Operate::rgb2gray(string method){
	BmpImage ori_image = bmpImage;
	BmpImage out_image = bmpImage;
	size_t step = 3;
	const uchar RGBMAX = 255;
	float r, g, b, o;//o代表输出的像素值
	vector<vector<uchar>> data;
	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		vector<uchar> dataValues;
		for (size_t j = 0; j <= bmpImage.data[i].size() - 3; j += step) {
			r = float(ori_image.data[i][j]);
			g = float(ori_image.data[i][j + 1]);
			b = float(ori_image.data[i][j + 2]);
			if (method == "1") {//使用rgb计算I的值
				r = r / RGBMAX;
				g = g / RGBMAX;
				b = b / RGBMAX;
				o = round((r + g + b) / 3 * RGBMAX);
			}
			else {//使用rgb计算Y的值
				o = 0.299 * r + 0.587 * g + 0.114 * b;
			}
			dataValues.push_back(uchar(o));
		}
		//补齐行的字节数，保证为4的整数倍
		int offset = dataValues.size()%4;
		if (offset != 0) {
			dataValues.push_back(uchar(0));
		}
		data.push_back(dataValues);
	}

	//将新生成的data放入outimage中
	out_image.setWidth(data[1].size());
	out_image.setData(data);

	//设置channel数为1，bitcount数为8, biWidth 为新width
	out_image.setBitCount(ushort(8));
	out_image.setChannels(1);
	
	//由于读24位图时没读调色板，现在还需要重新编写调色板内容，256色以0-255一一对应(仅针对8位256色)
	for (int i = 0; i < 256; i++) {
		Palette p = { i,i,i,0 };
		out_image.setPalette(p);
	}

	//头偏移和文件大小更改
	out_image.bmpFileHeader.bfOffBits = 1078;
	out_image.bmpFileHeader.bfSize = out_image.bmpFileHeader.bfOffBits + out_image.getWidth() * out_image.getHeight() ;
	return out_image;
}

//彩色转灰度图
void Operate::Color2gray() {
	string choice;
	//循环读取变换方式，用string读入防止用户胡乱输入
	do {
		cout << "请选择变换方式:\n1. RGB->HSI\n2. RGB->YCbCr\n";
		cin >> choice;
	} while (choice != "1" && choice != "2");
	getchar();
	BmpImage grayImage = rgb2gray(choice);
	grayImage.SaveImage();
}

//无损预测编码
void Operate::LosslessPreCoding() {
	int order;
	//循环读取阶次
	do {
		cout << "请选择预测器阶次:\n1. 1阶\n2. 2阶\n3. 3阶\n";
		cin >> order;
	} while (!(order == 1 || order == 2 || order == 3));
	getchar();
	vector<double> e;
	double coef;
	for (int i = 0; i < order; i++) {
		cout << "输入f" << i + 1 << "系数" << endl;
		cin >> coef;
		e.push_back(coef);
	}
	cin.ignore();//清空输入缓冲区
	//复制原data数据，防止预测过程中覆盖
	vector<vector<uchar>> ori_data;
	vector<vector<double>> out_data;
	for (int i = 0; i < bmpImage.getHeight(); i++) {
		vector<uchar> dataV;
		vector<double> dataVI;
		for (int j = 0; j < bmpImage.getWidth(); j++) {
			dataV.push_back(bmpImage.data[i][j]);
			dataVI.push_back((double)bmpImage.data[i][j]);
		}
		ori_data.push_back(dataV);
		out_data.push_back(dataVI);
	}

	if (order == 1) {//1阶预测
		for (int i = 0; i < bmpImage.getHeight(); i++) {
			for (int j = 1; j < bmpImage.getWidth(); j++) {
				double p = round(1.0 * e[0] * ori_data[i][j - 1]);
				//int pi = (int)round((round(p) - (-255)) * 255 * 1.0 / 510);
				//bmpImage.data[i][j] = (uchar)((int)bmpImage.data[i][j]-(int)p);
				out_data[i][j] = 1.0 * bmpImage.data[i][j] - p;
			}
		}
	}
	else if (order == 2) {//2阶预测
		for (int i = 1; i < bmpImage.getHeight(); i++) {
			for (int j = 1; j < bmpImage.getWidth(); j++) {
				double p = round(1.0 * e[0] * ori_data[i][j - 1] + 1.0 * e[1] * ori_data[i-1][j]);
				//int pi = (int)round((round(p) - (-255)) * 255 * 1.0 / 510);
				//bmpImage.data[i][j] = (uchar)((int)bmpImage.data[i][j] - (int)p);
				out_data[i][j] = 1.0 * bmpImage.data[i][j] - p;
			}
		}
	}
	else {//3阶预测
		for (int i = 1; i < bmpImage.getHeight(); i++) {
			for (int j = 1; j < bmpImage.getWidth(); j++) {
				double p = round(1.0 * e[0] * ori_data[i][j - 1] + 1.0 * e[1] * ori_data[i - 1][j] + 1.0 * e[2] * ori_data[i - 1][j - 1]);
				//int pi = (int)round((round(p) - (-255)) * 255 * 1.0 / 510);
				//bmpImage.data[i][j] = (uchar)((int)bmpImage.data[i][j] - (int)p);
				out_data[i][j] = 1.0 * bmpImage.data[i][j] - p;
				//printf("out_data[i][j]:%lf", out_data[i][j]);
			}
		}
	}

	//求全局最大值最小值
	double max = (double)INT_MIN, min = (double)INT_MAX;
	for (int i = 0; i < out_data.size(); i++) {
		for (int j = 0; j < out_data[0].size(); j++) {
			if (out_data[i][j] > max) max = out_data[i][j];
			if (out_data[i][j] < min) min = out_data[i][j];
		}
	}

	//做全局线性变换将值变为0-255
	for (int i = 0; i < out_data.size(); i++) {
		for (int j = 0; j < out_data[0].size(); j++) {
			double trans = round((out_data[i][j]*1.0 - min) * 255.0 / (max - min));
			out_data[i][j] = trans;
		}
	}

	//更新原数据
	vector<vector<uchar>> update_data;
	for (size_t i = 0; i <= out_data.size() - 1; i++) {
		vector<uchar> dataValues;
		for (size_t j = 0; j <= out_data[i].size() - 1; j++) {
			dataValues.push_back(uchar(out_data[i][j]));
		}
		//补齐行的字节数，保证为4的整数倍
		int offset = dataValues.size() % 4;
		while (offset != 0) {
			dataValues.push_back(uchar(0));
			offset--;
		}
		update_data.push_back(dataValues);
	}

	//将新生成的data放入outimage中
	bmpImage.setWidth(update_data[0].size());
	bmpImage.setData(update_data);

	//头偏移和文件大小更改
	bmpImage.bmpFileHeader.bfOffBits = 1078;
	bmpImage.bmpFileHeader.bfSize = bmpImage.bmpFileHeader.bfOffBits + bmpImage.getWidth() * bmpImage.getHeight();

	//保存结果
	bmpImage.SaveImage();
}

//反DCT变换
void Operate::DCT() {
	int width,height;
	cout << "请输入分块大小：" << endl;
	cout << "width：" << endl;
	cin >> width;
	cout << "height：" << endl;
	cin >> height;
	cin.ignore();
	//保存整个data变换后的值，为double，因为变换有负有正，而要存成灰度图，需要将值全局变换为0-255，就要求全局最大值和最小值
	vector<vector<double>> Fdata(bmpImage.getHeight(), vector<double>(bmpImage.getWidth()));//Fdata指的是DCT变换后的全局图片数据
	for (int i = 0; i < bmpImage.getHeight(); i+=height) {
		for (int j = 0; j < bmpImage.getWidth(); j+=width) {
			vector<vector<double>> F(height, vector<double>(width));//F中保存的是经过DCT变换后的小块二维数组
			vector<vector<uchar>> f(height, vector<uchar>(width));//f中保存的是小块原始二维数组
			for (int m = 0; m < height; m++) {
				for (int n = 0; n < width; n++) {
					if ((i + m) >= bmpImage.getHeight() || (j + n) >= bmpImage.getWidth()) {//选取块的部分在data区域外
						f[m][n] = 0x00;
					}
					else {
						f[m][n] = bmpImage.data[i+m][j+n];
					}
				}
			}
			//计算新F区域的值
			computeF(F, f, width, height);
			//将Fdata中F区域的值更新
			for (int m = 0; m < height; m++) {
				for (int n = 0; n < width; n++) {
					if (i + m >= bmpImage.getHeight() || j + n >= bmpImage.getWidth()) {//选取块的部分在data区域外
						continue;
					}
					Fdata[i + m][j + n] = F[m][n];//最终DCT变换后数据形成
				}
			}
		}
	}
	//求全局最大值最小值
	double max = 0, min = 100000;
	for (int i = 0; i < Fdata.size(); i++) {
		for (int j = 0; j < Fdata[0].size(); j++) {
			if (Fdata[i][j] > max) max = Fdata[i][j];
			if (Fdata[i][j] < min) min = Fdata[i][j];
		}
	}
	//做全局线性变换将值变为0-255
	for (int i = 0; i < Fdata.size(); i++) {
		for (int j = 0; j < Fdata[0].size(); j++) {
			int trans = (int)round((Fdata[i][j] - min) * 255 / (max - min));
			Fdata[i][j] = (uchar)trans;
		}
	}
	//此时Fdta内存储了DCT后的数据
	cout << "请问你需要做什么?( 1: 输出DCT变换图  2:输出反DCT变换图  3：输出反DCT变换图(扔掉百分之五十数据) )" << endl;
	int num;
	cin >> num;
	cin.ignore();
	if(num == 1){
		for (int i = 0; i < Fdata.size(); i++) {
			for (int j = 0; j < Fdata[0].size(); j++) {
				bmpImage.data[i][j] = Fdata[i][j];
			}
		}
		bmpImage.SaveImage();
	}
	else if (num == 2){
		vector<vector<double>> IDCTdata(bmpImage.getHeight(), vector<double>(bmpImage.getWidth()));//IDCTdata指的是反DCT变换后的全局图片数据
		for (int i = 0; i < bmpImage.getHeight(); i+=height) {
			for (int j = 0; j < bmpImage.getWidth(); j+=width) {
				vector<vector<double>> IF(height, vector<double>(width));//IF中保存的是经过反DCT变换后的小块二维数组
				vector<vector<uchar>> If(height, vector<uchar>(width));//If中保存的是小块原始DCT后二维数组
				for (int m = 0; m < height; m++) {
					for (int n = 0; n < width; n++) {
						if ((i + m) >= bmpImage.getHeight() || (j + n) >= bmpImage.getWidth()) {//选取块的部分在data区域外
							If[m][n] = 0x00;
						}
						else {
							If[m][n] = Fdata[i+m][j+n];
						
						}
					}
				}
				//计算新IF区域的值
				computeIF(IF, If, width, height);
				//将Fdata中F区域的值更新
				for (int m = 0; m < height; m++) {
					for (int n = 0; n < width; n++) {
						if (i + m >= bmpImage.getHeight() || j + n >= bmpImage.getWidth()) {//选取块的部分在data区域外
							continue;
						}
						IDCTdata[i + m][j + n] = IF[m][n];
					}
				}
			}
		}
		//求全局最大值最小值
		double max = 0, min = 100000;
		for (int i = 0; i < IDCTdata.size(); i++) {
			for (int j = 0; j < IDCTdata[0].size(); j++) {
				if (IDCTdata[i][j] > max) max = IDCTdata[i][j];
				if (IDCTdata[i][j] < min) min = IDCTdata[i][j];
			}
		}
		//做全局线性变换将值变为0-255
		for (int i = 0; i < IDCTdata.size(); i++) {
			for (int j = 0; j < IDCTdata[0].size(); j++) {
				int trans = (int)round((IDCTdata[i][j] - min) * 255 / (max - min));
				IDCTdata[i][j] = (uchar)trans;
			}
		}
		//此时IDCTdata存储了反DCT后的数据,Fdata存储了DCT后的数据,bmpImage存储原始数据
		//均方误差求两个图差异
		double MSE;
		double result = 0.0;
		for (int i = 0; i < IDCTdata.size(); i++) {
			for (int j = 0; j < IDCTdata[0].size(); j++) {
				result += (bmpImage.data[i][j] - IDCTdata[i][j]) * (bmpImage.data[i][j] - IDCTdata[i][j]);
			}
		}
		MSE =  sqrt(result / ( bmpImage.getHeight() *  bmpImage.getWidth() ));
		cout << "该两个图的均方差为" << MSE << endl;
		for (int i = 0; i < IDCTdata.size(); i++) {
			for (int j = 0; j < IDCTdata[0].size(); j++) {
				bmpImage.data[i][j] = IDCTdata[i][j];
			}
		}
		bmpImage.SaveImage();
	}
	else if(num == 3){
		vector<vector<double>> Fdata(bmpImage.getHeight(), vector<double>(bmpImage.getWidth()));
		for (int i = 0; i < bmpImage.getHeight(); i+=height) {
			for (int j = 0; j < bmpImage.getWidth(); j+=width) {
				vector<vector<double>> F(height, vector<double>(width));//F中保存的是经过DCT变换后的小块二维数组
				vector<vector<uchar>> f(height, vector<uchar>(width));//f中保存的是小块原始二维数组
				for (int m = 0; m < height; m++) {
					for (int n = 0; n < width; n++) {
						if ((i + m) >= bmpImage.getHeight() || (j + n) >= bmpImage.getWidth()) {//选取块的部分在data区域外
							f[m][n] = 0x00;
						}
						else {
							f[m][n] = bmpImage.data[i+m][j+n];
						}
					}
				}
				computeFC(F, f, width, height);
				for (int m = 0; m < height; m++) {
					for (int n = 0; n < width; n++) {
						if (i + m >= bmpImage.getHeight() || j + n >= bmpImage.getWidth()) {//选取块的部分在data区域外
							continue;
						}
						Fdata[i + m][j + n] = F[m][n];//最终DCT变换后数据形成
					}
				}
			}
		}
		//求全局最大值最小值
		double max = 0, min = 100000;
		for (int i = 0; i < Fdata.size(); i++) {
			for (int j = 0; j < Fdata[0].size(); j++) {
				if (Fdata[i][j] > max) max = Fdata[i][j];
				if (Fdata[i][j] < min) min = Fdata[i][j];
			}
		}

		//做全局线性变换将值变为0-255
		for (int i = 0; i < Fdata.size(); i++) {
			for (int j = 0; j < Fdata[0].size(); j++) {
				int trans = (int)round((Fdata[i][j] - min) * 255 / (max - min));
				Fdata[i][j] = (uchar)trans;
			}
		}

		{//此处的目标是借用反DCT块重新处理经过50%数据删除的图片，所以不添加任何注释以避免误解
			vector<vector<double>> IDCTdata(bmpImage.getHeight(), vector<double>(bmpImage.getWidth()));
			for (int i = 0; i < bmpImage.getHeight(); i+=height) {
				for (int j = 0; j < bmpImage.getWidth(); j+=width) {
					vector<vector<double>> IF(height, vector<double>(width));
					vector<vector<uchar>> If(height, vector<uchar>(width));
					for (int m = 0; m < height; m++) {
						for (int n = 0; n < width; n++) {
							if ((i + m) >= bmpImage.getHeight() || (j + n) >= bmpImage.getWidth()) {
								If[m][n] = 0x00;
							}
							else {
								If[m][n] = Fdata[i+m][j+n];
							}
						}
					}
					computeIF(IF, If, width, height);
					for (int m = 0; m < height; m++) {
						for (int n = 0; n < width; n++) {
							if (i + m >= bmpImage.getHeight() || j + n >= bmpImage.getWidth()) {//选取块的部分在data区域外
								continue;
							}
							IDCTdata[i + m][j + n] = IF[m][n];
						}
					}
				}
			}
			double max = 0, min = 100000;
			for (int i = 0; i < IDCTdata.size(); i++) {
				for (int j = 0; j < IDCTdata[0].size(); j++) {
					if (IDCTdata[i][j] > max) max = IDCTdata[i][j];
					if (IDCTdata[i][j] < min) min = IDCTdata[i][j];
				}
			}
			for (int i = 0; i < IDCTdata.size(); i++) {
				for (int j = 0; j < IDCTdata[0].size(); j++) {
					int trans = (int)round((IDCTdata[i][j] - min) * 255 / (max - min));
					IDCTdata[i][j] = (uchar)trans;
				}
			}
			for (int i = 0; i < IDCTdata.size(); i++) {
				for (int j = 0; j < IDCTdata[0].size(); j++) {
					bmpImage.data[i][j] = IDCTdata[i][j];
				}
			}
			bmpImage.SaveImage();
		}
	}
}

//C()函数
double Operate::C(int n) {
	return n == 0 ? 1.0 : sqrt(2);
}



//计算F矩阵，F矩阵存储的是即将DCT处理后的小型块数据 ， DCT变换
void Operate::computeF(vector<vector<double>> &F, vector<vector<uchar>> &f, int width, int height) {//f是原始的小块二维数组
	vector<vector<double>> coe(height, vector<double>(width));
	for (int u = 0; u < height; u++) {
		for (int v = 0; v < width; v++) {
			double result=0.0;
			double e = C(u) * C(v) / sqrt(width*height);
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					
					coe[i][j] = cos(((2.0 * i + 1) * u * PI) / (2.0 * height)) * cos(((2.0 * j + 1) * v * PI) / (2.0 * width));
				}
			}
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					result += coe[i][j] * (double)f[i][j];
				}
			}
			F[u][v] = e * result;
		}
	}
}


//计算FC矩阵，F矩阵存储的是即将DCT处理后的小型块数据 ， DCT变换 FC处理经过50%缩减的数据DCT变换
void Operate::computeFC(vector<vector<double>> &F, vector<vector<uchar>> &f, int width, int height) {//f是原始的小块二维数组
	vector<vector<double>> coe(height, vector<double>(width));
	for (int u = 0; u < height; u++) {
		for (int v = 0; v < width; v++) {
			double result=0.0;
			double e = C(u) * C(v) / sqrt(width*height);
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					//if(double(j) <=  (2 * (double)width - (double)width * (double)i / (double)height))//等于取低频，放弃高频，共放弃了50%的频率
					if( (double)j  + (double)i <   ((double)width -1  + (double)height - 1 )/1.3  )
						coe[i][j] = cos(((2.0 * i + 1) * u * PI) / (2.0 * height)) * cos(((2.0 * j + 1) * v * PI) / (2.0 * width));
					else
						coe[i][j] = 1;
						//coe[i][j] = 0;
				}
			}
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					result += coe[i][j] * (double)f[i][j];
				}
			}
			F[u][v] = e * result;
		}
	}
}






//计算IF矩阵，IF矩阵内存储了即将反DCT处理后的存储小型块数据
void Operate::computeIF(vector<vector<double>> &IF, vector<vector<uchar>> &If, int width, int height) {//If是原始的DCT小块二维数组，将要反DCT操作
	vector<vector<double>> coe(height, vector<double>(width));
	for (int u = 0; u < height; u++) {
		for (int v = 0; v < width; v++) {
			double result=0.0;
			double e = 1  / sqrt(width*height);//指的是PPT中的1/根号（M*N）
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					coe[i][j] = cos( ( (2.0 * u + 1) * i * PI )  /  (2.0 * height)  ) * cos( ( (2.0 * v + 1) * j * PI ) / (2.0 * width) );
				}
			}
			for (int i = 0; i < height; i++) {
				for (int j = 0; j < width; j++) {
					result += coe[i][j] * (double)If[i][j] * C(i) * C(j);
				}
			}
			IF[u][v] = e * result;
		}
	}
}


void Operate::LogOrExpo(){//目前仅能处理正方形4倍整数分辨率图片
	string choice;
	//循环读取变换方式，用string读入防止用户胡乱输入
	do {
		cout << "请选择变换方式:\n1. 指数变换\n2. 对数变换\n";
		cin >> choice;
	} while (choice != "1" && choice != "2");
	getchar();
	if(choice == "1"){
		cout << "请依次输入指数变换参数值(变换公式为y= c * pow( x , r )  , 顺序为c ， r) \n";//取 1 0.6
		double c;
		cin >> c;
		cin.ignore();
		double r;
		cin >> r;
		cin.ignore();
		// 映射表,用于256种灰度变换后的值 
   		int map[256]; 
		// 保存运算后的临时值 
    	double dTemp;  
   		for (int i = 1; i < 256; i++) { //像素为0的店不予考虑
        	// 计算当前像素变换后的值 
        	dTemp = c * pow(i / 255.0, r) * 255;
       		// 如果超界则修改其值 
        	if (dTemp < 0) 
            	dTemp = 0.0; 
        	else if (dTemp > 255) 
            	dTemp = 255; 
        	// 四舍五入 
        	map[i] = int(dTemp + 0.5); 
			//cout << map[i] << endl;
    	} 
		
		
		for(int i=0 ; i < bmpImage.getHeight() ; i++ ){//373
			for(int j=0 ; j<bmpImage.getWidth(); j++){//488
				bmpImage.data[i][j] = uchar(map[ bmpImage.data[i][j] ]  );
			}
		}

	}
	else if(choice == "2"){
		cout << "请依次输入对数变换参数值(变换公式为y= c * log(x+1) ，输入参数c) \n";//取参数为20时效果最好
		double c;
		cin >> c;
		cin.ignore();
		// 映射表,用于256种灰度变换后的值 
   		int map[256]; 
		// 保存运算后的临时值 
    	double dTemp;  
   		for (int i = 0; i < 256; i++) { //像素为0的店不予考虑
        	// 计算当前像素变换后的值 
			dTemp = c * log( (double)i + 1.0 ) ;
       		// 如果超界则修改其值 
        	if (dTemp < 0) 
            	dTemp = 0.0; 
        	else if (dTemp > 255) 
            	dTemp = 255; 
        	// 四舍五入 
        	map[i] = int(dTemp + 0.5); 
			//cout << map[i] << endl;
    	} 
		
		
		for(int i=0 ; i < bmpImage.getHeight() ; i++ ){//373
			for(int j=0 ; j<bmpImage.getWidth(); j++){//488
				//cout << int(bmpImage.data[i][j]) << endl;
				
				
				bmpImage.data[i][j] = uchar(map[ bmpImage.data[i][j] ]  );
				//cout << "*" << int(bmpImage.data[i][j]) << "*" << endl;
			}
		}
	}
	bmpImage.SaveImage();
}

//阈值处理：将灰度图像转化为黑白图像
bool Operate::singleThreshold() {

	bool TransDone = false;
	int threshold = 0;
	cout << "请输入合适的阈值(0--255):\n";
	while (true) {
		cin >> threshold;
		getchar();
		if (threshold > 255 || threshold < 0) {
			cout << "请输入正确的阈值!\n再次输入:\n";
		}
		else {
			for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
				for (size_t j = 0; j <= bmpImage.data[i].size() - 1; j++) {
					//遍历图像数组
					if (bmpImage.data[i][j] > threshold) {
						//大于阈值转化为黑色像素，小于阈值转化为白色像素
						bmpImage.data[i][j] = 0xff;
					}
					else {
						bmpImage.data[i][j] = 0x00;
					}
				}
			}
			bmpImage.SaveImage();
			//存储成新bmp图片
			TransDone = true;
			break;
		}
	}
	return TransDone;
}

//灰度图像直方图均衡化
bool Operate::histogramEqualication_gray() {
	bool TransDone = true;
	int n = bmpImage.getHeight()*bmpImage.getWidth();//像素总数
	int hist[256] = { 0 };//记录每个像素的个数
	double hist_prob[256] = { 0 };//记录灰度分布密度
	double hist_distribution[256] = { 0 };//记录累计密度
	int hist_equal[256] = { 0 };//均衡化后的灰度值



	//计算每个灰度的像素个数
	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		for (size_t j = 0; j <= bmpImage.data[i].size() - 1; j++) {
			int value = bmpImage.data[i][j];
			hist[value]++;
		}
	}

	//统计灰度频率
	for (int i = 0; i < 256; i++)
	{
		hist_prob[i] = ((double)hist[i] / n);
	}

	//计算累计密度
	hist_distribution[0] = hist_prob[0];
	for (int i = 1; i < 256; i++)
	{
		hist_distribution[i] = hist_distribution[i - 1] + hist_prob[i];
	}

	//重新计算均衡后灰度值
	for (int i = 0; i < 256; i++)
	{
		hist_equal[i] = (uchar)(255 * hist_distribution[i] + 0.5);
	}

	//均衡化到每个像素
	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		for (size_t j = 0; j <= bmpImage.data[i].size() - 1; j++) {
			bmpImage.data[i][j] = hist_equal[bmpImage.data[i][j]];

		}
	}

	bmpImage.SaveImage();
	return TransDone;
}


//彩色图像直方图均衡化

bool Operate::histogramEqualication_color() {
	bool TransDone = true;
	size_t step = 3;

	vector<vector<float> > arr(bmpImage.data.size());
	for (int i = 0; i < bmpImage.data.size(); i++) {

		arr[i].resize(bmpImage.data[i].size());
	}
	for (int i = 0; i < bmpImage.data.size() - 1; i++) {
		for (int j = 0; j < bmpImage.data[i].size() - 1; j++) {
			arr[i][j] = (float)0;
		}
	}


	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		//cout << i << endl;
		for (size_t j = 0; j <= bmpImage.data[i].size() - 3; j += step) {
			//	cout << j << endl;
			pixelRGBtoHSI(bmpImage.data[i][j], bmpImage.data[i][j + 1], bmpImage.data[i][j + 2], arr[i][j], arr[i][j + 1], arr[i][j + 2]);
		}
	}
	int n = (bmpImage.getHeight()*bmpImage.getWidth());//像素总数
	int hist[256] = { 0 };//记录每个像素的个数
	double hist_prob[256] = { 0 };//记录灰度分布密度
	double hist_distribution[256] = { 0 };//记录累计密度
	int hist_equal[256] = { 0 };//均衡化后的灰度值
	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		//cout << i << endl;
		for (size_t j = 0; j <= bmpImage.data[i].size() - 1; j += step) {
			//	cout << j << endl;
			int value = bmpImage.data[i][j + 2];
			hist[value]++;
		}
	}

	for (int i = 0; i < 256; i++)
	{
		hist_prob[i] = ((double)hist[i] / n);
	}

	hist_distribution[0] = hist_prob[0];
	for (int i = 1; i < 256; i++)
	{
		hist_distribution[i] = hist_distribution[i - 1] + hist_prob[i];
	}

	for (int i = 0; i < 256; i++)
	{
		hist_equal[i] = (uchar)(255 * hist_distribution[i] + 0.5);
	}

	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		//cout << i << endl;
		for (size_t j = 0; j <= bmpImage.data[i].size() - 3; j += step) {
			//	cout << j << endl;
			bmpImage.data[i][j + 2] = hist_equal[bmpImage.data[i][j + 2]];
		}
	}




	for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
		//cout << i << endl;
		for (size_t j = 0; j <= bmpImage.data[i].size() - 3; j += step) {
			//	cout << j << endl;
			pixelHSItoRGB(arr[i][j], arr[i][j + 1], arr[i][j + 2], bmpImage.data[i][j], bmpImage.data[i][j + 1], bmpImage.data[i][j + 2]);
		}
	}

	bmpImage.SaveImage();
	return TransDone;
}


void Operate::pixelRGBtoHSI(uchar R, uchar G, uchar B, float& H, float& S, float& I) {
	const uchar RGBMAX = 255;
	float r = float(R) / RGBMAX;
	float g = float(G) / RGBMAX;
	float b = float(B) / RGBMAX;
	double min, max, th;

	float   i = float(I);
	float   s = float(S);
	float   h = float(H);
	if (r > g && r > b)
	{
		max = r;
		min = MIN(g, b);
	}
	else
	{
		if (g > b)
		{
			max = g;
			min = MIN(r, b);
		}
		else
		{
			max = b;
			min = MIN(r, g);
		}
	}
	i = (r + g + b) / 3.0;
	if (i == 0 || max == min)
	{
		// this is a black image or grayscale image
		s = 0;
		h = 0;
	}
	else
	{
		s = 1 - min / i;

		th = (r - g)*(r - g) + (r - b)*(g - b);
		th = sqrt(th);
		th = acos((r - g + r - g - b)*0.5 / th);

		if (g >= b)
			h = th;
		else
			h = 2 * PI - th;
	}

	H = h;
	S = s;
	I = i;
}



void Operate::pixelHSItoRGB(float H, float S, float I, uchar& R, uchar& G, uchar& B) {
	const uchar RGBMAX = 255;
	float   i = I;
	float   s = S;
	float   h = H;

	float r = float(R) / RGBMAX;
	float g = float(G) / RGBMAX;
	float b = float(B) / RGBMAX;
	if (i == 0.0) {
		// black image
		r = g = b = 0;
	}
	else {
		if (s == 0.0) {
			// grayscale image
			r = g = b = i;
		}
		double temp1, temp2, temp3;
		temp1 = (1 - s) / 3;
		temp2 = s * cos(h);

		if (h >= 0 && h < (PI * 2 / 3))
		{
			b = temp1;
			temp3 = cos(PI / 3 - h);
			r = (1 + temp2 / temp3) / 3;
			g = 1 - r - b;
			r = 3 * i*r;
			g = 3 * g*i;
			b = 3 * i*b;

		}
		else if (h >= (PI * 2 / 3) && h < (PI * 4 / 3))
		{
			r = temp1;
			temp3 = cos(PI - h);
			g = (1 + temp2 / temp3) / 3;
			b = 1 - r - r;
			r = 3 * i*r;
			g = 3 * g*i;
			b = 3 * i*b;
		}
		else if (h >= (PI * 4 / 3) && h < (PI * 2))
		{
			g = temp1;
			temp3 = cos(PI * 5 / 3 - h);
			b = (1 + temp2 / temp3) / 3;
			r = 1 - r - b;
			r = 3 * i*r;
			g = 3 * g*i;
			b = 3 * i*b;
		}
	}
	R = (uchar)round(r*RGBMAX);    //range: 0 to 255;
	G = (uchar)round(g*RGBMAX);     //range: 0 to 255;
	B = (uchar)round(b*RGBMAX);     //range:  0 to 255;

}
//图像压缩之均匀量化
bool Operate::quantization() {

	bool TransDone = false;
	double compression_ratio = 0;
	int FloatToIntwidth, FloatToIntheight;
	int quantization[600] = { 0 };
	int a = 0;

	cout << "请输入压缩比:\n";
	while (true) {
		cin >> compression_ratio;
		getchar();
		if (compression_ratio < 0 || compression_ratio == 0) {
			cout << "请输入正确的压缩比!\n再次输入:\n";
		}
		else {
			compression_ratio = (int)compression_ratio / 1;
			while (a < 256) {
				for (int i = a; i < a + compression_ratio; i++)
				{
					quantization[i] = a;
				}
				a = a + compression_ratio;
			}
			for (size_t i = 0; i <= bmpImage.data.size() - 1; i++) {
				for (size_t j = 0; j <= bmpImage.data[i].size() - 1; j++) {
					bmpImage.data[i][j] = quantization[bmpImage.data[i][j]];
				}


			}
			bmpImage.SaveImage();
			//存储成新bmp图片
			TransDone = true;
			break;
		}

	}
	return TransDone;
}










