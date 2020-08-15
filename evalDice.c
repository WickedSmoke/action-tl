// User must define DICE_ROLL and #include this file.


static int evalDiceToken( int negative, int roll, int n )
{
    if( roll )
        n = DICE_ROLL(n);
    return negative ? -n : n;
}


/*
 * Examples spec strings:
 *    "d20"
 *    "d20+d4-3"
 */
static int evalDice( const char* spec )
{
    int sum = 0;
    int neg = 0;
    int roll = 0;
    int n = 0;
    int ch;

    while( (ch = *spec++) )
    {
        if( ch == 'd' )
        {
            roll = 1;
        }
        else if( ch == '+' )
        {
            sum += evalDiceToken( neg, roll, n );
            n = roll = 0;
            neg = 0;
        }
        else if( ch == '-' )
        {
            sum += evalDiceToken( neg, roll, n );
            n = roll = 0;
            neg = 1;
        }
        else if( ch >= '0' && ch <= '9' )
        {
            n = (n * 10) + (ch - '0');
        }
    }
    sum += evalDiceToken( neg, roll, n );

    return sum;
}
