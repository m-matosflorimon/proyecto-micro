/*
 * File:   ledBar.c
 * Author: theWig
 *
 * Created on March 24, 2019, 9:58 PM
 */


/**
 * Left
 * =====================================================
 * Code restructured
 * Se puede jugar
 * Agregar puntuacion e intentos
 * Agregar configuracion desde el dip sw // TEST
 * Agregar random
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
#define greenOff indicators |= (1<<greenLed)
#define greenOn indicators &= ~(1<<greenLed)
#define redOff indicators |= (1<<redLed)   
#define redOn indicators &= ~(1<<redLed)
#define btn PIND2
#define buzzer PORTD3
#define ctrl PIND
#define ctrl1 PIND4
#define ctrl2 PIND5
#define ctrl3 PIND6
#define ctrl4 PIND7 // Game Type

// LEDBAR
#define ledBar PORTB
#define _ledBar DDRB

// LCD ****to change in setup
#define RS PORTC4
#define E PORTC5
#define lcd PORTC
#define _lcd DDRC

// GLOBAL VARs
// char leds = 0;
// char gameStatus = 'F';

// Game
struct game game1;


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
    // If start, shoot or reset
    enum buttonEnum status;

    // If enabled or disabled
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
    hard,
    veryHard
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

enum gamer{
    loser,
    winner
};

struct game{
    enum gameState state;
    enum gamer player; 
    char chances;
    char score;
    char toWin;
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
char winShot();
void gameSet();
char pressed();
char ledInc();
char won();


void setup(){
    
    // LED | BTN | BUZZER | CTRL
    _indicators = (1<<redLed)|(1<<greenLed)|(0<<btn)|(1<<buzzer)|(0<<ctrl1)|(0<<ctrl2)|(0<<ctrl3)|(0<<ctrl4);
    indicators = (1<<redLed)|(1<<greenLed)|(1<<btn)|(0<<buzzer)|(1<<ctrl1)|(1<<ctrl2)|(1<<ctrl3)|(1<<ctrl4);

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

    // DIP SW INTERRUPT
    PCICR = (1<<PCIE2);
    PCMSK2 = (1<<PCINT23)|(1<<PCINT22)|(1<<PCINT21)|(1<<PCINT20);
    
    // LEDBAR TIMER
    ledTimer();
    
    // Global Interrupt
    sei();

    // Initial setup
    gameSet();
    
}



// =====================================================
// METODOS
// =====================================================

// Cambia el estado del boton al ser presionado
char pressed(){
    if( game1.gameButton.enable == enabled ){
        if( game1.gameButton.status != reset ){
            game1.gameButton.status++;
        }else{
            game1.gameButton.status = start;
        }
        return game1.gameButton.status;
    }
    return 4;
}

// Incrementa la barra de leds. Si pudo incrementar retorna 1; de lo contrario retorna 0.
char ledInc(){
    if(game1.gameBar.leds < (game1.gameBar.length - 1)){
        game1.gameBar.leds++;
        return 1;
    }
    return 0;
}

void gameSet(){
    game1.state = set;
    game1.chances = 10;
    game1.score = 0;
    game1.toWin = 8;
    struct control gc;
    game1.gameControl = gc;
    game1.gameControl.level = easy;
    game1.gameControl.type = practice;
    struct button bt;
    game1.gameButton = bt;
    game1.gameButton.status = reset;
    game1.gameButton.enable = enabled;
    struct bar br;
    game1.gameBar = br;
    game1.gameBar.length = 10;
    game1.gameBar.leds = 0;
    game1.player = loser;
 
    ledBar = game1.gameBar.leds;
    greenOff;
    redOff;
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
// @return 1 si atino 0 si fallo
char winShot(){
    // if(game1.gameBar.leds == game1.toWin)
    //     return 1;
    // return 0;
    return game1.gameBar.leds == game1.toWin;
}

// Chequea si ya gano o si le faltan intentos
// @return 0 si le faltan intentos 1 si ya gano
char won(){
    game1.score++;
    if(game1.score == game1.chances){
        game1.player = winner;
        return 1;
    }
    return 0;
}


// =====================================================
// INTERRUPCIONES
// =====================================================
// Timer1 interrupt
// Game timer. Se encarga de llevar el juego en si.
ISR(TIMER1_COMPA_vect){

    if(game1.state == inGame){
        if(ledInc()){
            ledBar = game1.gameBar.leds;
        }else{
            game1.state = gameover;
            game1.gameButton.status = shoot;
        }
    }else if(game1.state == gameover){
        if(winShot()){
            greenOn;
            if( won() ){}
                // revisar exactamente como se gana/pierde
        }else {
            redOn;
            game1.score = 0;
        }   
    }else{
        gameSet();
    }
}

// Int0 interrupt
// Btn. Inicia, dispara y resetea el juego.
ISR(INT0_vect){

    char b = pressed();
    if(b == start){
        game1.gameBar.leds = 0;
        game1.state = inGame;
    }else if(b == shoot){
        game1.state = gameover;
    }else if(b == reset){
        game1.state = set;
    }

    // debounce
    _delay_ms(150);                  
}

// PCINT2 interrupt
// Dip Sw. Setea la difficultad y el modo del juego.
ISR(PCINT2_vect){

    char sw = ctrl & (~((0<<ctrl3)|(0<<ctrl2)|(0<<ctrl1))); // tipo practica o juego

    switch(sw){
        case 0:
        case 1:
        case 3:
        case 7:
            game1.gameControl.level = sw;
            break;
        default:
            break; // Not valid
    }
    game1.gameControl.type = (ctrl & (~(0<<ctrl4))>>ctrl3);
}

// =====================================================
// APP
// =====================================================
int main(void) {
    /* Replace with your application code */
    setup();
    while (1) {
    }
}