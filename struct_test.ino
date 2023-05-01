
// typedef struct Data {
//   unsigned int t = 0;
//   float speed = 0;
// } Data;

// структура сумматора
typedef struct Sum {
  // выходной сигнал от сумматора
  double u = 0;
} Sum;

// структура коэффициента усиления Gain
typedef struct Gain {
  // выходной сигнал от коэффициента усиления
  double u = 0;
} Gain;

// структура насыщения Saturation
typedef struct Saturation {
  // выходной сигнал от насыщения
  double u = 0;
} Saturation;

// структура дискретного интегратора
typedef struct Disc_Integ {
  // переменные используемые внутри дискретного интегратора
  double t_before = 0;
  // выходной сигнал от дискретного интегратора
  double u = 0;

} Disc_Integ;

// структура реле
typedef struct Relay {
  // сигнал от реле
  double u = 0;

} Relay;

// структура Ф от тау
typedef struct F_ot_tau {
  // переменные для тау
  // сигнал сохранённый в тау
  double u = 0;
  // Время ВКЛючения двигателя
  unsigned long t_on = 0;
  // Время ВЫКЛючения двигателя
  unsigned long t_off = 0;
  // Логическая переменная (Работает ли двигатель?) - ДА, НЕТ
  // По умолчанию - НЕТ - не работает
  bool is_engine_work = false;
  // Счётчик, сколько раз запускался двигатель
  // По умолчанию 0, так как ни разу еще не запускался
  unsigned long cnt_start = 0;

} F_ot_tau;

// Глобальные переменные
// переменная типа структуры интегратора (угла)
// Integ integ;
// переменная типа структуры сумматора
Sum sum2;  // суммирует 2 сигнала
Sum sum3;  // суммирует 3 сигнала
// переменная типа структуры коэффициента усиления
Gain gain;
// переменная типа структуры насыщения
Saturation sat;
// переменная типа структуры дискретного интегратора1
Disc_Integ disc_integrator_angle;
// переменная типа структуры дискретного интегратора2
Disc_Integ disc_integrator;
// переменная типа структуры реле
Relay relay;
// переменная типа структуры Ф от тау
F_ot_tau ft;


// // интегратор (угол)
// void integratorIn(Integ *pI, double in_u, unsigned long t)
// {
//   pI->h = t - pI->t_before;
//   pI->s += in_u * pI->h * 0.001;
//   pI->t_before = t;
// }

/* Дискретный интегратор */
/* Подаём на вход сигнал */
// in_u - значение сигнала на входе в интегратор
// t - текущее время
void disc_integratorIn(Disc_Integ *p_DI, double in_u, unsigned long t) {

  // Параметры дискретного интегратора
  // k - коэффициент
  double K = 1;
  // Шаг дискретизации
   unsigned long T = 1;

  // Если мы ждали больше шага дискретизации
  if (t - p_DI->t_before >= T) {
    // Обновляем значение интеграла
    p_DI->u += in_u * K * T * 0.001;  // мс
    // Обновляем сохранённое время
    p_DI->t_before = t;
  }
  // Если еще не прождали нужный шаг дискретизации
  // То ничего не делаем, и выводим предыдущий результат
  // выход

}

// Summator
void summatorIn2(Sum *pSum, double u1, double u2) {
  // Значение сигнала, полученного Сумматором
  pSum->u = u1 + u2;
}


void summatorIn3(Sum *pSum, double u1, double u2, double u3) {
  // Значение сигнала, полученного Сумматором
  pSum->u = u1 + u2 + u3;
}


// Gain
void gainIn(Gain *pG, double in_u) {
  double multiplier = 0.1;
  pG->u = in_u * multiplier;
}


// Saturation
void saturationIn(Saturation *pSat, double in_u) {
  // Верхняя граница сигнала
  double upper = 0.5;
  // Нижняя граница сигнала
  double lower = -0.5;

  if (in_u >= upper) {
    pSat->u = upper;
  } else if (in_u <= lower) {
    pSat->u = lower;
  } else {
    pSat->u = in_u;
  }
}

// Relay
void relayIn(Relay *pR, double in_u, double sp, double an) {

  // Порог срабатывания реле
   double positive = 0.03;
   double negative = -0.03;  

  //   При малых углах и угловых скоростях, меняем параметры реле (расширяем область)
  if (abs(sp) < 0.03 && abs(an) < 0.125) {
    positive = 0.125;
    negative = -0.125;
  }

  // Преобразуем сигнал либо к -1, либо 0, либо 1
  if (in_u >= positive) {
    pR->u = 1;
  } else if (in_u <= negative) {
    pR->u = -1;
  } else {
    // u принадлежит интервалу (negative, positive )
    pR->u = 0;
  }
}

// Regulator

void regulatorIn(unsigned long t, double speed)
{
  disc_integratorIn (&disc_integrator_angle, speed, t);
  summatorIn2(&sum2, -disc_integrator_angle.u, -disc_integrator.u);
  gainIn(&gain, sum2.u);
  saturationIn(&sat, gain.u);
  summatorIn3(&sum3, -speed, sat.u, -relay.u);
  disc_integratorIn(&disc_integrator, sum3.u, t);
  relayIn(&relay, disc_integrator.u, speed, disc_integrator_angle.u);
}

/* Ф от тау */
// F_ot_tau
//  tau - задержку на включение двигателя в [мс]
void f_ot_tauIn(F_ot_tau *pF, unsigned long t, double in_u) {
  // Параметр тау
  unsigned long tau = 30;

  // Проверяем работает ли двигатель
  // Если работает
  if (pF->is_engine_work) {
    // Находим время работы двигателя
    // Это текущее время минус время запуска двигателя
    unsigned long wake_time = t - pF->t_on;
    // Если  время работы двигателя >30мс
    if (wake_time > tau) {
      // Нужно проверить значение сигнала
      // Если значение сигнала не равно сигналу при включении двигателя
      if (in_u != pF->u) {
        // Нужно выключить двигатель
        pF->is_engine_work = false;
        // Задаём время выключения двигателя - теущее
        pF->t_off = t;
        // сохранить значение сигнала, ставим именно 0
        // Потому что у нас ступенчатый вид сигнала
        // И должна быть его 0 фаза
        // Не может быть чтобы сигнал был,
        // например 0.5 и сразу стал -0.5
        pF->u = 0;
      }
    }
  } else {
    // Двигатель не работает
    // Проверяем запускался ли он хоть раз
    // Если еще не запускался
    if (pF->cnt_start == 0) {
      // То нам не нужно вычислять время спячки двигателя
      // И как только сигнал будет не нулевой
      // Включаем двигатель
      // если текущее значение сигнала НЕ нулевое
      if (in_u != 0) {
        // Включаем двигатель
        pF->is_engine_work = true;
        // Увеличиваем счётчик запусков двигателей
        pF->cnt_start += 1;
        // сохраняем время запуска двигателя - текущее время
        pF->t_on = t;
        // сохранить значение сигнала двигателя при старте
        // - то есть текущее значенеи сигнала
        pF->u = in_u;
      }
    } else {
      // Двигатель УЖЕ запускался
      // Нужно проверить значение сигнала
      // Если сигнал НЕ нулевой
      if (in_u != 0) {
        // Включаем двигатель
        pF->is_engine_work = true;
        // Увеличиваем счётчик запусков двигателей
        pF->cnt_start += 1;
        // сохраняем время запуска двигателя - текущее время
        pF->t_on = t;
        // сохранить значение сигнала двигателя при старте
        // - то есть текущее значение сигнала
        pF->u = in_u;
      }
    }
  }
}

void setup() {
  // Настройка вывода
  // инициирует последовательное соединение
  // и задает скорость передачи данных в бит/с (бод)
  Serial.begin(115200);
  // Отправляем сигнал, что готовы к работе
  Serial.println("OK");
  // Serial.println(sizeof(qt_data));
}

void SystemRun(unsigned long t, double speed) {  // Делаем прогон через систему
  // Передаём на вход регулятора значение времени, скорости и угла
  regulatorIn(t, speed);
  // В Ф(тау) передаём текущее время и выхлоп регулятора
  f_ot_tauIn(&ft, t, relay.u);
}
// Что получаем на выходе из системы
double SystemOut() {
 // Serial.println(ft.u);
  unsigned char* bytes = (unsigned char*)&ft.u;
  int len = sizeof(double);
  for (int i = 0; i < len; i++) // младший бит вперед 
  // for (int i = len - 1; i >= 0; i--) // старший бит вперед
   {
    Serial.write(bytes[i]);
  }
}

typedef struct Data {
  unsigned long t = 0;
  float speed = 0;
} Data;
byte readed = 0;
// получаем структуру
Data qt_data;

bool get_data() {
 char* bWrite = (const char*)&qt_data;
  bWrite += readed;

  while (Serial.available()) {
    if (readed >= sizeof(Data)) {
      readed = 0;
      return true;
    }

    *bWrite++ = Serial.read();
    readed++;
  }
  return false;
}

void loop() {
  if (Serial.available())
   {
    get_data();
    SystemRun(qt_data.t, qt_data.speed);
    //outPrint();
    SystemOut();
    }
  }
 
