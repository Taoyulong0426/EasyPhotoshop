#include "bmp.h"
#include "operate.h"
#include <string>
using namespace std;

int main() {
	int judge;
	string input_path;
	do
	{
		cout << "******����������ͼƬ·������(""C:\\..."")******\n" << endl;
		getline(std::cin, input_path);
		//ModelTrans modelTrans = ModelTrans(R"(E:\SimplePhotoshop2\test\lenna.bmp)");
		Operate operation = Operate(input_path.c_str());
		operation.doOperation();
		cout << "\n******����ʹ������1,�˳�����0******\n" << endl;
		cin >> judge;
		getchar();
	} while (judge == 1);

}