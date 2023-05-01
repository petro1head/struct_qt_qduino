void setup() {
  Serial.begin(115200); // Инициализация последовательного порта
  float value = 1234.56; // Число для отправки
  send_float(value); // Отправка числа
}
void send_float(float value) {
  // Преобразуем указатель на число к типу unsigned char*
  unsigned char* bytes = (unsigned char*)&value;
  int len = sizeof(float);

  // Отправляем каждый байт по отдельности, передавая младший бит первым
  //for (int i = len - 1; i >= 0; i--)
  for (int i = 0; i < len; i++) 
   //for (int i = len - 1; i >= 0; i--)
   {
    Serial.write(bytes[i]);
  }
}
void loop() 
{

}