/*---------------------------------------
Necessário para a conecção ao WiFi e ao MQTT
---------------------------------------*/
#include <WiFi.h> //WiFi.h é usado para conectar o ESP32 a uma rede Wi-Fi 
#include <PubSubClient.h> //PubSubClient.h é usada para comunicação MQTT 
/*---------------------------------------
Conecção ao WiFi 
---------------------------------------*/
const char *SSID = "iPhone de Inês"; //nome da rede wifi
const char *PWD = "santosines"; // palavra-passe da rede wifi
/*---------------------------------------
Conecção ao Mqtt Server
---------------------------------------*/
WiFiClient wifiClient; //Estas variáveis estão relacionadas à configuração do cliente MQTT. mqttClient é o cliente MQTT que se conecta ao servidor MQTT 
//definido por mqttServer e mqttPort.
PubSubClient mqttClient(wifiClient); 
char *mqttServer = "193.137.172.20"; 
int mqttPort = 85;
long lastMsg = 0; // variável do tipo long - usada para representar números inteiros longos (nesta variável armazena-se o momomento em que a última mensagem foi enviada)
const long interval = 5000; // o const antes do long indica que o valor desta variável não pode ser alterado depois de definido;
/*---------------------------------------
Variavés nevessárias para a parte do MQTT Server
---------------------------------------*/
long last_time=0; //variáveis são usadas para armazenar informações como a última vez que o código foi executado, dados recebidos via MQTT
char data[100];
String Command; 
int Pdy; 
int Pdz;
int Pz=1000; // valores aletários usados para testar o código
int Py=20000; // valores aletários usados para testar o código
int concluido=0;
/*---------------------------------------
Função criada para a conecção ao WiFi: conecção  do ESP32 à rede Wi-Fi especificada pelos SSID e senha
---------------------------------------*/
void connectToWiFi() {
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
/*---------------------------------------
Função criada para a conecção ao MQTT Server: Esta função configura o cliente MQTT para se conectar ao servidor MQTT especificado e define a função de 
\\retorno de chamada (callback) que será chamada quando mensagens forem recebidas.
---------------------------------------*/
void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
  // set the callback function
  mqttClient.setCallback(callback);
}
/*---------------------------------------
Função criada para a reconecção ao MQTT Server: Esta função é chamada para reconectar o cliente MQTT se a conexão for perdida. Ele gera um ID de cliente 
//aleatório e se conecta ao servidor MQTT. Em seguida, ele se inscreve em tópicos específicos para receber comandos e dados de sensores.
---------------------------------------*/
void reconnect() {
  while (!mqttClient.connected()) {
      String clientId = "ESP32Client-"; //criação de um iD aleatório que muda de cada vez que é necessário conectar.
      clientId += String(random(0xffff), HEX);
      if (mqttClient.connect(clientId.c_str())) {
        // subscribe to topic
        mqttClient.subscribe("PSA24G2_yzCommand");
      }   
  }
}
/*---------------------------------------
Função criada para a "chamda" para o MQTT server: Esta função é chamada sempre que uma mensagem MQTT é recebida. Ele analisa o tópico da mensagem e atualiza as variáveis 
\\ com os dados recebidos.
---------------------------------------*/
void callback(char* topic, byte* payload, unsigned int length) { //payload é o que vai dentro da mensagem que o ESP recebe do MQTT Server
  String message; //unsigned int length serve para indicar o comprimento do payload, ou seja, o número de bytes de dados que foram recebidos 
  for (int i = 0; i < length; i++) {                           // i serve para percorrer cada byte do payload e processá-lo durante a construção da mensagem MQTT.
    message += (char)payload[i];                                 // A função strcmp é usada para comparar duas strings (Se as duas strings forem iguais, strcmp é 0.  
  }                                  //Se a primeira string for maior que a segunda, é um valor positivo. Se a segunda string for maior que a primeira, é um valor negativo.)
  if (strcmp(topic,"PSA24G2_yzCommand")==0){
      Command=message; 
  }


   //separação da informação dos eixos  y e z 
  // 1º Passo: Encontrar os índices dos separadores
    int M_Index = Command.indexOf('M');
    int Y_Index = Command.indexOf('Y');
    int Z_Index = Command.indexOf('Z');

  // 2º Passo: Extrair as partes das mensagens
  String yPart = Command.substring(Y_Index + 1, Y_Index+6);
  String zPart = Command.substring(Z_Index + 1, Z_Index+6);

  //3ºPasso: Coverter valor para interiro 
  Pdy = yPart.toInt();
  Pdz = zPart.toInt();
  //"Imprimir os valores de Pdy e Pdz"  
  Serial.print("yPart: ");
  Serial.println(Pdy);
  Serial.print("zPart: ");
  Serial.println(Pdz);
}

//---------------------------------------*/
void setup() { //a conexão Wi-Fi e o cliente MQTT são configurados, e os pinos dos componentes são inicializados.
  // put your setup code here, to run once:
  Serial.begin(9600);
  connectToWiFi(); //start the wiFi connection
  setupMQTT(); //start the MQTT connection
}

void loop() {
/*---------------------------------------
Esta parte serve para confirmar se o ESP ainda está conectado ao MQTT Server
---------------------------------------*/
  if (!mqttClient.connected())
  reconnect();
  mqttClient.loop();
  /*---------------------------------------
  Esta parte permite publicar a mensagem no MQTT Server a cada 5 segundos
  ---------------------------------------*/
  long now = millis();
  if (now - lastMsg > interval) {
  lastMsg = now;
  sprintf(data, "C%iY%iZ%i",concluido,Py,Pz);
  mqttClient.publish("PSA24G2_yzFeedback",data); 
  }
}


