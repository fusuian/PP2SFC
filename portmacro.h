#define PORT(n) ( (n <=  7)? PORTD : \
                ( (n <= 13)? PORTB : \
                ( (n <= 19)? PORTC : PORTB)))

#define REG(n)  ( (n <=  7)? (n) : \
                ( (n <= 13)? (n - 8) : \
                ( (n <= 19)? (n - 14) : (13 - 8) )))

#define portOn(p)  ( PORT(p) |=  _BV(REG(p)) )
#define portOff(p) ( PORT(p) &= ~_BV(REG(p)) )

#define isPin(p)  ( (PORT(p) & ~_BV(REG(p))) != 0 )

