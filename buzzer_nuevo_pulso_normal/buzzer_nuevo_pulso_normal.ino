/******** Librerias a utilizar, Pot digital, BT, ESP  *******/
#include "BluetoothSerial.h"
#include <Tone32.h>
#include <pitches.h>
//#define BUZZER_PIN 21
//#define BUZZER_CHANNEL 0
#define Sound_drv 26 // sound output GPIO13
#define PWM_ch 0
BluetoothSerial SerialBT;

/*------------------------------------------------------------------------*/


/*******  Declaracion de variables a utilizar  *********/
unsigned long myTime;
unsigned long myTime2; // Para contar los microsegundos del pulso
unsigned long myTime3;
unsigned long myTime4;
float voltajeSensor2 = 0.0, muestra2 = 0.0, voltajePWM2 = 0.0, celcius2 = 0.0;
String estado="0";
float I = 0; //valor de la corriente
float T = 0;// valor de la temperatura
float B = 0;// valor de la bateria
float corriente=0;
float contI=0;
String estadoI="0";
String estadoT="0";
String estadoB="0";
int t = 0; // para contar el numero de pulsos 
int f = 0; // para contar el intertren
int cont = 0; // contar el numero de trenes e intertrenes
int terapia = 0; // contar el numero de pulsos necesarios para los 45 min
char g; // variable donde se almacenan play pausa o stop
/*-----------------------------------------------------*/


/******* Declarar los pines de salida, del esp32 y la velocidad de comunicacion  *********/
void setup()
{
  pinMode(5, OUTPUT);// declaraciòn del puerto donde se genera el pulso
  digitalWrite(5,LOW);
  
  pinMode(18, OUTPUT); // Declaracion de pines para leds
    digitalWrite(18, HIGH);//encender led 
  pinMode(19, OUTPUT); // Declaracion de pines para leds

  
  float B=Medicion3();//leemos valor de la batería
  Serial.print(B);
      if (B>=3.2){//Valor mínimo de Bateria 
      
        SerialBT.begin("ACTIPULSE_H2"); // si es mayor al minimo, inicia la comunicación Serial BT y prende el led Azul
digitalWrite(18, HIGH);
}

else {
digitalWrite(18, LOW);
   digitalWrite(19, HIGH); //si el valor no es mayor al permitido, no se inicia la comunicación serial por BT, no se podrá conectar al celular y el led prenderá rojo
  }

  Serial.begin(115200); 
  ledcAttachPin(26, 0); // (Sound_drv, PWM_ch)
}
/*-----------------------------------------------------------------------------------------*/


/****** Rutina principal de estimualcion y medicion de sensores  **********/
void loop() {
  
  while (SerialBT.available() == 0) { //Espera hasta que haya un dato en el serial 
 float B=Medicion3();//leemos valor de la batería
  Serial.print(B);
  delay(500);
  }

/*---------------------------------------------------------*/
 
  g = SerialBT.read(); // Lectura de dato enviado desde la app
Serial.print(g);

  /**************Si el dato es un 1 (Play)*****************/
  if (g == '1') {
    
     if (terapia < 683 ) {
          /***Buzzer indicando el inicio de la estimulacion****/

ledcWriteTone(0, NOTE_A7); // tone 880 HZ freq (PWM_ch,A5)
Duraciontono();
ledcWriteTone(0, NOTE_F6); // tone 1109 Hz freq (PWM_ch,CS6)
Duraciontono(); // pause 
ledcWrite(0, 0); // tone off


    //tone(BUZZER_PIN, NOTE_A7, 250, BUZZER_CHANNEL);
    //noTone(BUZZER_PIN, BUZZER_CHANNEL);
    //tone(BUZZER_PIN, NOTE_F6, 250, BUZZER_CHANNEL);
    //noTone(BUZZER_PIN, BUZZER_CHANNEL);
    
    /*****************************************************/

}
while (SerialBT.available() == 0 && terapia < 683 && SerialBT.hasClient() ) { // mientras se cumplan todas las condiciones ( haya conexiòn BT,la terapia no pase de 45 min, no haya dato en el serial)
 
      while (t <= 1724) //Encierra en un ciclo las subrutinas, 3s ON (Tren de pulso)
      {
        Alto ();// funcion que define los pulsos en alto, en este caso 1 ms 
        Bajo ();// funcion que define los pulsos en alto, en este caso 740 micro segundos 
        t = t + 1;
      }

  myTime3 = millis();
  myTime4 = millis();
  digitalWrite(5, LOW);
  
  while ((myTime4 - myTime3) <= 1000)
  {
    myTime4 = millis();
  }
      t = 0; //Se reinician esos contadores para el proximo tren e intertren
      f = 0;
      cont++; // contador para mediciones
      terapia++; // contador para el tiempo total de la estimulacion  (45 min)

      if (cont == 15) { // cuando se cuentan 15 trenes e intertrenes (4 segundos entre los dos), tenemos al rededor de 60 segundos, 1 min. este es el tiempo que tarda en tomar las mediciones
        while (t <= 1724){ //Encierra en un ciclo las subrutinas, 3s ON (Tren de pulso)
      
        corriente=AltoMediciones ();// Cambiar por funcion de alto con mediciones. funcion que define los pulsos en alto, en este caso 1 ms 
        contI=contI+corriente;
        BajoMediciones ();// Cambiar por funcion de bajo con mediciones. funcion que define los pulsos en alto, en este caso 740 micro segundos
       
        t = t + 1;
        }
        t=0;
  myTime3 = millis();
  myTime4 = millis();
  digitalWrite(5, LOW);
  
  while ((myTime4 - myTime3) <= 100)
  {
    myTime4 = millis();
  }
        
          contI=contI/1724; 
        if (contI > 2) {
         estadoI="1";
        }
         String T=Medicion2();//Medicion de temperatura
  myTime3 = millis();
  myTime4 = millis();
  digitalWrite(5, LOW);
  
  while ((myTime4 - myTime3) <= 100)
  {
    myTime4 = millis();
  }         
          float B=Medicion3();//Mandamos llamar la funcion y es float porque nos regresa el valor de la medicion de la bateria
          

        
String datos ="0,"+String(B)+","+estadoI+","+String(contI)+","+T;// los valores de las mediciones y sus respectivos estados, se concatenan para enviarse a la app
Serial.print("\n B ="+String(B)+" I = "+ String(contI)+"T = "+T);
BTSendToPhone(datos); //funcion que envia dichos datos
       
        cont = 0; //se reinicia el contador para en los proximos 5 min volver a repetir el proceso.
        contI=0;
        terapia++; // contador para el tiempo total de la estimulacion  (45 min)
      }

      digitalWrite(5, LOW);
    }


  }
  if (g == '2') {// si lo que se recibe es un "2", es una pausa
    
    while (SerialBT.available() == 0) { // se queda esperando el play
      digitalWrite(5, LOW);
    }
  }
  if (g == '3' || terapia >= 683 ) { // si lo que se recibe es un "3", o ya se llegó a los 45 min o se perdió la conexión BT, se para la estimulacion
    terapia = 684;
    /********Sonido de Buzezer que indica el final de la Estimulacion*********/

ledcWriteTone(0, NOTE_E6); 
Duraciontono();
Duraciontono();
ledcWriteTone(0, NOTE_C6); 
Duraciontono();
Duraciontono();
ledcWrite(0, 0); // tone off
    
   // tone(BUZZER_PIN, NOTE_E6, 500, BUZZER_CHANNEL);
    //noTone(BUZZER_PIN, BUZZER_CHANNEL);
    //tone(BUZZER_PIN, NOTE_C6, 500, BUZZER_CHANNEL);
    //noTone(BUZZER_PIN, BUZZER_CHANNEL);
    //delay(2000);
 /****************************************************************************/

    digitalWrite(5, LOW);
 
  }
}
/******  Rutina de un pulso en alto 1 ms *********/
void Alto()
{
  myTime = micros();
  myTime2 = micros();
  digitalWrite(5, HIGH);
  while ((myTime2 - myTime) <= 1000)
  {
    myTime2 = micros();
  }
}
/*-------------------------------------------*/


/******  Rutina de un pulso bajo 740 micro segundos *********/
void Bajo()
{
  myTime = micros();
  myTime2 = micros();
  digitalWrite(5, LOW);
  while ((myTime2 - myTime) <= 740)
  {
    myTime2 = micros();
  }
}
/*-------------------------------------------*/


///******  Parte de medir temperatura  ***************/
String Medicion2()
{
    voltajeSensor2 = analogRead(34);
Serial.print("\n ANALOGICO DIVISOR"+String(voltajeSensor2));
  voltajePWM2= ((voltajeSensor2 *3.3)/4096)*1.5;
Serial.print("\nConversion sensor"+String(voltajePWM2));
celcius2=voltajePWM2/.01;
if (celcius2 > 65){
  estado="1";
   
}  
 return (estado+","+String(celcius2));
}
/////*-----------------------------------------------------*



///******** Parte medir bateria  *********************/
float Medicion3()
{
  float voltajeSensor3 = 0.0, muestra3 = 0.0, voltajePWM3 = 0.00, celcius3 = 0.00, voltajePWM = 0.00;
  String estado = "0";
  for (int i = 0; i < 10; i++)
  {
    voltajeSensor3 = analogRead(35);
    muestra3 = muestra3 + voltajeSensor3;
  }
  voltajePWM3 = muestra3 / 10.0;
  voltajePWM =  (voltajePWM3 * 3.3) / 4096.0;
  celcius3 = voltajePWM;
  return (celcius3);
}
/*----------------------------------------------------*/


/***** Funcion para enviar los datos a la app por BT serial *****/
void BTSendToPhone(String string) {
  uint8_t buf[string.length()];
  memcpy(buf, string.c_str(), string.length());
  SerialBT.write(buf, string.length());
  SerialBT.println("");
}
/***************************************************************/


/******  Rutina de un pulso en alto 1 ms *********/
float AltoMediciones()
{
  float voltajeSensor = 0.0,Cero=0.0,voltajePWM=0.0;
  Cero = analogRead(39);
   voltajeSensor = analogRead(39);
 // Serial.print("\nCero: "+String(voltajeSensor));
  myTime = micros();
  myTime2 = micros();
  digitalWrite(5, HIGH);
  while ((myTime2 - myTime) <= 927)
  {
    myTime2 = micros();
  }
  voltajeSensor = analogRead(39);
  //Serial.print("\nMedida: "+String(voltajeSensor));
  //voltajePWM = abs(((voltajeSensor - Cero) / ((4095 - Cero) / 5)));
voltajePWM = (abs(voltajeSensor - Cero)*5)/Cero;
  return(voltajePWM);
}
/*-------------------------------------------*/

/******  Rutina de un pulso bajo 740 micro segundos *********/
void BajoMediciones()
{
  myTime = micros();
  myTime2 = micros();
  digitalWrite(5, LOW);
  while ((myTime2 - myTime) <= 655)
  {
    myTime2 = micros();
  }
}
/*-------------------------------------------*/

/****** DURACION TONOS *********/
void Duraciontono()
{
  myTime = millis();
  myTime2 = millis();
  while ((myTime2 - myTime) <= 500)
  {
    myTime2 = millis();
  }
}
/*-------------------------------------------*/
