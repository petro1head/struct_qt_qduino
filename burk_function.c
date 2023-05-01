#include "burk_function.h"
#include <math.h>
#include <stdlib.h>
#include "stdint.h"

//default setting function
void set_default(MySystem *ps) {

	ps->disc_integrator_angle.t_before = 0;
	ps->disc_integrator_angle.u = 0;

	ps->disc_integrator.t_before = 0;
	ps->disc_integrator.u = 0;

	ps->sum2.u = 0;

	ps->sum3.u = 0;

	ps->gain.u = 0;

	ps->sat.u = 0;

	ps->relay.u = 0;

	ps->ft.cnt_start = 0;
	ps->ft.is_engine_work = 0;
	ps->ft.t_off = 0;
	ps->ft.t_on = 0;
	ps->ft.u = 0;
}

/* Дискретный интегратор для интеграла угловой скорости*/
void disc_integratorIn_angle(MySystem *ps, double in_u, unsigned long t) {
	// Шаг дискретизации 
	unsigned long Td = 1;

	// Если мы ждали больше шага дискретизации
	if (t - ps->disc_integrator_angle.t_before >= Td) {
		// Обновляем значение интеграла
		ps->disc_integrator_angle.u += in_u * Td * 0.001;  // мс
		// Обновляем сохранённое время
		ps->disc_integrator_angle.t_before = t;
	}
	// Если еще не прождали нужный шаг дискретизации
	// То ничего не делаем, и выводим предыдущий результат
	// выход
}

/* Дискретный интегратор */
/* Подаём на вход сигнал */
// in_u - значение сигнала на входе в интегратор
// t - текущее время
void disc_integratorIn(MySystem *ps, double in_u, unsigned long t) {

	// Шаг дискретизации
	unsigned long Td = 1;

	// Если мы ждали больше шага дискретизации
	if (t - ps->disc_integrator.t_before >= Td) {
		// Обновляем значение интеграла
		ps->disc_integrator.u += in_u  * Td * 0.001;  // мс
		// Обновляем сохранённое время
		ps->disc_integrator.t_before = t;
	}
	// Если еще не прождали нужный шаг дискретизации
	// То ничего не делаем, и выводим предыдущий результат
	// выход
}

// Summator
void summatorIn2(MySystem *ps, double u1, double u2) {
	// Значение сигнала, полученного Сумматором
	ps->sum2.u = u1 + u2;
}

void summatorIn3(MySystem *ps, double u1, double u2, double u3) {
	// Значение сигнала, полученного Сумматором
	ps->sum3.u = u1 + u2 + u3;
}

// Gain
void gainIn(MySystem *ps, double in_u) {
	double multiplier = 0.1;
	ps->gain.u = in_u * multiplier;
}

// Saturation
void saturationIn(MySystem *ps, double in_u) {
	// Верхняя граница сигнала
	double upper = 0.5;
	// Нижняя граница сигнала
	double lower = -0.5;

	if (in_u >= upper) {
		ps->sat.u = upper;
	} else if (in_u <= lower) {
		ps->sat.u = lower;
	} else {
		ps->sat.u = in_u;
	}
}

// Relay
void relayIn(MySystem *ps, double in_u, double sp, double an) {
	// Порог срабатывания реле
	double positive = 0.03;
	double negative = -0.03;

	// При малых углах и угловых скоростях меняем параметры реле
	if (abs(sp) < 0.03 && abs(an) < 0.125) {
		positive = 0.125;
		negative = -0.125;
	}

	// Преобразуем сигнал либо к -1, либо 0, либо 1
	if (in_u >= positive) {
		ps->relay.u = 1;
	} else if (in_u <= negative) {
		ps->relay.u = -1;
	} else {
		// u принадлежит интервалу (negative, positive)
		ps->relay.u = 0;
	}
}

// Regulator

void regulatorIn(MySystem *ps, unsigned long t, double speed) {

	disc_integratorIn_angle(ps, speed, t);
	summatorIn2(ps, -(ps->disc_integrator_angle.u), -(ps->disc_integrator.u));
	gainIn(ps, ps->sum2.u);
	saturationIn(ps, ps->gain.u);
	summatorIn3(ps, -(speed), ps->sat.u, -(ps->relay.u));
	disc_integratorIn(ps, ps->sum3.u, t);
	relayIn(ps, ps->disc_integrator.u, speed, ps->disc_integrator_angle.u);

}

/* Ф от тау */
// F_ot_tau (30 мс)
//  tau - задержку на включение двигателя в [мс]
void f_ot_tauIn(MySystem *ps, unsigned long t, double in_u) {
	// Параметр тау
	unsigned long tau = 30;

	// Проверяем работает ли двигатель
	// Если работает
	if (ps->ft.is_engine_work) {
		// Находим время работы двигателя
		// Это текущее время минус время запуска двигателя
		unsigned long wake_time = t - ps->ft.t_on;
		// Если  время работы двигателя >30мс
		if (wake_time > tau) {
			// Нужно проверить значение сигнала
			// Если значение сигнала не равно сигналу при включении двигателя
			if (in_u != ps->ft.u) {
				// Нужно выключить двигатель
				ps->ft.is_engine_work = 0;
				// Задаём время выключения двигателя - текущее
				ps->ft.t_off = t;
				// сохранить значение сигнала, ставим именно 0
				// Потому что у нас ступенчатый вид сигнала
				// И должна быть его 0 фаза
				// Не может быть чтобы сигнал был,
				// например 0.5 и сразу стал -0.5
				ps->ft.u = 0;
			}
		}
	} else {
		// Двигатель не работает
		// Проверяем запускался ли он хоть раз
		// Если еще не запускался
		if (ps->ft.cnt_start == 0) {
			// То нам не нужно вычислять время спячки двигателя
			// И как только сигнал будет не нулевой
			// Включаем двигатель
			// если текущее значение сигнала НЕ нулевое
			if (in_u != 0) {
				// Включаем двигатель
				ps->ft.is_engine_work = 1;
				// Увеличиваем счётчик запусков двигателей
				ps->ft.cnt_start += 1;
				// сохраняем время запуска двигателя - текущее время
				ps->ft.t_on = t;
				// сохранить значение сигнала двигателя при старте
				// - то есть текущее значение сигнала
				ps->ft.u = in_u;
			}
		} else {
			// Двигатель УЖЕ запускался
			// Нужно проверить значение сигнала
			// Если сигнал НЕ нулевой
			if (in_u != 0) {
				// Включаем двигатель
				ps->ft.is_engine_work = 1;
				// Увеличиваем счётчик запусков двигателей
				ps->ft.cnt_start += 1;
				// сохраняем время запуска двигателя - текущее время
				ps->ft.t_on = t;
				// сохранить значение сигнала двигателя при старте
				// - то есть текущее значение сигнала
				ps->ft.u = in_u;
			}
		}
	}
}
// Делаем прогон через систему в процессоре 1907ВМ014(регулятор + ф от тау)
void SystemRun(MySystem *ps, unsigned long t, double speed) { 

	regulatorIn(ps, t, speed);
	f_ot_tauIn(ps, t, ps->relay.u);
}
// Что получаем на выходе из системы
double SystemOut(MySystem *ps) {
	// Система заканчивается Ф от тау
	return ps->ft.u;
}
