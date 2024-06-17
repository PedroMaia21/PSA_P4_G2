int z0 = 32; // Pino de saída para o controlo do motor 
int z1 = 33; // Pino de saída para o controlo do motor 
int ENC_Az = 22; // Pino de entrada do encoder A eixo Z
int ENC_Bz = 23; // Pino de entrada do encoder B eixo Z
int in_opto2_3 = 25; // Pino de entrada do sensor optoacoplador 3
int in_opto2_4 = 26; // Pino de entrada do sensor optoacoplador 4
int in_opto2_5 = 27; // Pino de entrada do sensor optoacoplador 5
int frente = 14; // Pino de entrada para o botão "frente"
int tras = 15; // Pino de entrada para o botão "tras"
int zero = 18; // Pino de entrada para o botão "zero"
int dir = 0; // Variável para armazenar a direção do movimento
int maxpz = 0; // Variável para armazenar a posição máxima positiva
int maxnz = 0; // Variável para armazenar a posição máxima negativa
int Pz = 0; // Variável para armazenar a posição atual do encoder Z

// Função de interrupção para o encoder Z
void isr_z() {
  if (digitalRead(ENC_Bz)) { // Se o pino B do encoder Z estiver em HIGH
    Pz++; // Incrementa a posição do encoder Z
  } else {
    Pz--; // Decrementa a posição do encoder Z
  }
}

void setup() {
  // Configuração dos pinos como entradas ou saídas
  pinMode(frente, INPUT); // Configura o pino do botão "frente" como entrada
  pinMode(tras, INPUT); // Configura o pino do botão "tras" como entrada
  pinMode(in_opto2_3, INPUT); // Configura o pino do sensor optoacoplador 3 como entrada
  pinMode(in_opto2_4, INPUT); // Configura o pino do sensor optoacoplador 4 como entrada
  pinMode(in_opto2_5, INPUT); // Configura o pino do sensor optoacoplador 5 como entrada
  pinMode(ENC_Az, INPUT); // Configura o pino do encoder A eixo Z como entrada
  pinMode(ENC_Bz, INPUT); // Configura o pino do encoder B eixo Z como entrada
  pinMode(z0, OUTPUT); // Configura o pino do motor Z0 como saída
  pinMode(z1, OUTPUT); // Configura o pino do motor Z1 como saída

  // Inicializa os motores desligados
  digitalWrite(z0, HIGH);
  digitalWrite(z1, HIGH);

  // Configura a interrupção para o encoder Z
  attachInterrupt(digitalPinToInterrupt(ENC_Az), isr_z, RISING);
  Serial.begin(9600); 

void loop() {
  // Se o botão "frente" for pressionado
  if (digitalRead(frente)) {
    digitalWrite(z0, LOW); // Liga o pino Z0
    dir = 1; // Define a direção como positiva
    // Continua movendo-se até que um dos sensores optoacopladores 3 ou 4 sejam acionados
    while (!(digitalRead(in_opto2_3) || digitalRead(in_opto2_4))) {
      Serial.println(Pz); // Imprime a posição atual do encoder Z
    }
    maxpz = Pz; // Armazena a posição máxima positiva
    digitalWrite(z0, HIGH); // Desliga o pino Z0
  }

  delay(1000); // Espera 1 segundo

  // Se o botão "tras" for pressionado
  if (digitalRead(tras)) {
    digitalWrite(z1, LOW); // Liga o pino Z1
    dir = 0; // Define a direção como negativa
    // Continua movendo-se até que um dos sensores optoacopladores 3 ou 4 sejam acionados
    while (!(digitalRead(in_opto2_3) || digitalRead(in_opto2_4))) {
      Serial.println(Pz); // Imprime a posição atual do encoder Z
    }
    maxnz = Pz; // Armazena a posição máxima negativa
    digitalWrite(z1, HIGH); // Desliga o pino Z1
  }

  // Se o botão "zero" for pressionado
  if (digitalRead(zero)) {
    if (dir == 1) {
      digitalWrite(z1, LOW); // Liga o pino Z1 se a direção for positiva
    } else {
      digitalWrite(z0, LOW); // Liga o pino Z0 se a direção for negativa
    }
    // Continua movendo até que o sensor optoacoplador 5 seja acionado
    while (digitalRead(in_opto2_5)) {
      Serial.println(Pz); // Imprime a posição atual do encoder Z
    }
    // Desliga ambos os motores
    digitalWrite(z0, HIGH);
    digitalWrite(z1, HIGH);
  }

  delay(1000); // Espera 1 segundo
}
