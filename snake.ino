#include "U8glib.h"
const byte outputPin = 4; // Left player

const byte inputPin = 3; // Left player right button
const byte inputPin2 = 2; // Left player left button

const byte outputPin2 = 12; // Right player
const byte inputPinRightP2 = 11; // Right player right button read
const byte inputPinLeftP2 = 10; // Right player left button read

boolean p2RightPressed = false;
boolean p2LeftPressed = false;

byte gameState = 0;

int p1KeyDown = 0;
int p2KeyDown = 0;

byte points[2] = {0, 0};

struct snakePart {
  byte x;
  byte y;
  byte oldx;
  byte oldy;
  byte dir;
  snakePart* parent;
};

struct apple {
  byte x;
  byte y;
};

snakePart playerHead[2];

snakePart tail1;
snakePart tail2;

snakePart* currentPart1;

snakePart* tail1End;
snakePart* tail2End;
apple apple[3];


U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  pinMode(inputPin,INPUT);
  pinMode(inputPin2,INPUT);

  pinMode(inputPinRightP2, INPUT);
  pinMode(inputPinLeftP2, INPUT);
  
  
  pinMode(outputPin,OUTPUT);
  pinMode(outputPin2,OUTPUT);

  initSnakes();
  for(byte i = 0; i < 3; i++) {
    apple[i].x = random(5,50)*2;
    apple[i].y = random(5,25)*2;
  }
}

void initSnakes() {
  for(byte i = 0; i < 2; i++) {
    playerHead[i].x = i*90+10;
    playerHead[i].y = 50;
    playerHead[i].oldx = playerHead[i].x;
    playerHead[i].oldy = playerHead[i].y;
    playerHead[i].parent = NULL;
    playerHead[i].dir = 0;
  }

  tail1.x = playerHead[0].oldx;
  tail1.y = playerHead[0].oldy;
  tail1.parent = &playerHead[0];

  tail2.x = playerHead[1].oldx;
  tail2.y = playerHead[1].oldy;
  tail2.parent = &playerHead[1];
  
  tail1End = &tail1;
  tail2End = &tail2;
  currentPart1 = tail1End;
}

void loop() {
  digitalWrite(outputPin,HIGH);
  digitalWrite(outputPin2,HIGH);
  
  
  if(gameState == 0) {
    introScreen();
    return;  
  }
  handleInput();
  moveTail(tail1End);
  moveTail(tail2End);
  
  
  for(byte i = 0; i < 2; i++) {
    playerHead[i].oldx = playerHead[i].x;
    playerHead[i].oldy = playerHead[i].y;
    switch(playerHead[i].dir) {
      case 0:
        playerHead[i].y = (playerHead[i].y > 200) ? 62 : playerHead[i].y-1;
        break;
      case 1:
        playerHead[i].x = (playerHead[i].x > 200) ? 126 : playerHead[i].x-1;
        break;
      case 2:
        playerHead[i].y = (playerHead[i].y > 62) ? 1 : playerHead[i].y+1;    
        break;
      case 3:
        playerHead[i].x = (playerHead[i].x > 126) ? 1 : playerHead[i].x+1;
        break;  
    } 
  }

  for(byte i = 0; i < 2; i++) {
    for(byte j = 0; j < 3; j++) {
      if(playerHead[i].x == apple[j].x && playerHead[i].y == apple[j].y) {
        snakePart* newPart = new snakePart();
        newPart->x = tail1End->x;
        newPart->y = tail1End->y;
        newPart->oldx = newPart->x;
        newPart->oldy = newPart->y;
        if(i == 0) {
          newPart->parent = tail1End;
          tail1End = newPart;
        } else if(i == 1) {
          newPart->parent = tail2End;
          tail2End = newPart;  
        }
        apple[j].x = random(2,126);
        apple[j].y = random(2,58);
      }
    }
    
    snakePart* currentHead2 = tail2End;
    currentPart1 = tail1End;
    while(currentPart1->parent != NULL) {
      if(playerHead[i].x == currentPart1->x && playerHead[i].y == currentPart1->y) {
        points[1-i]+=1;
        removeTail(tail1End);
        removeTail(tail2End);
        initSnakes();
        return;
      }
      currentPart1 = currentPart1->parent;
    }
    
    while(currentHead2->parent != NULL) {
      if(playerHead[i].x == currentHead2->x && playerHead[i].y == currentHead2->y) {
        points[1-i]+=1;
        removeTail(tail1End);
        removeTail(tail2End);
        initSnakes();
        return;
      }
      currentHead2 = currentHead2->parent;
    }
  }
  
  oled.firstPage();
  do {  
    currentPart1 = tail1End;
    while(currentPart1->parent != NULL) {
      oled.drawPixel(currentPart1->x, currentPart1->y);
      currentPart1 = currentPart1->parent;  
    }
    currentPart1 = tail2End;
    while(currentPart1->parent != NULL) {
      oled.drawPixel(currentPart1->x, currentPart1->y);
      currentPart1 = currentPart1->parent;  
    }
    for(byte i = 0; i < 3; i++) {
      oled.drawPixel(apple[i].x, apple[i].y);
    }
    oled.setFont(u8g_font_04b_24);
    for(byte i = 0; i < 2; i++) {
      char buf[9];
      sprintf (buf, "%d", points[i]);
      oled.drawStr(i*90+15, 64, buf);
    }
  } while(oled.nextPage());
  

}

void introScreen() {
  if(digitalRead(inputPin) || digitalRead(inputPin2)) {
    gameState = 1;
  }
  oled.firstPage();
  do {
    oled.setFont(u8g_font_timB12);
    oled.drawStr(100-millis()/50,32,"Snaaaake!");
    int x, y;
    x = random(1,80);
    y = random(1,60);
    oled.drawLine(x,y,x+random(30,100),y);
    oled.drawLine(x,y,x+random(30,100),y);  
    
  } while(oled.nextPage());
}

void handleInput() {
  if(digitalRead(inputPin)) {
      p1KeyDown++;
    if(p1KeyDown == 1) {
      playerHead[0].dir = --playerHead[0].dir == 255 ? 3 : playerHead[0].dir;
    }
  } else if(digitalRead(inputPin2)) {
    p1KeyDown++;
    
    if(p1KeyDown == 1) {
      playerHead[0].dir = ++playerHead[0].dir > 3 ? 0 : playerHead[0].dir;   
    }

  } else {
    p1KeyDown = 0;
  }

  if(digitalRead(inputPinRightP2)) {
      p2KeyDown++;
    if(p2KeyDown == 1) {
      playerHead[1].dir = --playerHead[1].dir == 255 ? 3 : playerHead[1].dir;
    }
  } else if(digitalRead(inputPinLeftP2)) {
    p2KeyDown++;
    
    if(p2KeyDown == 1) {
      playerHead[1].dir = ++playerHead[1].dir > 3 ? 0 : playerHead[1].dir;   
    }

  } else {
    p2KeyDown = 0;
  }

 
}

void moveTail(snakePart* tailEnd) {
  currentPart1 = tailEnd;
  while(currentPart1->parent != NULL) {
    currentPart1->oldx = currentPart1->x;
    currentPart1->oldy = currentPart1->y;
    currentPart1->x = currentPart1->parent->x;
    currentPart1->y = currentPart1->parent->y;
    currentPart1 = currentPart1->parent;  
  }
}

void removeTail(snakePart* tailEnd) {
  snakePart* lastPart = tailEnd;
  snakePart* currentPart = lastPart->parent;
  while(currentPart->parent!=NULL) {
    delete lastPart;
    lastPart = currentPart;
    currentPart = currentPart->parent;  
  }
}
