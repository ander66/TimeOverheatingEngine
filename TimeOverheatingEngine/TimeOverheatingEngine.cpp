#include <iostream>
#include <string>
#include <fstream>
#include "EngineInternalCombustion.h"
#include <Windows.h>

using namespace std;

int main()
{
	//Корректность пользовательского ввода
	setlocale(LC_ALL, "Russian");
	int temperature;
	string str;
	while (true) {
		cout << "Введите температуру окружающей среды (C*): ";
		cin >> str;
		bool indicatorError = false;
		for (int i = 0; i < str.size(); i++) {
			if (!isdigit(str[i]) && str[i] != '-') {
				cout << "Введён некоректный символ " << str[i] << " Попробуйте ещё раз. \n";
				indicatorError = true;
				cin.ignore(32767, '\n');
				break;
			}
		}
		if (!indicatorError) {
			temperature = atoi(str.c_str());
			cin.ignore(32767, '\n');
			if (temperature >= -273 && temperature < 1000) break;
			else cout << "Температура окружающей среды должна быть в интервале [-273;1000]\n";
		}
	}


	//Достпупность конфигуционного файла
	LPCSTR path = ".\\Data.ini";
	ifstream file;
	file.open(path);
	if (!file) {
		cout << "Проблема открытия конфигурационного файла. Проверьте имя файла и его наличие.\n";
		return -1;
	}


	EngineInternalCombustion dvigatel(path);

	try
	{
		if (dvigatel.isCorrect()) {
			cout << "Время до перегрева: " << dvigatel.getTime(temperature) << " сек.";
		}
		else {
			cout << "Возникли проблемы при чтении конфигурационного файла. Проверьте настройки двигателя.";
			return 0;
		}
	}
	catch (int temperature)
	{
		cerr << "Систмема стабилизовалась. Температура двигателя не превышает " << temperature << " (C*)";
		return -1;
	}
}




