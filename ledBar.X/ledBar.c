/*
 * File:   ledBar.c
 * Author: theWig
 *
 * Created on March 24, 2019, 9:58 PM
 */


/**
 * Left
 * =====================================================
 * Restructuring code
 * pseudo oop
 * Just added the structs, no implementation change
 * =====================================================
 * */

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/iom328p.h>
#include <util/delay.h>

// Definitions
// ===================================

// LED | BTN | BUZZER | CTRL
#define indicators PORTD
#define _indicators DDRD
#define redLed PORTD0
#define greenLed PORTD1
#define btn PORTD2
#define buzzer PORTD3
#define ctrl1 PORTD4
#define ctrl2 PORTD5
#define ctrl3 PORTD6
#define ctrl4 PORTD7

// LEDBAR
#define ledBar PORTB
#define _ledBar DDRB

// LCD ****to change in setup
#define RS PORTC4
#define E PORTC5
#define lcd PORTC
#define _lcd DDRC

// GLOBAL VARs
char leds = 0;
char gameStatus = 'F';


// Structs

// =====================================================
// Bar
// =====================================================
struct bar{
    // Actual number in bar 
    char leds;

    // LED Bar length
    char length;
};

// =====================================================
// Button
// =====================================================
enum buttonEnum{
    start,
    shoot,
    reset
};

enum buttonEnable{
    disabled,
    enabled
};

struct button{
    // If start or shoot
    enum buttonEnum status;

    // Is it enabled
    enum buttonEnable enable;
};

// =====================================================
// Music
// =====================================================
enum play{
    shot,
    progress,
    win,
    lose
};

struct music{
    // Music to be played
    enum play playing;
};

// =====================================================
// Control
// =====================================================
enum controlLevel{
    easy,
    medium,
    hard
};

enum controlType{
    practice,
    game
};

struct control{
    // Difficulty
    enum controlLevel level;

    // Practice or actual game
    enum controlType type;
};

// =====================================================
// Game
// =====================================================
enum gameState{
    set,
    inGame,
    gameover
};

struct game{
    enum gameState state;
    char chances;
    char score;
    struct bar gameBar;
    struct button gameButton;
    struct music gameMusic;
    struct control gameControl;
};
// =====================================================

// Prototipos
void setup();
void ledTimer();
void ledTimerLoad( int timerVal );
void ledTimerStop();
void ledTimerStart();
void shotCheck();



void setup(){
    
    // LED | BTN | BUZZER | CTRL
    _indicators = (1<<redLed)|(1<<greenLed)|(0<<btn)|(1<<buzzer)|(0<<ctrl1)|(0<<ctrl2)|(0<<ctrl3)|(0<<ctrl4);
    // _indicators |= (0<<ctrl1)|(0<<ctrl2)|(0<<ctrl3)|(0<<ctrl4);
    indicators = (0<<redLed)|(0<<greenLed)|(1<<btn)|(0<<buzzer)|(1<<ctrl1)|(1<<ctrl2)|(1<<ctrl3)|(1<<ctrl4);
    // indicators |= (1<<ctrl1)|(1<<ctrl2)|(1<<ctrl3)|(1<<ctrl4);

    // LEDBAR
    // _ledBar &= (1<<PORTB0)|(1<<PORTB1)|(1<<PORTB2)|(1<<PORTB3);
    // _ledBar |= (0<<PORTB4)|(0<<PORTB5); //Available
    // ledBar &= (0<<PORTB0)|(0<<PORTB1)|(0<<PORTB2)|(0<<PORTB3);
    // ledBar |= (1<<PORTB4)|(1<<PORTB5); //Available
    _ledBar = 0xFF;
    ledBar = 0x00;

    // LCD
    DDRC |= (1<<PORTC0)|(1<<PORTC1)|(1<<PORTC2)|(1<<PORTC3);
    DDRC |= (1<<PORTC4)|(1<<PORTC5);
    PORTC |= (0<<PORTC0)|(0<<PORTC1)|(0<<PORTC2)|(0<<PORTC3);
    PORTC |= (0<<PORTC4)|(0<<PORTC5);

    // BTN INTERRUPT
    EICRA |= (0<<ISC01)|(0<<ISC00);
    EIMSK |= (0<<INT1)|(1<<INT0);
    
    // LEDBAR TIMER
    ledTimer();
    
    // Global Interrupt
    sei();

    // Game
    struct game game1;
}

// Carga un valor al ledTimer 
// CTC 
// Timer1
void ledTimerLoad( int timerVal ){
    OCR1AH |= (timerVal >> 8);
    OCR1AL |= timerVal & 0x00FF;
}


// Setea el timer para el ledBar
// Timer1
// Opera en CTC
// Prescaler: 1024
void ledTimer(){
    TCCR1A = (0<<COM1A1)|(0<<COM1A0)|(0<<COM1B1)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10);
    TCCR1B = (0<<WGM13)|(1<<WGM12)|(1<<CS12)|(0<<CS11)|(1<<CS10);
    TIMSK1 = (1<<OCIE1A); 
    ledTimerLoad( 800 );
}

// Chequea si atino o no
void shotCheck(){
    leds = 0;
}

// Interrupciones
ISR(TIMER1_COMPA_vect){
    if(gameStatus == 'T'){
        if( leds >= 10 )
            shotCheck();
        ledBar = leds++;
    }
}

ISR(INT0_vect){
    if( gameStatus == 'F'){ //First time. Game start.
        gameStatus = 'T';
    }else{ //Second time. Game stop.
        gameStatus = 'F';
        shotCheck();
    }
    _delay_ms(300);                  
}

int main(void) {
    /* Replace with your application code */
    setup();
    while (1) {
    }
}