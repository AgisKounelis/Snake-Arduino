#include <LiquidCrystal.h>
#include "LedControl.h"
#include "LinkedList.h"
#include "IRremote.h"

#define start_parts_number 3
#define refresh_time 220  // in milliseconds

#define matrix_first 0
#define matrix_last 7

#define start_collumn 4
#define start_row 3

typedef struct coordinates { int row, collumn; } coordinates;
enum class Move { LEFT, RIGHT, UP, DOWN };

const int DIN = 8, CLK = 10, CS = 9;
LedControl panel = LedControl(DIN, CLK, CS, 1);

const int RS = 2, EN = 3, D4 = 4, D5 = 5, D6 = 6, D7 = 7;
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

const int BUZZER_PIN = 12;

const int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;


Move prev_move, myMove;
LinkedList<coordinates> parts;
coordinates food;

unsigned int score;
unsigned int times = 1;


void setup() {
  pinMode(RECV_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  irrecv.enableIRIn();
  irrecv.blink13(true);

  panel.shutdown(0, false);

  lcd.begin(16,1);
  
  set_initial_conditions();

}


void loop() {
  if (irrecv.decode(&results)) {  //signal get
    if (results.value == 0xFF18E7 && prev_move != Move::DOWN)  // 2 = UP
      myMove = Move::UP;
    else if (results.value == 0xFF10EF && prev_move != Move::RIGHT)  // 4 = LEFT
      myMove = Move::LEFT;
    else if (results.value == 0xFF5AA5 && prev_move != Move::LEFT)  // 6  = RIGHT
      myMove = Move::RIGHT;
    else if (results.value == 0xFF4AB5 && prev_move != Move::UP)  // 8 = DOWN
      myMove = Move::DOWN;

    irrecv.resume();
  }

  if (millis() / refresh_time == times) { // refresh every refresh_time ms
    
    digitalWrite(BUZZER_PIN, LOW);  // buzzer deactivate

    prev_move = myMove;

    panel.clearDisplay(0);
    moveParts();
    print_parts();
    print_food();

    if (hasEatenFood()) {
      coordinates newLast = {.row = parts.get(parts.size() - 1).row,
                             .collumn = parts.get(parts.size() - 1)
                                            .collumn};  // creates last at the
                                                        // same position with
                                                        // the previous last
      parts.add(newLast);
      setNewFoodPosition();

      digitalWrite(BUZZER_PIN, HIGH); // buzzer activate

      score++;
      printScore();
    }


    if (isTheEnd())
      set_initial_conditions();



    times++;
      
  }
}



void printScore() { lcd.clear(); lcd.print("Score: "); lcd.print(score); }

void print_parts() {
  for (int i = 0; i < parts.size(); i++)
    panel.setLed(0, parts.get(i).row, parts.get(i).collumn,
                 true);  // turns on LED at collumn, row
}

void print_food() { panel.setLed(0, food.row, food.collumn, true); }

void set_initial_conditions() {
  myMove = Move::LEFT;
  prev_move = Move::LEFT;

  food.row = 3;
  food.collumn = 1;

  parts.clear();
  for (int i = 0; i < start_parts_number; i++) {
    coordinates temp;
    temp.row = start_row;
    temp.collumn = start_collumn + i;
    parts.add(temp);
  }

  
  print_parts();
  print_food();
  
  score = 0;
  printScore();
}

bool isTheEnd() {
  for (int i = 1; i < parts.size(); i++)
    if (parts.get(0).row == parts.get(i).row &&
        parts.get(0).collumn == parts.get(i).collumn)
      return true;

  return false;
}

bool hasEatenFood() {
  return parts.get(0).row == food.row && parts.get(0).collumn == food.collumn;
}

bool isSnakePosition(coordinates coo) {
  for (int i = 0; i < parts.size(); i++)
    if (coo.row == parts.get(i).row && coo.collumn == parts.get(i).collumn)
      return true;

  return false;
}

void setNewFoodPosition() {
  coordinates newPos = {.row = (int)random(0, 8), .collumn = (int)random(0, 8)};

  while (isSnakePosition(newPos)) {
    newPos.row = random(0, 8);
    newPos.collumn = random(0, 8);
  }

  food = newPos;
}

void moveParts() {
  int x, y;

  if (prev_move == Move::LEFT) {
    x = -1;
    y = 0;
  } else if (prev_move == Move::RIGHT) {
    x = 1;
    y = 0;
  } else if (prev_move == Move::UP) {
    x = 0;
    y = -1;
  } else  // prev_move==Move::DOWN
  {
    x = 0;
    y = 1;
  }

  coordinates newFirst = parts.pop();
  newFirst.row = parts.get(0).row + y;
  newFirst.collumn = parts.get(0).collumn + x;

  if (newFirst.row > matrix_last)
    newFirst.row = matrix_first;
  else if (newFirst.row < matrix_first)
    newFirst.row = matrix_last;
  if (newFirst.collumn > matrix_last)
    newFirst.collumn = matrix_first;
  else if (newFirst.collumn < matrix_first)
    newFirst.collumn = matrix_last;

  parts.unshift(newFirst);  // adds last to the first position
}
