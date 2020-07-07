#include <Windows.h>

class EngineInternalCombustion
{
private:
	int momentInertiaEngine;                            //Момент инерции двигателя
	double *array_torque;                               //Крутящий момент
	double *array_crankshaftRotationSpeed;              //Скорость вращения коленвала
	int temperatureSuperheat;                           //Температура перегрева
	double	coefficientHeating_Torque;                  //Коэффициент зависимости скорости нагрева от крутящего момента
	double  coefficientHeating_Crankshaft;              //Коэффициент зависимости скорости нагрева от скорости вращения коленвала
	double  coefficientCooling_TemperatureEnviroment;   //Коэффициент зависимости скорости охлаждения от температуры двигателя и окружающей среды

	int sizeArray_torque;
	int sizeArray_crankshaftRotationSpeed;
	//Получение значений крутящего момента от скорости вращения коленвала при кусочно-линейной зависимости
	double	getTorque(int crankshaftRotationSpeed) {
		double torque;
		for (int i = 1; i < sizeArray_torque; i++) {
			if (crankshaftRotationSpeed == array_crankshaftRotationSpeed[i]) return array_torque[i];
			if (crankshaftRotationSpeed < array_crankshaftRotationSpeed[i] && crankshaftRotationSpeed > array_crankshaftRotationSpeed[i - 1]) {
				return torque = ((crankshaftRotationSpeed - array_crankshaftRotationSpeed[i - 1]) *  (array_torque[i] - array_torque[i - 1])) / (array_crankshaftRotationSpeed[i] - array_crankshaftRotationSpeed[i - 1]) + array_torque[i - 1];
			}
		}
		return array_torque[sizeArray_torque - 1];      // Возвращаем последнее значение графика, при условии что превысили максимальное количество оборотов
	}

	char* replace(char* out) {
		for (char * p = out; *p; ++p)
			if (*p == '.')
				*p = ',';
		return out;
	}
	double* readArrayConfig(LPCSTR key, double *array_, int &sizeArray_, LPCSTR path) {
		int size = 256;
		char* out = new char[size];
		DWORD dd = GetPrivateProfileStringA((LPCSTR)"EngineInternalCombustion", (LPCSTR)key, (LPCSTR) "empty", (LPSTR)out, sizeof(char) * size, path);

		sizeArray_ = 1;
		for (char * p = out; *p; ++p)
			if (*p == ',')
				sizeArray_++;
		char *ptr = 0; int i = 0;
		ptr = strtok(out, "{,}\n");
		array_ = new double[sizeArray_];
		while (ptr) {
			array_[i++] = atof(replace(ptr));
			ptr = strtok(0, "{,}\n");
		}
		return array_;
	}
public:
	EngineInternalCombustion(LPCSTR path) {
		DWORD dd;
		int size = 256;
		char* out = new char[size];

		momentInertiaEngine = GetPrivateProfileIntA((LPCSTR)"EngineInternalCombustion", (LPCSTR) "momentInertiaEngine", 0,  path);
		array_torque = readArrayConfig((LPCSTR) "array_torque", array_torque, sizeArray_torque, path);
		array_crankshaftRotationSpeed = readArrayConfig((LPCSTR) "array_crankshaftRotationSpeed", array_crankshaftRotationSpeed, sizeArray_crankshaftRotationSpeed, path);
		temperatureSuperheat = GetPrivateProfileIntA((LPCSTR)"EngineInternalCombustion", (LPCSTR) "temperatureSuperheat", 0, path);
		dd = GetPrivateProfileStringA((LPCSTR)"EngineInternalCombustion", (LPCSTR) "coefficientHeating_Torque", (LPCSTR) "empty", (LPSTR)out, sizeof(char) * size, path);
		coefficientHeating_Torque = atof(replace(out));
		dd = GetPrivateProfileStringA((LPCSTR)"EngineInternalCombustion", (LPCSTR) "coefficientHeating_Crankshaft", (LPCSTR) "empty", (LPSTR)out, sizeof(char) * size, path);
		coefficientHeating_Crankshaft = atof(replace(out));
		dd = GetPrivateProfileStringA((LPCSTR)"EngineInternalCombustion", (LPCSTR) "coefficientCooling_TemperatureEnviroment", (LPCSTR) "empty", (LPSTR)out, sizeof(char) * size, path);
		coefficientCooling_TemperatureEnviroment = atof(replace(out));
	}

	//Проверка данных считанных с конфигуционного файла
	bool isCorrect() {
		// Предполагаем, что размеры массивов одинаковые
		// Предпологаем, что размер массива имеет минимум 2 ячейки, это необходимо
		// Массив скорости вращения коленвала всегда возрастает
		// Предполагаем, что скорости вращения коленвала не может быть отрицательной;
		if (sizeArray_crankshaftRotationSpeed != sizeArray_torque) return false;
		if (sizeArray_crankshaftRotationSpeed < 2) return false;
		for (int i = 1; i < sizeArray_crankshaftRotationSpeed; i++) {
			if (array_crankshaftRotationSpeed[i] < array_crankshaftRotationSpeed[i - 1]) return false;
			if (array_crankshaftRotationSpeed[i] < 0) return false;
		}
		if (array_crankshaftRotationSpeed[0] < 0) return false;
		return true;
	}

	int getTime(int temperatureEnviroment) {
		int time = 0;
		double torque = array_torque[0];
		double crankshaftRotationSpeed = array_crankshaftRotationSpeed[0];
		int temperaturaEngine = temperatureEnviroment;

		double check[3];

		while (temperaturaEngine < temperatureSuperheat) {
			time++;
			double crankshaftAcceleration;
			double speedHeatingEngine = torque * coefficientHeating_Torque +
				crankshaftRotationSpeed * crankshaftRotationSpeed * coefficientHeating_Crankshaft;
			double speedCoolingEngine = coefficientCooling_TemperatureEnviroment * (temperatureEnviroment - temperaturaEngine);

			temperaturaEngine = temperaturaEngine + speedHeatingEngine + speedCoolingEngine;
			if (temperaturaEngine >= temperatureSuperheat) return time;

			crankshaftAcceleration = torque / momentInertiaEngine;
			crankshaftRotationSpeed = crankshaftRotationSpeed + crankshaftAcceleration;
			torque = getTorque(crankshaftRotationSpeed);

			//Если цикл бесконечный, "выбрасываем" текущую температуру двигателя
			if (torque == check[0] && crankshaftRotationSpeed == check[1] && temperaturaEngine == check[2]) {
				throw temperaturaEngine;
			}
			check[0] = torque;
			check[1] = crankshaftRotationSpeed;
			check[2] = temperaturaEngine;
		}
		return time;
	}
};
