#ifndef SRC_BURK_FUNCTION_H_
#define SRC_BURK_FUNCTION_H_

// структура сумматора
typedef struct Sum {
	// выходной сигнал от сумматора
	double u;
} Sum;

// структура коэффициента усиления Gain
typedef struct Gain {
	// выходной сигнал от коэффициента усиления
	double u;
} Gain;

// структура насыщения Saturation
typedef struct Saturation {
	// выходной сигнал от насыщения
	double u;
} Saturation;

// структура дискретного интегратора
typedef struct Disc_Integ {
	// переменные используемые внутри дискретного интегратора
	double t_before;
	// выходной сигнал от дискретного интегратора
	double u;
} Disc_Integ;

// структура реле
typedef struct Relay {
	// сигнал от реле
	double u;
} Relay;

// структура Ф от тау
typedef struct F_ot_tau {
	// переменные для тау
	// сигнал сохранённый в тау
	double u;
	// Время ВКЛючения двигателя
	unsigned long t_on;
	// Время ВЫКЛючения двигателя
	unsigned long t_off;
	// Логическая переменная (Работает ли двигатель?) - ДА, НЕТ
	// По умолчанию - НЕТ - не работает
	// bool is_engine_work = false (0);
	int is_engine_work;
	// Счётчик, сколько раз запускался двигатель
	// По умолчанию 0, так как ни разу еще не запускался
	unsigned long cnt_start;

} F_ot_tau;

typedef struct MySystem {
	/* Глобальные переменные:*/

	// переменная типа структуры сумматора
	Sum sum2;  // суммирует 2 сигнала
	Sum sum3;  // суммирует 3 сигнала
	// переменная типа структуры коэффициента усиления
	Gain gain;
	// переменная типа структуры насыщения
	Saturation sat;
	// переменная типа структуры дискретного интегратора1 (интеграл угловой скорости)
	Disc_Integ disc_integrator_angle;
	// переменная типа структуры дискретного интегратора2
	Disc_Integ disc_integrator;
	// переменная типа структуры реле
	Relay relay;
	// переменная типа структуры Ф от тау
	F_ot_tau ft;
} MySystem;

void set_default(MySystem *ps);
void disc_integratorIn(MySystem *ps, double in_u, unsigned long t);
void disc_integratorIn_angle(MySystem *ps, double in_u, unsigned long t);
void summatorIn2(MySystem *ps, double u1, double u2);
void summatorIn3(MySystem *ps, double u1, double u2, double u3);
void gainIn(MySystem *ps, double in_u);
void saturationIn(MySystem *ps, double in_u);
void relayIn(MySystem *ps, double in_u, double sp, double an);
void regulatorIn(MySystem *ps, unsigned long t, double speed);
void f_ot_tauIn(MySystem *ps, unsigned long t, double in_u);
void SystemRun(MySystem *ps, unsigned long t, double speed);
double SystemOut(MySystem *ps);

#endif /* SRC_BURK_FUNCTION_H_ */
