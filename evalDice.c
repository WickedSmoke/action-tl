/*
  evalDice.c verion 1.0
  Copyright 2020 Karl Robillard

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/*
  User must define DICE_ROLL and #include this file.

  #define DICE_ROLL(n)    (rand() % n + 1)
  #include "evalDice.c"
*/


static int evalDiceToken( int negative, int rollCount, int n )
{
    if( rollCount )
    {
        int t = 0;
        while( rollCount-- )
            t += DICE_ROLL(n);
        n = t;
    }
    return negative ? -n : n;
}


/*
 * Examples spec strings:
 *    "d20"
 *    "d20+d4-3"
 *    "2+4d6"
 */
static int evalDice( const char* spec, void (*emitf)(void*, int), void* user )
{
    int sum = 0;
    int neg = 0;
    int roll = 0;
    int n = 0;
    int r, ch;

#define EVAL_TOKEN \
    r = evalDiceToken( neg, roll, n ); \
    emitf(user, r); \
    sum += r

    while( (ch = *spec++) )
    {
        if( ch == 'd' )
        {
            roll = n ? n : 1;
            n = 0;
        }
        else if( ch == '+' )
        {
            EVAL_TOKEN;
            n = roll = 0;
            neg = 0;
        }
        else if( ch == '-' )
        {
            EVAL_TOKEN;
            n = roll = 0;
            neg = 1;
        }
        else if( ch >= '0' && ch <= '9' )
        {
            n = (n * 10) + (ch - '0');
        }
    }
    EVAL_TOKEN;

#undef EVAL_TOKEN

    return sum;
}
