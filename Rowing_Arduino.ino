#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
LiquidCrystal lcd(9, 7, 8, 12, 10, 11);
#define LED_Alerta 6 // Pin is connected to the LED ALERTA
#define LED_Direito 5 // Pin is connected to the LED DIREITO
#define LED_Esquerdo 13 // Pin is connected to the LED ESQUERDO
/**
* ponteiro de função da máquina de estados. Ele aponta sempre para a função da
* máquina de estados que deve ser executada */
bool aux_CH12 = false;
bool aux_CH34 = false;
bool config = true;
bool print;
bool run = false;
bool stim = true;
byte channels = B00000000;
byte set_channels = B00000000;
char acaba = 3;
char desce = 4;
char mode = 0;
char passo;
char sobe = 2;
int corrente_CH12 = (int) EEPROM.read(0);
int corrente_CH34 = (int) EEPROM.read(1);
int corrente_CH56 = (int) EEPROM.read(2);
int corrente_CH78 = (int) EEPROM.read(3);
int freq = (int) (EEPROM.read(5) * 5);
int pw = (int) (EEPROM.read(4) * 10);
int temp = 0;
int variavel;
int variavel_old;
String flag;
String newText1;
String newText2;
String oldText1;
String oldText2;
String text;
String unidade;
void(* PonteiroDeFuncao) ();
void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);
  Serial.flush(); // Limpar receber buffer.
  // input
  pinMode(acaba, INPUT_PULLUP);
  pinMode(desce, INPUT_PULLUP);
  pinMode(sobe, INPUT_PULLUP);
  // output
  pinMode(LED_Alerta, OUTPUT);
  pinMode(LED_Direito, OUTPUT);
  pinMode(LED_Esquerdo, OUTPUT);
}

void loop()
{
  PonteiroDeFuncao = StateConect;
  while (1)
  {
    while (config)
    {
      /**
      * chama a função apontada pelo ponteiro de função (logo, chama o estado
      * corrente)
      */
      (* PonteiroDeFuncao) ();
    }
    while (run)
    {
      int state_0 = 0;
      int state_1 = 1;
      digitalWrite(LED_Alerta, LOW);
      while (stim)
      {
        if (digitalRead(acaba) == LOW)
        {
          digitalWrite(acaba, HIGH);
          state_0 = 3;
        }
        else
          if (digitalRead(desce) == LOW)
          {
            digitalWrite(desce, HIGH);
            state_0 = 2;
            lcd.clear();
            lcd.print("Flexao");
            digitalWrite(LED_Esquerdo, HIGH);
          }
        else
          if (digitalRead(sobe) == LOW)
          {
            digitalWrite(sobe, HIGH);
            state_0 = 1;
            lcd.clear();
            lcd.print("Extensao");
            digitalWrite(LED_Direito, HIGH);
          }
        else
        {
          state_0 = 0;
        }
        if (state_1 != state_0)
        {
          Serial.print(state_0);
          state_1 = state_0;
          lcd.clear();
          lcd.print("    Pressione   ");
          lcd.setCursor(0, 1);
          lcd.print("<-Flex     Ext->");
          digitalWrite(LED_Direito, LOW);
          digitalWrite(LED_Esquerdo, LOW);
        }
        if (state_0 == 3)
        {
          Serial.print(state_0);
          lcd.clear();
          lcd.print("Fim");
          channels = B00000000;
          set_channels = B00000000;
          config = true;
          run = false;
          stim = false;
          mode = 0;
          PonteiroDeFuncao = StateSetMov;
        }
        delay(150);
      }
      delay(150);
      digitalWrite(LED_Alerta, HIGH);
      delay(150);
    }
  }
  system("PAUSE");
}

/*-------------------------------------------------------------------------*/
/**
* @brief      Estado inicial de conexão das portas
*/
void StateConect(void)
{
  newText1 = "   Conectando...";
  newText2 = " EMA FES-Rowing ";
  if ((oldText1 != newText1) | (oldText2 != newText2))
    func_print_LCD(newText1, newText2);
  delay(1000);
  /**
  * verificando conexão
  */
  bool conexao = false;
  while (conexao == false)
  {
    if (Serial.available())
    {
      flag = Serial.readString();
      if (flag.equals("a"))
      {
        conexao = true;
        PonteiroDeFuncao = StateSetMov;
      }
    }
  }
}

/**
* @brief      { Estado que seleciona a quantidade de movimento:
*             (extensão)e/ou(flexão) }
*/
void StateSetMov(void)
{
  newText1 = "Sel.Movimento(s):";
  if ((oldText1 != newText1) | (oldText2 != newText2))
    func_print_LCD(newText1, "");
  variavel = mode;
  passo = 1;
  unidade = "";
  print = false;
  mode = func_set_parametro(variavel, passo, text, unidade, print);
  if (mode == 0)
    mode = 3;
  if (mode > 3)
    mode = 1;
  if (mode == 1)
  {
    newText2 = "Extensao       ";
    if ((oldText1 != newText1) | (oldText2 != newText2))
      func_print_LCD(oldText1, newText2);
    channels = B00000011;
  }
  if (mode == 2)
  {
    newText2 = "         flexao";
    if ((oldText1 != newText1) | (oldText2 != newText2))
      func_print_LCD(oldText1, newText2);
    channels = B00001100;
  }
  if (mode == 3)
  {
    newText2 = "Extensao+flexao";
    if ((oldText1 != newText1) | (oldText2 != newText2))
      func_print_LCD(oldText1, newText2);
    channels = B00001111;
  }
  delay(150);
  if (digitalRead(acaba) == LOW)
  {
    if (mode == 2)
      PonteiroDeFuncao = SetAuxCH34;
    else
      PonteiroDeFuncao = SetAuxCH12;
    delay(250);
  }
}

/**
* @brief      { Estado para selecionar se o canal 12 (CH12) necessita de canal
*             auxiliar, se sim: o canal auxíliar será o CH56 }
*/
void SetAuxCH12(void)
{
  passo = 1;
  unidade = "";
  variavel = ((int) aux_CH12) + 128;
  variavel_old = variavel;
  print = false;
  variavel = func_set_parametro(variavel, passo, text, unidade, print);
  if (variavel != variavel_old)
    aux_CH12 = !aux_CH12;
  if (aux_CH12 == false)
  {
    newText2 = "Nao             ";
    set_channels = B00000000;
  }
  if (aux_CH12 == true)
  {
    newText2 = "Sim, CH 5/6     ";
    set_channels = B00110000;
  }
  newText1 = "Ext.c/ Auxiliar?";
  if ((oldText1 != newText1) | (oldText2 != newText2))
    func_print_LCD(newText1, newText2);
  if (digitalRead(acaba) == LOW)
  {
    channels = channels | set_channels;
    if (mode == 1)
      PonteiroDeFuncao = StateSetAmp;
    else
      PonteiroDeFuncao = SetAuxCH34;
    delay(250);
  }
}

/**
* @brief      { Estado para selecionar se o canal 12 (CH12) necessita de canal
*             auxiliar, se sim: o canal auxíliar será o CH56 }
*/
void SetAuxCH34(void)
{
  passo = 1;
  unidade = "";
  variavel = ((int) aux_CH34) + 128;
  variavel_old = variavel;
  print = false;
  variavel = func_set_parametro(variavel, passo, text, unidade, print);
  if (variavel != variavel_old)
    aux_CH34 = !aux_CH34;
  if (aux_CH34 == false)
  {
    newText2 = "Nao             ";
    set_channels = B00000000;
  }
  if (aux_CH34 == true)
  {
    newText2 = "Sim, CH 7/8     ";
    set_channels = B11000000;
  }
  newText1 = "Flex.c/ Auxiliar?";
  if ((oldText1 != newText1) | (oldText2 != newText2))
    func_print_LCD(newText1, newText2);
  if (digitalRead(acaba) == LOW)
  {
    channels = channels | set_channels;
    PonteiroDeFuncao = StateSetAmp;
    digitalWrite(acaba, HIGH);
    delay(250);
  }
}

/**
* @brief      Estado para selecionar as amplitudes de todos os canais
*             selecionados anteriormente
*/
void StateSetAmp(void)
{
  if ((B00000011 & channels) > 0)
  {
    while (digitalRead(acaba) == HIGH)
    {
      text = "CorrenteExtensao";
      variavel = corrente_CH12;
      passo = 2;
      unidade = "mA (CH [1,2])";
      print = true;
      corrente_CH12 = func_set_parametro(variavel, passo, text, unidade, print);
    }
    digitalWrite(acaba, HIGH);
    delay(250);
  }
  if ((B00110000 & channels) > 0)
  {
    while (digitalRead(acaba) == HIGH)
    {
      text = "Corrente Aux Ext";
      variavel = corrente_CH56;
      passo = 2;
      unidade = "mA (CH [5,6])";
      print = true;
      corrente_CH56 = func_set_parametro(variavel, passo, text, unidade, print);
    }
    digitalWrite(acaba, HIGH);
    delay(250);
  }
  if ((B00001100 & channels) > 0)
  {
    while (digitalRead(acaba) == HIGH)
    {
      text = "Corrente Flexao";
      variavel = corrente_CH34;
      passo = 2;
      unidade = "mA (CH [3,4])";
      print = true;
      corrente_CH34 = func_set_parametro(variavel, passo, text, unidade, print);
    }
    digitalWrite(acaba, HIGH);
    delay(250);
  }
  if ((B11000000 & channels) > 0)
  {
    while (digitalRead(acaba) == HIGH)
    {
      text = "Corrente AuxFlex";
      variavel = corrente_CH78;
      passo = 2;
      unidade = "mA (CH [7,8])";
      print = true;
      corrente_CH78 = func_set_parametro(variavel, passo, text, unidade, print);
    }
    digitalWrite(acaba, HIGH);
    delay(250);
  }
  PonteiroDeFuncao = StateSetLP;
}

/**
* @brief      Estado para selecionar a Largura de Pulso (única para todos os
*             canais)
*/
void StateSetLP(void)
{
  text = "Largura de Pulso:";
  variavel = pw;
  passo = 10;
  unidade = "micro seg";
  print = true;
  pw = func_set_parametro(variavel, passo, text, unidade, print);
  if (digitalRead(acaba) == LOW)
  {
    PonteiroDeFuncao = StateSetFreq;
    digitalWrite(acaba, HIGH);
    delay(250);
  }
}

/**
* @brief      Estado para selecionar a Frequencia (única para todos os canais)
*/
void StateSetFreq(void)
{
  text = "Frequencia:";
  variavel = freq;
  passo = 5;
  unidade = "Hz";
  print = true;
  freq = func_set_parametro(variavel, passo, text, unidade, print);
  if (digitalRead(acaba) == LOW)
  {
    PonteiroDeFuncao = Send;
    digitalWrite(acaba, HIGH);
    delay(250);
  }
}

/**
* @brief      Estado para enviar os parâmetros para o Raspberry
*/
void Send(void)
{
  // Estado Inicial
  if (B00000000 == channels)
    mode = 0;
  // Extensão
  if (B00000011 == channels)
    mode = 1;
  // Flexão
  if (B00001100 == channels)
    mode = 2;
  // Extensão + Flexão
  if (B00001111 == channels)
    mode = 3;
  // (Extensão & Aux_Ext)
  if (B00110011 == channels)
    mode = 4;
  // (Extensão & Aux_Ext) + Flexão
  if (B00111111 == channels)
    mode = 5;
  // (Flexão & Aux_Flex)
  if (B11001100 == channels)
    mode = 6;
  // Extensao + (Flexão & Aux_Flex)
  if (B11001111 == channels)
    mode = 7;
  // (Extensão & Aux_Ext) + (Flexão & Aux_Flex)
  if (B11111111 == channels)
    mode = 8;
  // enviando dados pela serial
  Serial.print("c"); // marcador de corrente ch12
  func_dim_string_to_Stim(corrente_CH12);
  Serial.print("d"); // marcador de corrente ch34
  func_dim_string_to_Stim(corrente_CH34);
  Serial.print("e"); // marcador de corrente ch56
  func_dim_string_to_Stim(corrente_CH56);
  Serial.print("x"); // marcador de corrente ch78
  func_dim_string_to_Stim(corrente_CH78);
  Serial.print("p"); // marcador de largura de pulso
  func_dim_string_to_Stim(pw);
  Serial.print("f"); // marcador de frequecia
  func_dim_string_to_Stim(freq);
  Serial.print("m"); // marcador do modo
  func_dim_string_to_Stim(mode);
  lcd.clear();
  lcd.print("Enviando");
  delay(2000);
  PonteiroDeFuncao = waiting;
  lcd.setCursor(0, 1);
  lcd.print(channels, BIN);
  EEPROM.write(0,(byte) corrente_CH12);
  EEPROM.write(1,(byte) corrente_CH34);
  EEPROM.write(2,(byte) corrente_CH56);
  EEPROM.write(3,(byte) corrente_CH78);
  EEPROM.write(4,(byte) (pw/10));
  EEPROM.write(5,(byte) (freq) /5);
}

/**
* @brief      Estado para aguardar a confirmação de recebimento pelo Raspberry
*/
void waiting(void)
{
  if (Serial.available())
  {
    flag = Serial.readString();
    if (flag.equals("a"))
    {
      newText1 = "Iniciando estimulacao";
      if ((oldText1 != newText1) | (oldText2 != newText2))
        func_print_LCD(newText1, "");
      PonteiroDeFuncao = Runing;
    }
  }
}

/**
* @brief      Estado para iniciar o modo Run no loop()
*/
void Runing(void)
{
  run = true;
  config = false;
}

/**
* @brief      Função para acrescentar/diminuir valores da variavel, conforme o
*             passo escolhido, através dos botões do Handle
*
* @param      variavel  Define qual é a varialvel a ser alterada
* @param      passo     Define o passo de acrescimo/decrescimo
* @param      text      Texto informado no display
* @param      unidade   Unidade de medida informada no display
* @param      print     Indica se imprime as mensagens no display (true/false)
*
* @return     Retorna o valor da variavel definida através dos botções
*/
int func_set_parametro(int variavel, int passo, String text, String unidade, bool print)
{
  int BTN_Down = HIGH;
  int BTN_Up = HIGH;
  int lastReadingDown = HIGH;
  int lastReadingUp = HIGH;
  long lastSwitchTimeDown = 0;
  long lastSwitchTimeUp = 0;
  long longSwitchTime = 500;
  long shortSwitchTime = 100;
  long switchTimeDown = 500;
  long switchTimeUp = 500;
  BTN_Up = digitalRead(sobe);
  BTN_Down = digitalRead(desce);
  if (print == true)
  {
    newText1 = text;
    newText2 = String(variavel);
    if ((newText1 != oldText1) | (newText2 != oldText2))
    {
      func_print_LCD(newText1, newText2);
      lcd.print(" ");
      lcd.print(unidade);
      lcd.print("   ");
    }
  }
  if (BTN_Up == LOW && ((((millis() - lastSwitchTimeUp) > switchTimeUp) && lastSwitchTimeUp != 0) || lastReadingUp == HIGH))
  {
    if (((millis() - lastSwitchTimeUp) > switchTimeUp) && lastSwitchTimeUp != 0)
    {
      switchTimeUp = shortSwitchTime;
    }
    lastSwitchTimeUp = millis();
    lastReadingUp = BTN_Up;
    variavel = variavel + passo;
    delay(500);
  }
  if (BTN_Up == HIGH)
  {
    lastReadingUp = BTN_Up;
    switchTimeUp = longSwitchTime;
    lastSwitchTimeUp = 0;
  }
  if (BTN_Down == LOW &&
  ((((millis() - lastSwitchTimeDown) > switchTimeDown) && lastSwitchTimeDown != 0) || lastReadingDown == HIGH))
  {
    if (((millis() - lastSwitchTimeDown) > switchTimeDown) && lastSwitchTimeDown != 0)
    {
      switchTimeDown = shortSwitchTime;
    }
    lastSwitchTimeDown = millis();
    lastReadingDown = BTN_Down;
    variavel = variavel - passo;
    if (variavel < 0)
    {
      variavel = 0;
    }
    delay(500);
  }
  if (BTN_Down == HIGH)
  {
    lastReadingDown = BTN_Down;
    switchTimeDown = longSwitchTime;
    lastSwitchTimeDown = 0;
  }
  return variavel;
}

/*-------------------------------------------------------------------------*/
/**
* @brief      Configura o envio de parametros sempre com 3 algarismo
*
* @param      entrada  Valor a ser convertido para 3 algarismo
*
* @return     Valor convertido com 3 algarismo
*/
void func_dim_string_to_Stim(int entrada)
{
  if (qtdAlgarismos(entrada) == 3)
  {
    Serial.print(entrada);
  }
  else
    if (qtdAlgarismos(entrada) == 2)
    {
      Serial.print(0);
      Serial.print(entrada);
    }
  else
    if (qtdAlgarismos(entrada) == 1)
    {
      Serial.print(0);
      Serial.print(0);
      Serial.print(entrada);
    }
}

/*-------------------------------------------------------------------------*/
/**
* @brief      Conta a quantidade de Algarismo
*
* @param      numero  Entrada a ser avaliada
*
* @return     Quatidade de algarismo
*/
int qtdAlgarismos(int numero)
{
  int cont = 0;
  while (numero != 0)
  {
    numero /= 10;
    cont++;
  }
  return cont;
}

void func_print_LCD(String linha1, String linha2)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(linha1);
  lcd.setCursor(0, 1);
  lcd.print(linha2);
  oldText1 = linha1;
  oldText2 = linha2;
}
