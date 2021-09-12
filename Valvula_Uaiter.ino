
#include <BlynkSimpleEsp8266.h>
#include <Ultrasonic.h> 

#define BLYNK_PRINT Serial
#define pinFluxo 13
#define RelePin 2
#define countLED 12
#define timeSeconds 0.5
#define pino_trigger 5
#define pino_echo 4

//Blynk
char auth[] = "";
//WiFi
char ssid[] = "";
char pass[] = "";

int j = 0;
String texto;
String texto2;
float valoresFluxo[50];
int i = 0;
int contPulso = 0;
unsigned long lastMillis;
unsigned long agr;
float fatorCalib = 4.5;
float fluxo = 0;
float volume = 0;
float volumeConsumido = 0;
//float volumeRecipiente = 7;     //Capacidade em L do Recipiente
float volRecipienteUtilizado;
float valorFluxoMedia = 0;
int aux2=1;
int aux=0;

Ultrasonic ultrasonic(pino_trigger, pino_echo);


ICACHE_RAM_ATTR void incPulso() 
{
  contPulso++;
}

void initInterrupt()
{
  attachInterrupt(digitalPinToInterrupt(pinFluxo), incPulso, FALLING); 
}

void endInterrupt()
{
  detachInterrupt(digitalPinToInterrupt(pinFluxo));
}

BLYNK_WRITE(V2)
{
  int rele = param.asInt();

  if( rele == 1 )
  {
    digitalWrite(RelePin, HIGH);
    texto2 = "Valvula aberta";
  }
  else
  {
    digitalWrite(RelePin, LOW);
    texto2 = "Valvula fechada";
  }
}

void VolumeAutomatico(float altura)
{
  if(altura < V7)
  {
     digitalWrite(RelePin, HIGH);
     texto2 = "Valvula aberta";
   //  Blynk.email("Atualizacao da Valvula Uaiter","A valvula está aberta, será fechada após o recipiente atingir o volume selecionado.");
     aux2 = 1;
  }
  
  if(altura >= V7)
   {
     digitalWrite(RelePin, LOW);
     texto2 = "Valvula fechada";
     Blynk.email("Atualizacao da Valvula Uaiter","Seu recipiente já esta cheio! A válvula foi fechada."); 
     aux2 = 0;  
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  Blynk.begin(auth, ssid, pass);
  
  Serial.begin(115200);
  pinMode(RelePin, OUTPUT);
  digitalWrite(RelePin, LOW);
  pinMode(pinFluxo, INPUT_PULLUP);
  pinMode(countLED, OUTPUT);  
  
  initInterrupt();
  lastMillis = millis();

  Serial.println("Setup Concluido");
}

//-------------------------------=---------------------------------------------=------------------------------------=-------------------------------------------=----------------------------------=----------------

void loop() 
{
  Blynk.run();
  digitalWrite(countLED, !digitalRead(pinFluxo)); 
  
  //espera 1s
  if((millis() - lastMillis) >= 1000) 
  {
    //finaliza a contagem de pulsos por segundo para realizar calculos
    endInterrupt();

    //sensor de fluxo 
    fluxo = ((1000 / (millis() - lastMillis)) * contPulso) / fatorCalib;
    volume = fluxo/60;
    volumeConsumido += volume;

    if (fluxo != 0)
    {
      valoresFluxo[i] = fluxo;        //recebe valore de fluxo para a média
    }

    if (i == 50)
    {
      for (j = 0; j<50 ; j++ )
      {
        valorFluxoMedia += valoresFluxo[j];
      }
      valorFluxoMedia = ( valorFluxoMedia / 50 );
      i = 0;
    }

    texto = "-Válvula Uaiter-";
    //sensor ultrassonico
    float alturaRecipiente;
    long microsec = ultrasonic.timing();
    alturaRecipiente = ultrasonic.convert(microsec, Ultrasonic::CM);
    Serial.println(alturaRecipiente);
    volRecipienteUtilizado = ((-0.45893) *alturaRecipiente +  7.69979);
    Serial.println(volRecipienteUtilizado);
    
    if( volRecipienteUtilizado <=0 )
    {
      volRecipienteUtilizado = 0;
    }

    aux = V7;
    //Verificacao do slider
     if(aux != 0 && aux2 == 1)
     {
      VolumeAutomatico(alturaRecipiente);    
     }

    
    //envio ao blynk
    Blynk.virtualWrite(V0, volumeConsumido);
    Blynk.virtualWrite(V1, fluxo);
    Blynk.virtualWrite(V3, valorFluxoMedia);
    Blynk.virtualWrite(V4, volRecipienteUtilizado);
    Blynk.virtualWrite(V5, texto);
    Blynk.virtualWrite(V6, texto2);
    //Blynk.virtualWrite(V, );

    
     
    contPulso = 0;
    
    lastMillis = millis();  //zera o tempo
    
    initInterrupt();
    i++;
  }


  
}
