/*
 * File:   ledBar.c
 * Author: theWig
 *
 * Created on March 24, 2019, 9:58 PM
 */

/**
 * Left
 * =====================================================
 * Agregar random // agregar rango
 * Agregar buzzer // Timers configurados
 * =====================================================
 * */

// =====================================================
// INCLUDES
// =====================================================
// #region
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/iom328p.h>
#include <util/delay.h>
// #endregion

// =====================================================
// DEFINITIONS
// =====================================================
// #region
// LED | BTN | BUZZER | CTRL
#define indicators PORTD
#define _indicators DDRD
#define redLed PORTD0
#define greenLed PORTD1
#define greenOff indicators |= (1 << greenLed)
#define greenOn indicators &= ~(1 << greenLed)
#define redOff indicators |= (1 << redLed)
#define redOn indicators &= ~(1 << redLed)
#define btn PIND2
#define buzzer PORTD3
#define ctrl PIND
#define ctrl1 PIND4
#define ctrl2 PIND5
#define ctrl3 PIND6
#define ctrl4 PIND7 // Game Type
#define l0 1400
#define l1 800
#define l3 500
#define l7 150

#define testIt PORTB |= (1 << PB5)

// LEDBAR
#define ledBar PORTB
#define _ledBar DDRB
#define _ledBar_ PINB

// LCD ****to change in setup
#define RS PORTC4
#define E PORTC5
#define lcd PORTC
#define _lcd DDRC
#define num 48
// #endregion

// =====================================================
// GLOBAL VARS
// =====================================================
// #region
// Game
struct game game1;

// #endregion

// =====================================================
// ESTRUCTURAS
// =====================================================
// #region
// =====================================================
// Bar
// =====================================================
// #region
struct bar
{
    // Actual number in bar
    char leds;

    // LED Bar length
    char length;
};
// #endregion

// =====================================================
// Button
// =====================================================
// #region
enum buttonEnum
{
    start,
    shoot,
    reset
};

enum buttonEnable
{
    disabled,
    enabled
};

struct button
{
    // If start, shoot or reset
    enum buttonEnum status;

    // If enabled or disabled
    enum buttonEnable enable;
};
// #endregion

// =====================================================
// Music
// =====================================================
// #region
enum play
{
    shot,
    progress,
    win,
    lose
};

struct music
{
    // Music to be played
    enum play playing;
};
// #endregion

// =====================================================
// Control
// =====================================================
// #region
enum controlLevel
{
    easy=0,
    medium=1,
    hard=3,
    veryHard=7
};

enum controlType
{
    practice = 0,
    game = 1
};

struct control
{
    // Difficulty
    enum controlLevel level;

    // Practice or actual game
    enum controlType type;
};
// #endregion

// =====================================================
// Game
// =====================================================
// #region
enum gameState
{
    set,
    inGame,
    gameover
};

enum gamer
{
    loser,
    winner
};

struct game
{
    enum gameState state;
    enum gamer player;
    char chances;
    char score;
    char toWin;
    char winScore;
    struct bar gameBar;
    struct button gameButton;
    struct music gameMusic;
    struct control gameControl;
};
// =====================================================
// #endregion

// #endregion

// =====================================================
// PROTOTIPOS
// =====================================================
// #region
void setup();
void ledTimer();
void ledTimerLoad(int timerVal);
void ledTimerStop();
void ledTimerStart();
char winShot();
void gameSet();
char pressed();
char ledInc();
char won();
char loseShot();
void controlSet();
void gameReset();
void buzzTimer();
void buzzTimerLoad(int timerVal);
void periodTimer();
void periodTimerLoad(int timerVal);
void rScreen();
void lcdClear();

void charger(char a);
void chargerInst(char a);
void lcdEcrire(char a);
void habiliter();
void lcdInitialiser4();
void lcdPrint(const char *p);
void lcdCursor(int a, int b);
// #endregion

// =====================================================
// SETUP
// =====================================================
// #region
// Setea el ambiente e inicializa el primer juego
void setup()
{

    // LED | BTN | BUZZER | CTRL
    _indicators = (1 << redLed) | (1 << greenLed) | (0 << btn) | (1 << buzzer) | (0 << ctrl1) | (0 << ctrl2) | (0 << ctrl3) | (0 << ctrl4);
    indicators = (1 << redLed) | (1 << greenLed) | (1 << btn) | (0 << buzzer) | (1 << ctrl1) | (1 << ctrl2) | (1 << ctrl3) | (1 << ctrl4);

    // LEDBAR
    _ledBar = 0xFF;
    ledBar = 0x00;

    // LCD
    DDRC |= (1 << PORTC0) | (1 << PORTC1) | (1 << PORTC2) | (1 << PORTC3);
    DDRC |= (1 << PORTC4) | (1 << PORTC5);
    PORTC |= (0 << PORTC0) | (0 << PORTC1) | (0 << PORTC2) | (0 << PORTC3);
    PORTC |= (0 << PORTC4) | (0 << PORTC5);

    // BTN INTERRUPT
    EICRA |= (0 << ISC01) | (0 << ISC00);
    EIMSK |= (0 << INT1) | (1 << INT0);

    // DIP SW INTERRUPT
    PCICR = (1 << PCIE2);
    PCMSK2 = (1 << PCINT23) | (1 << PCINT22) | (1 << PCINT21) | (1 << PCINT20);

    // LEDBAR TIMER
    ledTimer();

    // BUZZER TIMER
    buzzTimer();

    // Global Interrupt
    sei();

    // Initial setup
    game1.toWin = 8;
    struct control gc;
    game1.gameControl = gc;
    game1.winScore = 10;
    struct button bt;
    game1.gameButton = bt;
    struct bar br;
    game1.gameBar = br;
    game1.gameBar.length = 10;

    gameSet();
}
// #endregion

// =====================================================
// METODOS
// =====================================================
// #region
// Cambia el estado del boton al ser presionado
char pressed()
{
    if (game1.gameButton.enable == enabled)
    {
        if (game1.gameButton.status != reset)
        {
            game1.gameButton.status++;
        }
        else
        {
            game1.gameButton.status = start;
        }
        return game1.gameButton.status;
    }
    return 4;
}

// Incrementa la barra de leds. Si pudo incrementar retorna 1; de lo contrario retorna 0.
char ledInc()
{
    if (game1.gameBar.leds < (game1.gameBar.length - 1))
    {
        game1.gameBar.leds++;
        return 1;
    }
    return 0;
}

// Inicia un nuevo juego
void gameSet()
{
    game1.gameButton.enable = disabled;
    // char temp;
    controlSet();
    if(game1.gameControl.type == practice)
        game1.chances = 25 - game1.winScore;
    else
        game1.chances = 11 - game1.winScore;
    
    game1.score = 0;
    game1.player = loser;

    gameReset();
    
}

// Reinicia el juego para la proxima oportunidad
void gameReset()
{
    game1.state = set;
    game1.gameButton.status = reset;
    game1.gameButton.enable = enabled;
    game1.gameBar.leds = 0;
    ledBar = game1.gameBar.leds;
}

// Chequea si atino o no
// @return 1 si atino 0 si fallo
char winShot()
{
    return game1.gameBar.leds == game1.toWin;
}

// Chequea si le quedan oportunidades al jugador
// Retorna 1 si ya no le quedan oportunidades 0 si aun le quedan
char loseShot()
{
    return game1.chances == 0;
}

// Chequea si ya gano o si le faltan intentos
// @return 0 si le faltan intentos 1 si ya gano
char won()
{
    // game1.score++;
    if (game1.score == game1.winScore)
    {
        game1.player = winner;
        return 1;
    }
    return 0;
}

void controlSet()
{
    testIt;
    char sw;
    sw = ctrl & ((1 << ctrl3) | (1 << ctrl2) | (1 << ctrl1)); // dificultad
    sw = (sw>>4);
    switch (sw)
    {
        case 0:
            game1.gameControl.level = easy;
            ledTimerLoad(l0);
            break;
        case 1:
            game1.gameControl.level = medium;
            ledTimerLoad(l1);
            break;
        case 3:
            game1.gameControl.level = hard;
            ledTimerLoad(l3);
            break;
        case 7:
            game1.gameControl.level = veryHard;
            ledTimerLoad(l7);
            break;
        default:
            break;
    }
        
    // if((ctrl & (1 << ctrl4)) == )
    game1.gameControl.type = ((ctrl & (1 << ctrl4)) >> 4); // tipo practica o juego
}

char random(char seed){
    char number = TCNT0;
    number += TCNT1;
    //agregar rango
    return number*2;
}

void rScreen(){
    char temp;
    lcdClear();
    lcdCursor(0,0);
    if (game1.gameControl.type == practice)
        lcdPrint("Nov ");
    else
        lcdPrint("Pro ");
    lcdPrint("Game");
    lcdCursor(10,0);
    lcdPrint("lvl:");
    lcdCursor(15,0);
    temp = game1.gameControl.level + num;
    lcdPrint(&temp);
    lcdCursor(0,1);
    lcdPrint("Pt: ");
    temp = game1.score + num;
    lcdPrint(&temp);
    lcdPrint("  Try: ");
    temp = game1.chances + num;
    if(game1.chances < 10){
        lcdPrint(&temp);
    }else{
        switch (game1.chances)
        {
            case 10:
                lcdPrint("10");
                break;
            case 11:
                lcdPrint("11");
                break;
            case 12:
                lcdPrint("12");
                break;
            case 13:
                lcdPrint("13");
                break;
            case 14:
                lcdPrint("14");
                break;
            case 15:
                lcdPrint("15");
                break;
            default:
                break;
        }
    }
    
}

// #endregion

// =====================================================
// TIMERS
// =====================================================
// #region
// Carga un valor al ledTimer
// CTC
// Timer1
void ledTimerLoad(int timerVal)
{
    OCR1AH = (timerVal >> 8);
    OCR1AL = timerVal & 0x00FF;
}

// Setea el timer para el ledBar
// Timer1
// Opera en CTC
// Prescaler: 1024
void ledTimer()
{
    TCCR1A = (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << WGM11) | (0 << WGM10);
    TCCR1B = (0 << WGM13) | (1 << WGM12) | (1 << CS12) | (0 << CS11) | (1 << CS10);
    TIMSK1 = (1 << OCIE1A);
    ledTimerLoad(1400);
}

// Carga un valor al buzzerTimer
// CTC
// Timer2
void buzzTimerLoad(int timerVal)
{
    OCR2B = timerVal;
}

// Setea el timer para el buzzer
// Timer2
// Opera en CTC
// Prescaler: 0
void buzzTimer()
{
    TCCR2A = (0 << COM2A1) | (0 << COM2A0) | (0 << COM2B1) | (1 << COM2B0) | (1 << WGM21) | (0 << WGM20);
    TCCR2B = (0 << WGM22) | (0 << CS22) | (0 << CS21) | (1 << CS20);
    TIMSK2 = (1 << OCIE2B);
    buzzTimerLoad(255);
}

// Carga un valor al periodTimer (periodo del buzzer)
// CTC
// Timer0
// timerVal = 250 for 1ms with 64 prescaler
void periodTimerLoad(int timerVal)
{
    OCR0B = timerVal;
}

// Setea el timer para el periodo del buzzer
// Timer0
// Opera en CTC
// Prescaler: 64
void periodTimer()
{
    TCCR0A = (0 << COM0A1) | (0 << COM0A0) | (0 << COM0B1) | (0 << COM0B0) | (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (1 << CS00);
    TIMSK0 = (1 << OCIE0B);
    periodTimerLoad(250);
}

// #endregion

// =====================================================
// LCD
// =====================================================
// #region
// Carga una instruccion a la lcd E 4 bits
void chargerInst(char a)
{
    charger((a >> 4) & 0x0F);
    charger(a & 0x0F);
}

// Carga la lcd con una instruccion
void charger(char a)
{
    lcd = a;
    lcd &= ~(1 << RS);
    habiliter();
}

// Carga la lcd con un dato
void chargerD(char a)
{
    lcd = a;
    lcd |= (1 << RS);
    habiliter();
}

// Carga un dato a la lcd E 4 bits
void lcdEcrire(char a)
{
    chargerD((a >> 4) & 0x0F);
    chargerD(a & 0x0F);
}

// Da un pulso al Eable
void habiliter()
{
    lcd |= (1 << E);
    _delay_ms(2);
    lcd &= ~(1 << E);
    _delay_ms(2);
}

// Inicializa la lcd E 4 bits
void lcdInitialiser4()
{
    char commands[6] = {0x01, 0x02, 0x28, 0x06, 0x0C, 0x80};
    _delay_ms(2);

    for (int i = 0; i < 6; i++)
    {
        chargerInst(commands[i]);
    }
}

// Imprime un string E la lcd
void lcdPrint(const char *p){
// void lcdPrint(char *p){
    while (*p != '\0')
    {
        lcdEcrire(*p);
        p++;
        _delay_ms(4);
    }
}

void lcdClear()
{
    lcdCursor(0, 0);
    lcdPrint("                ");
    lcdCursor(0, 1);
    lcdPrint("                ");
}

// Setea el cursor de la lcd (solo funciona para 16x2)
void lcdCursor(int a, int b)
{
    chargerInst((0x80 + b * 0x40) | a);
}
// #endregion

// =====================================================
// INTERRUPCIONES
// =====================================================
// #region
// Timer1 interrupt
// Game timer. Se encarga de llevar el juego en si.
ISR(TIMER1_COMPA_vect)
{

    if (game1.state == inGame)
    {
        if (ledInc())
        {
            ledBar = game1.gameBar.leds;
            buzzTimerLoad(255 - game1.gameBar.leds*25);
        }
        else
        {
            game1.state = gameover;
            game1.gameButton.status = shoot;
        }
    }
    else if (game1.state == gameover)
    {
        if (winShot())
        {
            game1.score++;
            greenOn;
            _delay_ms(2000);
            greenOff;
            if (won())
            {
                game1.gameButton.enable = disabled;
                lcdClear();
                lcdCursor(0,0);
                lcdPrint("Ganaste perra!");
                _delay_ms(4000);
                gameSet();
            }
            rScreen();
            gameReset();
        }
        else if (loseShot())
        {
            game1.gameButton.enable = disabled;
            game1.score = 0;
            game1.player = loser;
            lcdClear();
            lcdCursor(0,0);
            lcdPrint("Perdiste perra!");
            _delay_ms(4000);
            gameSet();
            rScreen();
        }
        else
        {
            game1.chances--;
            redOn;
            _delay_ms(2000);
            redOff;
            rScreen();
            gameReset();
        }
    }
    else
    {
        gameReset();
    }
}

// Int0 interrupt
// Btn. Inicia, dispara y resetea el juego.
ISR(INT0_vect)
{

    char b = pressed();
    if (b == start)
    {
        game1.gameBar.leds = 0;
        game1.state = inGame;
        lcdClear();
        lcdCursor(0,0);
        lcdPrint("Throw...");
        buzzTimer();
    }
    else if (b == shoot)
    {
        game1.state = gameover;
        lcdCursor(0,1);
        lcdPrint("Shoot!");
        TCCR2B = 0;
    }
    else if (b == reset)
    {
        game1.state = set;
    }

    // debounce
    _delay_ms(200);
}

// PCINT2 interrupt
// Dip Sw. Setea la difficultad y el modo del juego.
ISR(PCINT2_vect)
{
    gameSet();
    rScreen();
    _delay_ms(200);
}
// #endregion

// =====================================================
// APP
// =====================================================
// #region
int main(void)
{
    setup();

    // for(char i = 255; i>0 ; i--){
    //     buzzTimerLoad(i);
    // }
    lcdInitialiser4();
    lcdCursor(0, 0);
    lcdPrint("Mushroom");
    lcdCursor(1, 1);
    lcdPrint("Kingdom");
    _delay_ms(2000);
    lcdClear();
    rScreen();

    while (1)
        ;
}
// #endregion