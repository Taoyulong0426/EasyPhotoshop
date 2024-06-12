#include "bmp.h"
#include "operate.h"
#include <string>
using namespace std;

int main() {
	int judge;
	string input_path;
	do
	{
		cout << "******请输入输入图片路径，例(""C:\\..."")******\n" << endl;
		getline(std::cin, input_path);
		//ModelTrans modelTrans = ModelTrans(R"(E:\SimplePhotoshop2\test\lenna.bmp)");
		Operate operation = Operate(input_path.c_str());
		operation.doOperation();
		cout << "\n******继续使用输入1,退出输入0******\n" << endl;
		cin >> judge;
		getchar();
	} while (judge == 1);

}