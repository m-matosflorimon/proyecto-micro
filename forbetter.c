// =====================================================
// INTERRUPCIONES
// =====================================================
// #region
// Timer1 interrupt
// Game timer. Se encarga de llevar el juego en si.
ISR(TIMER1_COMPA_vect){

    if (game1.state == inGame)
    {
        if (ledInc())
        {
            ledBar = game1.gameBar.leds;
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
            greenOn;
            if (won())
            {
            }
            // revisar exactamente como se gana/pierde
        }
        else if (loseShot())
        {

            while (1)
            {
                redOn;
                _delay_ms(1000);
                redOff;
                _delay_ms(1000);
            }
            game1.score = 0;
            game1.player = loser;
        }
        else
        {
            game1.chances--;
            redOn;
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
ISR(INT0_vect){

    char b = pressed();
    if (b == start)
    {
        game1.gameBar.leds = 0;
        game1.state = inGame;
        greenOff;
        redOff;
    }
    else if (b == shoot)
    {
        game1.state = gameover;
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
}
// #endregion
