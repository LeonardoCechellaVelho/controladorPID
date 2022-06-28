#include <LiquidCrystal.h>

/* LCD */
const int rs = 12, en = 11, d4 = 6, d5 = 4, d6 = 9, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/* Sistema Temperatura */
const byte heater = 3, cooler = 5, tempPin = A0;
float valorADTemp;
float uProp, uInt, uDeriv, uErro, PID, kd = 0.1, temp = 0, tempReferencia = 27.5;
int setPointCalculo = 40, kp = 30, ki = 3;

/* Controle do menu */
const byte botaoPin = A1;
bool editando = false, mudandoValor = false, condicaoCooler = false, menuPid = false;
int posicao = 1;
long ultimoProcesso = 0;

/* Filtro passa-baixa */
float y, x = 56, valorADFiltrado;

void setup() {
  /* Definir tamanho do LCD "20x4" */
  lcd.begin(20, 4);

  /* Portas do sensor de temperatura */
  pinMode(heater,OUTPUT);
  pinMode(cooler,OUTPUT);
  
  Serial.begin(9600);
}

void realizarCalculo() {
  valorADTemp = analogRead(tempPin);

  /* Aplicar Filtro Passa Baixa no valor AD da temperatura */
  valorADFiltrado = filtroPassaBaixa(valorADTemp);

  /*
   * Conversão do valor retornado do sensor de temperatura (LM35)
   * V = 0.01V
   * Conversão AD = 5V / 1024
   */
  temp = valorADFiltrado*5/(0.01*1024);

  uErro = (setPointCalculo-temp);
  
  uProp = kp * uErro;
  uInt += (uErro * ki) * 0.1;
  uDeriv = ((tempReferencia - temp) * kd) / 0.1;
  
  tempReferencia = temp;
  
  PID = (uProp + controleMaximoMinimo(uInt) + uDeriv);
  PID = controleMaximoMinimo(PID);

  analogWrite(heater, PID);

  /* Alterar entre o menus de acordo com o valor da variável menuPid */
  if (!menuPid) {
    defineMenuPID(); 
  } else {
    defineMenuDERIV();
  }
}

int controleMaximoMinimo(int valor) {
  if (valor < 0){
      valor = 0;
  }
  if (valor > 255){
      valor = 255;
  }
  return valor;
}

/* Filtro passa baixa
 * Frequência de corte: 1 Hz
 * Frequência de amostragem: 10 Hz (Intervalo de amostragem de 0,1 s)
 */
float filtroPassaBaixa(float temperatura){
   /* Função de transferência calculada pelo site http://sim.okawa-denshi.jp/en/CRtool.php
    * G(s) = 6.25 / (s + 6.25)
    * Equação de transferência convertida para espaço de estado no dominio discreto
    * x(k + 1) = Adx(k) + Bdu(k)
    * Ad = (1 - 6.25 * 0.1) = 0.375
    * Bd = 6.25 * 0.1 = 0,625
    */
   y = x;
   x = 0.375 * x + 0.625 * temperatura;
   return y;
}

void selecionaMenu() {
  unsigned int valorBotao = analogRead(botaoPin);

  /* Iniciar edição de variáveis */
  if (!menuPid) {
    if ((valorBotao >= 0) && (valorBotao < 50)) {
      editando = !editando;
      posicao = 1;
      if (editando) {
        mudaPosicao(1);
      } else {
        mudaPosicao(0);
      }
    }
    if (editando) {
      if ((valorBotao > 73) && (valorBotao < 173)) {
        /* Alterar para o item da esquerda */
        if (mudandoValor) {
          diminuiValor(posicao);
        } else if (posicao != 1) {
          posicao = posicao - 1;
          mudaPosicao(posicao);
        }
      }
      else if ((valorBotao > 196) && (valorBotao < 296)) {
        /* Alterar para o item da direita */
        if (mudandoValor) {
          aumentaValor(posicao);
        } else if (posicao != 4) {
          posicao = posicao + 1;
          mudaPosicao(posicao);
        }
      }
      else if ((valorBotao > 339) && (valorBotao < 439)) {
        /* Seleciona e confirma a edição */
        mudandoValor = !mudandoValor;
        mudaPosicao(posicao);
      }
    }
  }

  if ((valorBotao > 462) && (valorBotao < 562)) {
    /* Ligar cooler para simular perturbação no sistema */
    condicaoCooler = !condicaoCooler;
    ligaDeslCooler(condicaoCooler);
  }

  if ((valorBotao > 584) && (valorBotao < 684)) {
    /* Variável auxiliar para alternar entre o menu com os valores do PID e o menu de temperatura */
    lcd.clear();
    menuPid = !menuPid;
    editando = false;
    posicao = 1;
  }
}

void ligaDeslCooler(bool condicaoCooler) {
  if (condicaoCooler) {
    analogWrite(cooler, 255);
  } else {
    analogWrite(cooler, 0);
  }
}

void aumentaValor(int posicao) {
  switch(posicao) {
    case 1:
    setPointCalculo = setPointCalculo + 1;
    break;

    case 2:
    ki = ki + 1;
    break;

    case 3:
    kp = kp + 1;
    break;

    case 4:
    kd = kd + 0.1;
    break;
  }
}

void diminuiValor(int posicao) {
  switch(posicao) {
    case 1:
    setPointCalculo = setPointCalculo - 1;
    break;

    case 2:
    ki = ki - 1;
    break;

    case 3:
    kp = kp - 1;
    break;

    case 4:
    kd = kd - 0.1;
    break;
  }
}

void mudaPosicao(int posicao) {
  int xSup = 7;
  int xInf = 19;
  switch(posicao) {
    case 1:
      if (mudandoValor) {
        lcd.setCursor(xSup-1, 0);
        lcd.print("<");
        lcd.setCursor(xSup, 0);
        lcd.print(" ");
      } else {
        lcd.setCursor(xSup-1, 0);
        lcd.print(" ");
        lcd.setCursor(xSup, 0);
        lcd.print("<");
      }
      lcd.setCursor(xInf, 0);
      lcd.print(" ");
      lcd.setCursor(xSup, 1);
      lcd.print(" ");
      lcd.setCursor(xInf, 1);
      lcd.print(" ");
    break;

    case 2:
      if (mudandoValor) {
        lcd.setCursor(xInf-1, 0);
        lcd.print("<");
        lcd.setCursor(xInf, 0);
        lcd.print(" ");
      } else {
        lcd.setCursor(xInf-1, 0);
        lcd.print(" ");
        lcd.setCursor(xInf, 0);
        lcd.print("<");
      }
      lcd.setCursor(xSup, 0);
      lcd.print(" ");
      lcd.setCursor(xSup, 1);
      lcd.print(" ");
      lcd.setCursor(xInf, 1);
      lcd.print(" ");
    break;

    case 3:
      if (mudandoValor) {
        lcd.setCursor(xSup-1, 1);
        lcd.print("<");
        lcd.setCursor(xSup, 1);
        lcd.print(" ");
      } else {
        lcd.setCursor(xSup-1, 1);
        lcd.print(" ");
        lcd.setCursor(xSup, 1);
        lcd.print("<");
      }
      lcd.setCursor(xSup, 0);
      lcd.print(" ");
      lcd.setCursor(xInf, 0);
      lcd.print(" ");
      lcd.setCursor(xInf, 1);
      lcd.print(" ");
    break;

    case 4:
      if (mudandoValor) {
        lcd.setCursor(xInf-1, 1);
        lcd.print("<");
        lcd.setCursor(xInf, 1);
        lcd.print(" ");
      } else {
        lcd.setCursor(xInf-1, 1);
        lcd.print(" ");
        lcd.setCursor(xInf, 1);
        lcd.print("<");
      }
      lcd.setCursor(xSup, 0);
      lcd.print(" ");
      lcd.setCursor(xInf, 0);
      lcd.print(" ");
      lcd.setCursor(xSup, 1);
      lcd.print(" ");
    break;

    case 0:
      lcd.setCursor(6, 0);
      lcd.print(" ");
      lcd.setCursor(7, 0);
      lcd.print(" ");
      lcd.setCursor(18, 0);
      lcd.print(" ");
      lcd.setCursor(19, 0);
      lcd.print(" ");
      lcd.setCursor(6, 1);
      lcd.print(" ");
      lcd.setCursor(7, 1);
      lcd.print(" ");
      lcd.setCursor(18, 1);
      lcd.print(" ");
      lcd.setCursor(19, 1);
      lcd.print(" ");
    break;
  }
}

void defineMenuDERIV() {
  lcd.setCursor(0, 0);
  lcd.print("uProp: ");
  lcd.setCursor(7, 0);
  lcd.print(uProp);
  lcd.setCursor(0, 1);
  lcd.print("uInt: ");
  lcd.setCursor(6, 1);
  lcd.print(uInt);
  lcd.setCursor(0, 2);
  lcd.print("uDeriv: ");
  lcd.setCursor(7, 2);
  lcd.print(uDeriv);
  lcd.print("   ");
  lcd.setCursor(0, 3);
  lcd.print("PID: ");
  lcd.setCursor(5, 3);
  lcd.print(PID);
  lcd.setCursor(14, 3);
  lcd.print("TP:");
  lcd.setCursor(17, 3);
  lcd.print(setPointCalculo);
}

void defineMenuPID() {
  lcd.setCursor(0, 0);
  lcd.print("TP:");
  lcd.setCursor(3, 0);
  lcd.print(setPointCalculo);
  lcd.setCursor(5, 0);
  lcd.print("C");
  lcd.setCursor(10, 0);
  lcd.print("KI:");
  lcd.setCursor(13, 0);
  lcd.print(ki);
  lcd.setCursor(0, 1);
  lcd.print("KP:");
  lcd.setCursor(3, 1);
  lcd.print(kp);
  lcd.setCursor(10, 1);
  lcd.print("KD:");
  lcd.setCursor(13, 1);
  lcd.print(kd);

  lcd.setCursor(0, 2);
  lcd.print("AD:");
  lcd.setCursor(3, 2);
  lcd.print(valorADTemp);

  lcd.setCursor(0, 3);
  lcd.print("TP ATUAL:");
  lcd.setCursor(9, 3);
  lcd.print(temp);
  lcd.setCursor(14, 3);
  lcd.print("C");
}

void loop() {  
  realizarCalculo();
  selecionaMenu();
  delay(100);
}
