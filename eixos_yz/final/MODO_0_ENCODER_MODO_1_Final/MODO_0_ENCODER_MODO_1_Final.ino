/*---------------------------------------
Iniciação - bibliotecas WiFi e MQTT
---------------------------------------*/
#include <WiFi.h> //WiFi.h é usado para conectar o ESP32 a uma rede Wi-Fi 
#include <PubSubClient.h> //PubSubClient.h é usada para comunicação MQTT 

/*---------------------------------------
variavéveis
---------------------------------------*/
int y_cima = 13; // cima representa o pin 1 do opto-relé que faz o eixo y ir para cima;
int y_baixo = 4; // baixo representa o pin 2 do opto-relé que faz o eixo y ir para baixo;
int z_frente = 33; // frente representa o pin 3 do opto-relé que faz o eixo z andar para a frente (longe do zero);
int z_tras = 32; // tras representa o pin 4 do opto-relé que faz o eixo z andar para trás (perto do zero);
int ENC_Ay = 16; //Pin1_optocoupler1 ligado ao canal A do encoder y;
int ENC_By= 17; //Pin2_optocoupler1 ligado ao canal B do encoder y;
int fcy_cima = 18;//Pin3_optocoupler1 ligado ao fim de curso de cima;
int fcy_baixo = 19;//Pin4_optocoupler1 ligado ao fim de curso de baixo;
int sref_y = 21; //Pin5_optocoupler1 ligado ao sensor de referência do eixo y;
int ENC_Az = 22; //Pin1_optocoupler1 ligado ao canal A do encoder z;
int ENC_Bz = 23; //Pin2_optocoupler1 ligado ao canal B do encoder z;
int fcz_frente = 25;//Pin3_optocoupler1 ligado ao fim de curso da frente;
int fcz_tras = 26;//Pin4_optocoupler1 ligado ao fim de curso de tras;
int sref_z = 27; //Pin5_optocoupler1 ligado ao sensor de referÊncia do exio z;

volatile int Py= 0; 
volatile int Pz= 0; 

int Modo=2; //Modo de funcionamento 2 significa stand by. Os outros modos são 0 e 1
int concluido; //variável para demonstrar que determinado pedido foi concluído

//Variáveis "intermédias" usadas no loop
int FC_y_cima, FC_y_baixo, SREF_Y, FC_z_frente, FC_z_tras, SREF_Z;

/*---------------------------------------
Funções encoders
---------------------------------------*/
//Função do encoder eixo y
void isr_y() {
  if (digitalRead(ENC_By)) {
    Py ++;
  } else {
    Py --;
  }
  //Serial.print("y: ");  motivo de erro
  //Serial.println(count_Py);
}

//Função do encoder eixo z
void isr_z() {
  if (digitalRead(ENC_Bz)) {
    Pz ++;
  } else {
    Pz --;
  }
  //Serial.print("z: ");
  //Serial.println(count_Pz);
}

/*-------------------------------------------
Conecção WiFi - Depende de telemóvel pessoal
-------------------------------------------*/
const char *SSID = "Rede do Maia"; //nome da rede wifi
const char *PWD = "21_7PabM"; // palavra-passe da rede wifi

/*---------------------------------------
Conecção ao Mqtt Server
---------------------------------------*/
WiFiClient wifiClient; //Estas variáveis estão relacionadas à configuração do cliente MQTT. 
//mqttClient é o cliente MQTT que se conecta ao servidor MQTT definido por mqttServer e mqttPort.
PubSubClient mqttClient(wifiClient); 
char *mqttServer = "193.137.172.20"; 
int mqttPort = 85;
long lastMsg = 0; //Tempo da ultima mensagem
const long interval = 1000; //Intervalo entre mensagens

/*---------------------------------------
Váriavies usadas para o Mqtt Server
---------------------------------------*/
// long last_time=0; <--- Ver se é preciso ou apagar
char data[100]; //Armazenamento de mensagens trocadas
String Command;
int Pdy; //Posião desejada em y e z
int Pdz;

/*---------------------------------------
Conecção ao  WiFi: conecção  do ESP32 à rede Wi-Fi especificada pelos SSID e senha
---------------------------------------*/
void connectToWiFi() {
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

/*---------------------------------------
Conecção ao MQTT server: Esta função configura o cliente MQTT para se conectar ao servidor MQTT 
//especificado e define a função de retorno de chamada (callback) que será chamada quando mensagens forem recebidas.
---------------------------------------*/
void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}

//Esta função é chamada para reconectar o cliente MQTT se a conexão for perdida. Ele gera um ID de cliente aleatório e se conecta ao servidor MQTT. 
//Em seguida, ele se inscreve em tópicos específicos para receber comandos e dados de sensores.
void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-"; //criação de um iD aleatório que muda de cada vez que é necessário conectar.
    clientId += String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      // subscribe to topic
      Serial.println("connected");
      mqttClient.subscribe("PSA24G2_yzCommand");
    }   
  }
}

/*---------------------------------------
Setting the callbacks for MQTT server: Esta função é chamada sempre que uma mensagem MQTT é recebida. 
//Ele analisa o tópico da mensagem e atualiza as variáveis apropriadas com os dados recebidos.
---------------------------------------*/
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if (strcmp(topic,"PSA24G2_yzCommand")==0){
    Command=message; 
  }
  
  //Separação da informação dos eixos  y e z 
  // 1º Passo: Encontrar os índices dos separadores
  int M_Index = Command.indexOf('M');
  int Y_Index = Command.indexOf('Y');
  int Z_Index = Command.indexOf('Z');
  
  // 2º Passo: Extrair as partes das mensagens
  Modo = Command.substring(M_Index+1, M_Index+2).toInt(); //Numero do modo de funcionamento
  String yPart = Command.substring(Y_Index + 1, Y_Index+6);
  String zPart = Command.substring(Z_Index + 1, Z_Index+6);

  //3ºPasso: Coverter valor para inteiro 
  Pdy = yPart.toInt();
  Pdz = zPart.toInt();
  concluido=0; //"Reiniciar" variável

  //"Imprimir os valores de Pdy e Pdz"  <--- Retirar depois
  //Serial.print("yPart: ");
  //Serial.println(Pdy);
  //Serial.print("zPart: ");
  //Serial.println(Pdz);
}

/*---------------------------------------
FUNÇÃO COMUNICAR COM O MQTT
---------------------------------------*/
void sendMQTTMessage(){
  long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    sprintf(data, "C%iY%iZ%i",concluido,Py,Pz);
    mqttClient.publish("PSA24G2_yzFeedback",data);
  }
}

void setup() {
  //Saídas - variadores
  pinMode(y_cima,OUTPUT); 
  pinMode(y_baixo,OUTPUT);
  pinMode(z_frente,OUTPUT);
  pinMode(z_tras,OUTPUT);

  //Entradas - sensores
  pinMode(fcy_cima,INPUT);
  pinMode(fcy_baixo,INPUT);
  pinMode(sref_y,INPUT);
  pinMode(fcz_frente,INPUT);
  pinMode(fcz_tras,INPUT);
  pinMode(sref_z,INPUT);

  //Iniciakização do relé (apesar de estar HIGH como este funciona ao contrário na verdade é LOW)
  digitalWrite(y_cima, HIGH);
  digitalWrite(y_baixo,HIGH);
  digitalWrite(z_frente,HIGH);
  digitalWrite(z_tras,HIGH);
  
  Serial.begin (19200);   //<--- Retirar depois

  // Set encoder pins and attach interrupts
  pinMode(ENC_Ay, INPUT);
  pinMode(ENC_By, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_Ay), isr_y, RISING);
  pinMode(ENC_Az, INPUT);
  pinMode(ENC_Bz, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_Az), isr_z, RISING);

  //Connect to Wifi and MQTT
  connectToWiFi();
  setupMQTT();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  FC_y_baixo=digitalRead(fcy_baixo); //Fim de curso é normalmente fechado e optocoupler inverte, o que faz que quando toca no fc fica a 3.3V
  SREF_Y=!digitalRead(sref_y);       //Sensor de ref é normalmente aberto, e o optocoupler inverte, o que faz que quando toca no sref fica a 0V
  FC_z_tras=digitalRead(fcz_tras); 
  SREF_Z=!digitalRead(sref_z);
  static int lastPy = 0; // variáveis de visualização - apagar mais tarde
  static int lastPz = 0;

  if (Modo == 0){
    /*---------------------------------------
    Modo 0
    ---------------------------------------*/

    concluido=0; // Inicialização da variável
    digitalWrite(y_cima,LOW);  //ligar o motor para andar para baixo
    while (!SREF_Y) {
      FC_y_baixo=digitalRead(fcy_baixo);
      SREF_Y=!digitalRead(sref_y);
      Serial.println(SREF_Y);
      if(Py != lastPy){   // Visualização - apagar mais tarde
        Serial.print("Y: ");
        Serial.println(Py);
        lastPy = Py;
      }

      if(FC_y_baixo){
        digitalWrite(y_cima,HIGH); // Inverter sentido se sensor abrir (entrada a 3v3)
        digitalWrite(y_baixo,LOW);
        
        while (!SREF_Y) {
          SREF_Y=!digitalRead(sref_y);
          sendMQTTMessage();
          
          if(Py != lastPy){   // Visualização - apagar depois
            Serial.print("Y: ");
            Serial.println(Py);
            lastPy = Py;
          }
        }
      }
      sendMQTTMessage(); // função enviar sms MQTT
    }

    digitalWrite(y_cima,HIGH);    //Conclusão do modo 0 em y
    digitalWrite(y_baixo,HIGH);
    Py=0;

    digitalWrite(z_tras,LOW);   //Início do modo 0 em z, andar para trás
    while (!SREF_Z){
      FC_z_tras=digitalRead(fcz_tras);
      SREF_Z=!digitalRead(sref_z);

      if(Pz != lastPz){   //Visualização - apagar depois
        Serial.print("Z: ");
        Serial.println(Pz);
        lastPz = Pz;
      }

      if(FC_z_tras){
        digitalWrite(z_tras,HIGH);  //Inverter sentido - passa a andar para a frente
        digitalWrite(z_frente,LOW);

        while (!SREF_Z) {
          SREF_Z = !digitalRead(sref_z);// pin 27
          sendMQTTMessage(); // função enviar sms MQTT
          
          if(Pz != lastPz){ //Visualização - apagar depois
            Serial.print("Z: ");
            Serial.println(Pz);
            lastPz = Pz;
          }
        }
      }
      sendMQTTMessage(); // função enviar sms MQTT
    }
    digitalWrite(z_tras,HIGH);  //desligar motores
    digitalWrite(z_frente,HIGH);    
    Pz=0;
    
    Modo=2; // modo stand-by
    concluido=1;  //enviar mensagem de concluído após 2 segundos
    delay(2000);
    sendMQTTMessage();
  
  } else if(Modo==1) {
    /*---------------------------------------
    Modo 1 
    ---------------------------------------*/

    Serial.println(Pdy); //Visualização - apagar depois
    Serial.println(Pdz);

    //Eixo y
    while(Py<Pdy-20 && Modo==1){
      FC_y_baixo=digitalRead(fcy_baixo);
      FC_y_cima=digitalRead(fcy_cima);
      
      digitalWrite(y_cima,LOW);   //Sentido positivo de baixo para cima
      sendMQTTMessage(); // função enviar sms MQTT
      
      if(Py != lastPy){   //Visualização - apagar depois
        Serial.print("Y: ");
        Serial.println(Py);
        lastPy = Py;
      }

      if(FC_y_cima || FC_y_baixo){    //Evitar exceder os limites - lança um erro
        digitalWrite(y_cima,HIGH);    //Desliga o motor
        digitalWrite(y_baixo,HIGH);
        Modo = 0; //Volta para o ponto zero para recalibrar
        concluido=2;  //O valor 2 indica que foi concluído com erro
        delay(2000);
        sendMQTTMessage();
        break;
      }
    }
    while (Py>Pdy+20 && Modo==1){
      digitalWrite(y_baixo,LOW);  //sentido negativo - para baixo
      
      FC_y_baixo=digitalRead(fcy_baixo);
      FC_y_cima=digitalRead(fcy_cima);
      sendMQTTMessage(); // função enviar sms MQTT
      
      if(Py != lastPy){  //Visualização - apagar depois
        Serial.print("Y: ");
        Serial.println(Py);
        lastPy = Py;
      }
      
      if(FC_y_cima || FC_y_baixo){ // Para garantir que não se ultrapassam os limites dos sensores de fim de curso
        digitalWrite(y_cima,HIGH);  //desligar o motor
        digitalWrite(y_baixo,HIGH);
        Modo =0;  //voltar ao zero
        concluido=2;  //concluído com erro
        delay(2000);
        sendMQTTMessage();
        break;
      }
    }

    digitalWrite(y_cima,HIGH); //desligar o motor
    digitalWrite(y_baixo,HIGH);

    //Eixo z
    while(Pz<Pdz-20 && Modo==1){
      FC_z_frente=digitalRead(fcz_frente);
      FC_z_tras=digitalRead(fcz_tras);
      digitalWrite(z_tras,LOW); //sentido positivo - da frente para trás
      sendMQTTMessage(); // função enviar sms MQTT
      
      if(Pz != lastPz){   //Visualização - apagar depois
        Serial.print("Z: ");
        Serial.println(Pz);
        lastPz = Pz;
      }
      
      if(FC_z_frente || FC_z_tras){ //Erro
        digitalWrite(z_frente,HIGH);  //DEsliga o motor
        digitalWrite(z_tras,HIGH);
        Modo =0;  //Tenta voltar ao zero
        concluido=2;  //Indica o erro
        delay(2000);
        sendMQTTMessage();
        break;
      }
    }

    while(Pz>Pdz+20 && Modo==1){
      FC_z_frente=digitalRead(fcz_frente);
      FC_z_tras=digitalRead(fcz_tras);
      digitalWrite(z_frente,LOW); // sentido negativo - para a frente
      sendMQTTMessage(); // função enviar sms MQTT
      
      if(Pz != lastPz){ //Visualização - apagar depois
        Serial.print("Z: ");
        Serial.println(Pz);
        lastPz = Pz;
      }
      
      if(FC_z_frente || FC_z_tras){ //caso de erro
        digitalWrite(z_frente,HIGH);  //desliga o motor
        digitalWrite(z_tras,HIGH);
        Modo =0;  //volta para o zero
        concluido=2;  //mensagem de erro
        delay(2000);
        sendMQTTMessage();
        break;
      }
    }
    
    digitalWrite(z_frente,HIGH); //desliga o motor
    digitalWrite(z_tras,HIGH); 
    
    if (concluido !=2 ) { //caso não haja erro
    Modo=2;   //modo de stand-by
    concluido=1;  //mensagem de conclusão
    delay(2000);
    sendMQTTMessage();
    }
  }
} //fim do loop