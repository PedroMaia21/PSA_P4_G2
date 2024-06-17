int ENC_Ay = 16; // Pino de entrada do encoder A eixo Y
int ENC_By = 17; // Pino de entrada do encoder B eixo Y
int in_opto2_3 = 18; // Pino de entrada do sensor optoacoplador 3
int in_opto2_4 = 19; // Pino de entrada do sensor optoacoplador 4
int in_opto2_5 = 21; // Pino de entrada do sensor optoacoplador 5
int frente = 14; // Pino de entrada para o botão "frente"
int tras = 15; // Pino de entrada para o botão "tras"
int PY = 0; // Variável para armazenar a posição atual do encoder Y

// Função de interrupção para o encoder Y
void isr_y() {
  if (digitalRead(ENC_By)) { // Se o pino B do encoder Y estiver em HIGH
    PY++; // Incrementa a posição do encoder Y
  } else {
    PY--; // Decrementa a posição do encoder Y
  }
}

void setup() {
  // Configuração dos pinos como entradas ou saídas
  pinMode(in_opto2_5, INPUT); // Configura o pino do sensor optoacoplador 5 como entrada
  pinMode(tras, INPUT); // Configura o pino do botão "tras" como entrada
  pinMode(in_opto2_3, INPUT); // Configura o pino do sensor optoacoplador 3 como entrada
  pinMode(ENC_Ay, INPUT); // Configura o pino do encoder A eixo Y como entrada
  pinMode(ENC_By, INPUT); // Configura o pino do encoder B eixo Y como entrada
  pinMode(in_opto2_4, INPUT); // Configura o pino do sensor optoacoplador 4 como entrada
  pinMode(in_opto2_5, INPUT); // Configura o pino do sensor optoacoplador 5 como entrada
  pinMode(13, OUTPUT); // Configura o pino 13 como saída
  pinMode(4, OUTPUT); // Configura o pino 4 como saída
  pinMode(frente, INPUT); // Configura o pino do botão "frente" como entrada

  // Inicializa as saídas em estado HIGH (desligadas)
  digitalWrite(13, HIGH);
  digitalWrite(4, HIGH);

  // Configura a interrupção para o encoder Y
  attachInterrupt(digitalPinToInterrupt(ENC_Ay), isr_y, RISING);
  Serial.begin(9600); 
}

  void loop() {
  // Se o botão "frente" for pressionado
  if (digitalRead(frente)) {
    digitalWrite(13, LOW); // Liga a saída 13
  } else {
    digitalWrite(13, HIGH); // Desliga a saída 13
  }

  // Se o botão "tras" for pressionado
  if (digitalRead(tras)) {
    digitalWrite(4, LOW); // Liga a saída 4
  } else {
    digitalWrite(4, HIGH); // Desliga a saída 4
  }
}
