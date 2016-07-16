#pragma once

// This is a buffered serial reading class, using the serial interrupt introduced in mbed library version 18 on 17/11/09

// In the simplest case, construct it with a buffer size at least equal to the largest message you 
// expect your program to receive in one go.

class SerialBuffered : public Serial
{
public:
    SerialBuffered( PinName tx, PinName rx, size_t bufferSize );
    virtual ~SerialBuffered();
    
    int readable(); // returns 1 if there is a character available to read, 0 otherwise
    
    void setTimeout( float seconds );    // maximum time in seconds that getc() should block 
                                         // while waiting for a character
                                         // Pass -1 to disable the timeout.
    
    size_t readBytes( uint8_t *bytes, size_t requested );    // read requested bytes into a buffer, 
                                                             // return number actually read, 
                                                             // which may be less than requested if there has been a timeout
    
    void disable_flow_ctrl(void) { serial_set_flow_control(&_serial, FlowControlNone, NC, NC); }
//    void suspend(bool enable) {
//        if (enable) {
//            serial_irq_set(&_serial, (SerialIrq)RxIrq, 0);
//                serial_break_set(&_serial);
//                _serial.uart->ENABLE         = (UART_ENABLE_ENABLE_Disabled << UART_ENABLE_ENABLE_Pos);
//        } else {
//                _serial.uart->ENABLE         = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
//                serial_break_clear(&_serial);
//            _serial.uart->EVENTS_RXDRDY  = 0;
//            // dummy write needed or TXDRDY trails write rather than leads write.
//            //  pins are disconnected so nothing is physically transmitted on the wire
//            _serial.uart->TXD            = 0;
//            serial_irq_set(&_serial, (SerialIrq)RxIrq, 1);
//        }
//    }

protected:
    PinName _txpin;
    PinName _rxpin;

protected:
    virtual int _getc();
    virtual int _putc(int c);
    
private:
    
    void handleInterrupt();
    
   
    uint8_t *m_buff;            // points at a circular buffer, containing data from m_contentStart, for m_contentSize bytes, wrapping when you get to the end
    volatile uint16_t  m_contentStart;   // index of first bytes of content
    volatile uint16_t  m_contentEnd;     // index of bytes after last byte of content
    uint16_t m_buffSize;
    float m_timeout;
    Timer m_timer;

};
