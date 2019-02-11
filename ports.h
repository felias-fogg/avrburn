
//Port-Macros
#define BIT(p,b)                (b) 
#define PORT(p,b)               (PORT ## p) 
#define PIN(p,b)                (PIN ## p) 
#define DDR(p,b)                (DDR ## p) 

#define setBitHigh(p,b)         ((p)  |=  (1 << b)) 
#define setBitLow(p,b)          ((p)  &= ~(1 << b)) 
#define toggleBit(p,b)          ((p)  ^=  (1 << b)) 
#define getBit(p,b)             (((p) &   (1 << b)) != 0) 

#define setHigh(io)             setBitHigh(PORT(io),BIT(io)) 
#define setLow(io)              setBitLow(PORT(io),BIT(io)) 
#define toggle(io)              toggleBit(PORT(io),BIT(io)) 

#define getOutput(io)           getBit(PORT(io),BIT(io)) 
#define getInput(io)            getBit(PIN(io),BIT(io)) 

#define setInput(io)            setBitLow(DDR(io),BIT(io)) 
#define setOutput(io)           setBitHigh(DDR(io),BIT(io)) 

