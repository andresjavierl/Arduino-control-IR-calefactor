
//*************************************************************
//**** Control de temperatura para caloventor de baño *********
//*************************************************************

#include <DHT.h>    // importa la Librerias DHT
#include <DHT_U.h>
#include <TM1637Display.h>    // incluye libreria TM1637
#include <IRremote.h>			// importa libreria IRremote

#define CLK 4       // CLK a pin 4
#define DIO 5       // DIO a pin 5

TM1637Display display(CLK, DIO);  // crea objeto
const uint8_t alto[] = {    // array para almacenar caracteres especiales
  SEG_F | SEG_G | SEG_B | SEG_F | SEG_E | SEG_C,    // H
  SEG_B | SEG_C      // I
};
//****  Codigos remoto caloventor  **********
//#define Power 0x70385D93
//#define Mode 0xCDFA7E97
//#define Swing 0x55A8EBF7
//#define Timer 0x9BFD328E

unsigned int Power[77] = {11298, 4478,  678, 518,  680, 520,  698, 520,  678, 548,  688, 548,  694, 548,  688, 548,  686, 548,  688, 1664,  704, 1662,  706, 1636,  716, 1666,  750, 1638,  716, 1668,  728, 1666,  710, 1638,  696, 550,  704, 548,  692, 546,  670, 1634,  684, 520,  674, 548,  702, 548,  666, 516,  658, 1630,  656, 1632,  654, 1630,  654, 514,  658, 1630,  656, 1630,  654, 1630,  652, 1630,  652, 40236,  9690, 2242,  658};
unsigned int Mode [71]= {12174, 4322,  766, 622,  706, 578,  732, 620,  726, 550,  736, 524,  694, 548,  662, 548,  688, 590,  688, 1694,  708, 1664,  716, 1666,  704, 1664,  678, 1634,  680, 1664,  722, 1666,  704, 1664,  784, 470,  684, 578,  706, 1692,  724, 1694,  728, 552,  702, 550,  678, 548,  686, 520,  688, 1664,  672, 1662,  674, 518,  662, 518,  662, 1632,  660, 1634,  694, 1636,  686, 1666,  794, 40176,  9708, 2242,  712};  // UNKNOWN CDFA7E97
unsigned int Swing [71] = {12438, 4474,  714, 660,  748, 552,  754, 622,  738, 590,  736, 594,  706, 618,  728, 592,  708, 620,  736, 1708,  714, 1734,  712, 1694,  812, 1716,  774, 1712,  758, 1716,  756, 1642,  784, 1734,  848, 528,  818, 552,  908, 474,  854, 502,  810, 596,  760, 596,  746, 594,  742, 550,  786, 1668,  758, 1666,  710, 1664,  726, 1638,  696, 1664,  708, 1638,  684, 1662,  710, 1638,  694, 40232,  11866, 2170,  728};  // UNKNOWN 55A8EBF7
unsigned int Timer [79] = {1250, 19764,  2862, 19804,  994, 19782,  7242, 19836,  11688, 4474,  696, 546,  724, 550,  694, 574,  708, 616,  696, 578,  748, 550,  716, 548,  706, 548,  708, 1662,  814, 1554,  706, 1662,  702, 1660,  724, 1634,  698, 1660,  726, 1634,  702, 1662,  730, 522,  708, 548,  714, 1662,  712, 548,  718, 550,  720, 550,  724, 550,  722, 550,  716, 1662,  738, 1636,  708, 548,  716, 1664,  736, 1636,  738, 1638,  728, 1692,  748, 1634,  712, 40338,  12070, 2272,  744};

//****  Codigos remoto TV samsung  **********
//#define Vol_up 0xE0E0E01F
//#define Vol_down 0xE0E0D02F

int value;
IRsend irsend;
int khz = 38; // 38kHz carrier frequency for the NEC protocol

int SENSOR = 6;     // pin DATA de DHT11 a pin digital 6
int TEMPERATURA;
int HUMEDAD;
int Pulsador_Encendido = A0;
int Pulsador_Modo = A1;
int Pulsador_Timer = A2;
int Pulsador_Swing = A3;

int Led_Power = 2;
int Set_alarma = 20;
int muestra_temp1;
int muestra_temp2;
int muestra_temp3;
int temp_final;
bool flag1 = true;
bool flag2 = true;
bool flag3 = false;
unsigned long tiempo_ant_millis;
unsigned long tiempo_act_millis;
int Tpo_espera_on = 0;
int Tpo_espera_off = 0;
int hysteresis = 2; // Hysteresis para sumar/restar a set alarma
int entrar_Set = 0; // contador para entrar a set de temperatura
bool calefaccionar = 0;
bool BandModoCalentarAuto = 0;
bool prendido = 0;
DHT dht(SENSOR, DHT11);   // creacion del objeto dht

void setup(){
  Serial.begin(9600);   // inicializacion de monitor serial
  dht.begin();          // inicializacion de sensor
  display.setBrightness(1);   // establece nivel de brillo
  Serial.println("******* IR caloventor Baño *******");
  Serial.println(F("START " __FILE__ " from " __DATE__ ));

  pinMode(Pulsador_Encendido, INPUT_PULLUP);
  pinMode(Pulsador_Modo, INPUT_PULLUP);// Se activa resistencia pull UP interna
  pinMode(Pulsador_Timer, INPUT_PULLUP);
  pinMode(Pulsador_Swing, INPUT_PULLUP);
  pinMode(Led_Power, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(Led_Power, LOW);
  toma_Muestras(); //toma la primera muestra para mostrar
  display.showNumberDec(muestra_temp3, false, 2, 0); // muestra valor temp_final
  display.showNumberDec(Set_alarma,false, 2, 2);   // muestra set alarma
}

//************* LOOP principal ***********
void loop()
{
// **************** RUTINA PULSADOR ENCENDIDO VENTILADOR **************
  if (digitalRead(Pulsador_Encendido) == LOW) //evaluo si la entrada esta en nivel BAJO
  {
    while (digitalRead(Pulsador_Encendido) == LOW) //retraso antirebote
     {
      entrar_Set ++;
      delay(200);
      if (entrar_Set == 5) //si paso un segundo apretado
      {
        Serial.println("Set de temperatura");
      }
     }

  if (prendido == 0)//** estaba Encendido?
  {
    irsend.sendRaw(Power, sizeof(Power) / sizeof(int), khz);
    delay(20);
    prendido = 1;
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Prendido");

  }else // No edtaba prendido
      {
        calefaccionar = 0;
        BandModoCalentarAuto = 0;
        prendido = 0;
        irsend.sendRaw(Power, sizeof(Power) / sizeof(int), khz);
        delay(20);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("Apagado");
      }
   } //***********  FIN PULSADOR ENCENDIDO  ***************

if (prendido == 1)//Verifica si esta Encendido para comprobar los otros botones
{
  //   ****** rutina pulsador MODO ventilador o calefaccion ********
  if (digitalRead(Pulsador_Modo) == LOW)  // evaluo si la entrada esta en nivel BAJO
    {
      while (digitalRead(Pulsador_Modo) == LOW) //retraso antirebote
       {

        delay(10);
       }
      if (calefaccionar == 0)
       {
        for (int i = 0; i < 2; i++)
         {
           //  Encender calefaccion
           irsend.sendRaw(Mode, sizeof(Mode) / sizeof(int), khz);
           delay(200);
         }
         calefaccionar = 1;
         Serial.println("Calefaccion encendida");
         digitalWrite(Led_Power, HIGH);
         }else
         {
           //  Apagar calefaccion
           irsend.sendRaw(Mode, sizeof(Mode) / sizeof(int), khz);
           delay(100);
           calefaccionar = 0;
           Serial.println("Calefaccion Apagada");
           digitalWrite(Led_Power, LOW);
         }

    }  //***********  FIN PULSADOR MODO  ***************

// ******  ****** RUTINA PULSADOR TIMER ********
      if (digitalRead(Pulsador_Timer) == LOW) // evaluo si la entrada esta en nivel alto
        {
          irsend.sendRaw(Timer, sizeof(Timer) / sizeof(int), khz); //Note the approach used to automatically calculate the size of the array.
          delay(20);
          Serial.println("TIMER");
        }

// ************ RUTINA PULSADOR Swing ********
      if (digitalRead(Pulsador_Swing) == LOW) // evaluo si la entrada esta en nivel alto
        {
          irsend.sendRaw(Swing, sizeof(Swing) / sizeof(int), khz); //Note the approach used to automatically calculate the size of the array.
          delay(20);
          Serial.println("SWING");
        }

} //**** Cierra IF comprobacion de estado ENCENDIDO ****

 //***** TEMPORIZADOR PARA CADA MUESTRA TEMPERATURA ******
 tiempo_act_millis = millis();
if ( tiempo_act_millis - tiempo_ant_millis >= 5000) // Cada 5 seg
  {
   toma_Muestras(); //Va a tomar muestras de temperatura
   tiempo_ant_millis = tiempo_act_millis;

  //  Si ya se tomaron las 3 muestras promediar y mostrar
   if (flag1==true && flag2 == true && flag3 == true)
   {
       temp_final = (muestra_temp1+muestra_temp2+muestra_temp3) / 3;
       flag1 = true;
       flag2 = true;
       flag3 = false;
       mostrar_Display();
// sia esta encendido calefaccion controlar set
   if (calefaccionar == 1)
    {
       calentar_ON_OFF (); // Check de temp final y set -> ON / OFF calefa
    }
   }
  }

} //******************** Fin LOOP principal **********************


//**************************************************************
//********************* Toma de muestras ***********************
//**************************************************************
void toma_Muestras()
{
    Serial.println("********   Tomando muestra ********");
    TEMPERATURA = dht.readTemperature();  // obtencion de valor de temperatura
    HUMEDAD = dht.readHumidity();   // obtencion de valor de humedad
    if (flag1 == false)
    {
        muestra_temp1 = TEMPERATURA;
        flag1 = true;
        Serial.println("M1: "+String(muestra_temp1));

    }

    if (flag2 == false)
    {
        muestra_temp2 = TEMPERATURA;
        flag2 = true;
        flag1 = false;
        Serial.println("M2: "+String(muestra_temp2));

    }

    if (flag3 == false)
    {
        muestra_temp3 = TEMPERATURA;
        flag3 = true;
        flag2 = false;
        Serial.println("M3: "+String(muestra_temp3));

    }
} //********************* fin toma de muestras ****************

//**************************************************************
// ************************ Mostar Display *********************
//**************************************************************
void mostrar_Display()
{

      display.showNumberDec(temp_final, false, 2, 0); // muestra valor temp_final

      Serial.print("Temperatura: ");  // escritura en monitor serial de los valores
      Serial.println(TEMPERATURA);
      //Serial.print("Temperatura final: ");
      //Serial.println(temp_final);
      Serial.print("Humedad: ");
      Serial.println(HUMEDAD);
      Serial.println("M1: "+String(muestra_temp1) +" M2: "+String(muestra_temp2)+" M3: "+String(muestra_temp3)+" FINAL: "+String(temp_final));
      Serial.println("*********************************** ");
      //Serial.println(" ");


}

//******     Rutina para apagar caloventor  *************
void calentar_ON_OFF(){
       if (calefaccionar == 1)
       {
         if (temp_final >= Set_alarma + hysteresis)
         {
           Tpo_espera_off++;  //si esta alta suma 1 a la espera para apagar
           Tpo_espera_on = 0;
           Serial.println("Esperando para apagar");
           if (Tpo_espera_off >=2) // si pasaron 10 seg
            {
          //  if (Led_Power==HIGH) {
              for (int i = 0; i >= 2; i++)
              {
                irsend.sendRaw(Mode, sizeof(Mode) / sizeof(int), khz); //Note the approach used to automatically calculate the size of the array.
                delay(500);
              }

             Serial.println("Calefaccion AUTO: Apagado ");
             digitalWrite(Led_Power,LOW); // Caloventor apagado
             display.setSegments(alto, 2, 2); // muestra HI
            }
         }

 //******    Rutina para encender caloventor  ************
       if ( temp_final <= Set_alarma )
         {
           Tpo_espera_on++;
           Tpo_espera_off = 0;
           if (Tpo_espera_on >= 2)
           {
            if (Led_Power == LOW) {
              for (int i = 0; i < 1; i++)
              {
                irsend.sendRaw(Mode, sizeof(Mode) / sizeof(int), khz); //Note the approach used to automatically calculate the size of the array.
                delay(200);
                Serial.println("**** IR Apagado ****");
              }
            }

             Serial.println("Calefaccion AUTO: Encendido " + String(Tpo_espera_on));
             digitalWrite(Led_Power,HIGH); // pulsador presionado, encender LED
             display.showNumberDec(Set_alarma,false, 2, 2);   // muestra set alarma
           }
         }
       }

  }
