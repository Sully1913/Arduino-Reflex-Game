#include <Wire.h>
#include "PCF8574.h"
#include <LedControl.h>

//Konfiguracja wyświetlacza i pinów obsługujących go
// DIN=8, CLK=12, CS=10
LedControl lc = LedControl(8, 12, 10, 1); 

PCF8574 pcf(0x20); 

// Piny diody
const int ledPins[] = {5, 7, 2, 4, 6, 3}; 

// Piny przycisków
const int pcfPins[] = {0, 1, 2, 3, 5, 6}; 

const int startButton = 9;
const int modePins[] = {A0, A1, A2, A3}; 

const int pedalBrightness = 40; 

// Zmienne gry
bool gameRunning = false;
int selectedMode = 1; // Domyślnie F1

int score = 0;
int currentTarget = -1;

unsigned long startTime;
const unsigned long gameDuration = 31000;
const int maxAttempts = 10;

// Zmienne do pomiaru refleksu
unsigned long reactionStartTime = 0;
unsigned long totalReactionTime = 0;
int validHits = 0; 

// Zmienne wyświetlacza
int lastDisplayedValue = -1;
int lastDisplayedScore = -1;

void setup() {
  pinMode(10, OUTPUT); digitalWrite(10, HIGH); 
  pinMode(12, OUTPUT); digitalWrite(12, LOW);
  pinMode(8, OUTPUT);  digitalWrite(8, LOW);
  delay(500); 

  Serial.begin(9600);
  Serial.println("--- SYSTEM GOTOWY ---");

  // I2C
  Wire.begin();
  Wire.setClock(100000);
  
  //budzenie ekranu
  for(int i = 0; i < 3; i++) {
      lc.shutdown(0, false);   
      delay(50);
      lc.setIntensity(0, 8);   
      delay(50);
      lc.clearDisplay(0);      
      delay(50);
  }
  testDisplayOrder(); 

  for (int i = 0; i < 6; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  
  pinMode(startButton, INPUT_PULLUP);
  for(int i=0; i<4; i++) pinMode(modePins[i], INPUT_PULLUP);

  if(pcf.begin()){
    Serial.println("Ekspander OK");
  } else {
    Serial.println("BLAD Ekspandera!");
  }

  for (int i = 0; i < 6; i++) pcf.write(pcfPins[i], HIGH);
  
  displayModeMenu(selectedMode);
  randomSeed(analogRead(A5));
}

void loop() {
  if (!gameRunning) {
    // MENU
    if (digitalRead(modePins[0]) == LOW) { changeMode(1); }
    if (digitalRead(modePins[1]) == LOW) { changeMode(2); }
    if (digitalRead(modePins[2]) == LOW) { changeMode(3); }
    if (digitalRead(modePins[3]) == LOW) { changeMode(4); }

    if (digitalRead(startButton) == LOW) {
      startGame();
    }
  } else {

    // Obsługa przycisku STOP
    if (digitalRead(startButton) == LOW) {
       delay(50); 
       endGame();
       return; 
    }

    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;

    // logika końca gry
    bool isGameOver = false;
    int displayValue = 0; 

    if (selectedMode == 1 || selectedMode == 2) {
        if (elapsedTime >= gameDuration) isGameOver = true;
        displayValue = (gameDuration - elapsedTime) / 1000; 
    } else {
        if (validHits >= maxAttempts) isGameOver = true;
        displayValue = maxAttempts - validHits; 
    }

    if (isGameOver) {
      endGame();
      return;
    }
    
    // Odświeżanie ekranu
    if (displayValue != lastDisplayedValue || score != lastDisplayedScore) {
       displayInGameStatus(displayValue, score, selectedMode);
       lastDisplayedValue = displayValue;
       lastDisplayedScore = score;
    }

    // Odczyt przycisku
    int buttonState = pcf.read(pcfPins[currentTarget]);
    
    if (buttonState == 0) {
      unsigned long reactionEndTime = millis();
      unsigned long singleReaction = reactionEndTime - reactionStartTime;
      
      validHits++;
      score++;
      
      // Format CSV
      Serial.print(validHits); 
      Serial.print(","); 
      Serial.print(singleReaction); 
      Serial.print(",");            
      Serial.println(selectedMode); 
      
      totalReactionTime += singleReaction;

      turnOffLed(currentTarget);

      delay(20); 
      while(pcf.read(pcfPins[currentTarget]) == 0);
      delay(20); 
      
      bool limitOsiagniety = false;
      if (selectedMode == 3 || selectedMode == 4) {
          if (validHits >= maxAttempts) limitOsiagniety = true;
      }
      
      // Uruchom nową rundę jeśli limit nie został osiągnięty
      if (!limitOsiagniety) {
          newRound(); 
      }
    }
  }
}

// Funkcje pomocnicze
void changeMode(int mode) {
  selectedMode = mode;
  displayModeMenu(mode);
  delay(200);
}

//Odliczanie przed rozpoczęciem gry
void startCountdown() {
  lc.clearDisplay(0);
  
  // Wyświetl 03
  lc.setDigit(0, 4, 0, false); // Lewa cyfra (0) na pozycji 4
  lc.setDigit(0, 3, 3, false); // Prawa cyfra (3) na pozycji 3
  delay(1000);
  
  // Wyświetl 02
  lc.setDigit(0, 4, 0, false);
  lc.setDigit(0, 3, 2, false);
  delay(1000);
  
  // Wyświetl 01
  lc.setDigit(0, 4, 0, false);
  lc.setDigit(0, 3, 1, false);
  delay(1000);
  
  lc.clearDisplay(0);
  delay(500); // Pauza przed startem
}

void startGame() {
  while(digitalRead(startButton) == LOW) {
    delay(10);
  }
  delay(200);

  // Odliczanie
  startCountdown(); 
  
  score = 0;
  totalReactionTime = 0;
  validHits = 0;
  lastDisplayedValue = -1; 
  lastDisplayedScore = -1;
  
  gameRunning = true;
  startTime = millis();
  
  Serial.println("\n--- START BADANIA ---");
  Serial.println("Nr_Proby,Czas_ms,Tryb");
  
  lc.clearDisplay(0);
  newRound();
}

void newRound() {
  if (!gameRunning) return;

  int newTarget;
  int maxRange; 

  if (selectedMode == 1 || selectedMode == 3) maxRange = 6; 
  else maxRange = 4;

  if (selectedMode == 3 || selectedMode == 4) {
    unsigned long waitTime = random(200, 1500);
    unsigned long waitStart = millis();

    while (millis() - waitStart < waitTime) {
      if (digitalRead(startButton) == LOW) {
        delay(50);
        endGame(); 
        return;    
      }
    }

  if (!gameRunning) return;
  }
  do {
    newTarget = random(0, maxRange);
  } while (newTarget == currentTarget && score > 0);
  
  currentTarget = newTarget;
  
  turnOnLed(currentTarget);
  reactionStartTime = millis(); 
}

void endGame() {
  gameRunning = false;
  for (int i = 0; i < 6; i++) digitalWrite(ledPins[i], LOW);
  
  int avg = (validHits > 0) ? (totalReactionTime / validHits) : 0;
  if(avg > 999) avg = 999;

  Serial.println("--- KONIEC ---");
  Serial.print("Srednia: "); Serial.println(avg);

  lc.clearDisplay(0);
  
  lc.setChar(0, 7, 't', false);
  
  lc.setDigit(0, 6, (avg / 100) % 10, false);
  lc.setDigit(0, 5, (avg / 10) % 10, false);
  lc.setDigit(0, 4, avg % 10, false);

  lc.setDigit(0, 2, (score / 100) % 10, false);
  lc.setDigit(0, 1, (score / 10) % 10, false);
  lc.setDigit(0, 0, score % 10, false);
  
  delay(500);

  while(true) {
    if (digitalRead(modePins[0]) == LOW) { selectedMode = 1; break; }
    if (digitalRead(modePins[1]) == LOW) { selectedMode = 2; break; }
    if (digitalRead(modePins[2]) == LOW) { selectedMode = 3; break; }
    if (digitalRead(modePins[3]) == LOW) { selectedMode = 4; break; }
    
    if (digitalRead(startButton) == LOW) { break; }
    
    delay(50); 
  }
  
  displayModeMenu(selectedMode);
  delay(500);
}

void turnOnLed(int index) {
  int pin = ledPins[index];
  if (index == 4 || index == 5) analogWrite(pin, pedalBrightness);
  else digitalWrite(pin, HIGH);
}

void turnOffLed(int index) {
  digitalWrite(ledPins[index], LOW);
}

void displayInGameStatus(int leftValue, int points, int mode) {
  if (mode == 3 || mode == 4) {
      lc.setChar(0, 7, 'P', false); 
      lc.setChar(0, 6, ' ', false);
      
      // Cyfry przesunięte w prawo
      lc.setDigit(0, 5, (leftValue / 10) % 10, false);
      lc.setDigit(0, 4, leftValue % 10, false);
      
      lc.setChar(0, 3, ' ', false);
  } else {
      // Dla F1/F2 (Czas)
      lc.setChar(0, 7, ' ', false); 
      lc.setDigit(0, 6, (leftValue / 10) % 10, false);
      bool showDot = (mode == 1 || mode == 2);
      lc.setDigit(0, 5, leftValue % 10, showDot); 
      
      lc.setChar(0, 4, ' ', false);
      lc.setChar(0, 3, ' ', false);
  }

  // Prawa strona (Wynik)
  lc.setDigit(0, 2, (points / 100) % 10, false);
  lc.setDigit(0, 1, (points / 10) % 10, false);
  lc.setDigit(0, 0, points % 10, false);
}

void displayModeMenu(int mode) {
  lc.clearDisplay(0);
  
  lc.setChar(0, 7, '-', false);
  lc.setChar(0, 6, '-', false);
  lc.setChar(0, 5, '-', false);
  
  lc.setChar(0, 4, 'F', false);
  lc.setDigit(0, 3, mode, false);
  
  lc.setChar(0, 2, '-', false);
  lc.setChar(0, 1, '-', false);
  lc.setChar(0, 0, '-', false);
}

void testDisplayOrder() {
  for(int i=0; i<8; i++) lc.setDigit(0, i, 8, false);
  delay(500);
  lc.clearDisplay(0);
}