
#include "mbed.h"
#include "SerialBuffered.h"

SerialBuffered::SerialBuffered( PinName tx, PinName rx, size_t bufferSize ) : Serial(  tx,  rx ) 
{
    m_buffSize = 0;
    m_contentStart = 0;
    m_contentEnd = 0;
    m_timeout = -1.0;
    _txpin = tx;
    _rxpin = rx;
    
    attach( this, &SerialBuffered::handleInterrupt );
    
    m_buff = (uint8_t *) malloc( bufferSize );
    if( m_buff )
        m_buffSize = bufferSize;
}


SerialBuffered::~SerialBuffered()
{
    if( m_buff )
        free( m_buff );
}

void SerialBuffered::setTimeout( float seconds )
{
    m_timeout = seconds;
}
    
size_t SerialBuffered::readBytes( uint8_t *bytes, size_t requested )
{
    int i = 0;

    for( ; i < requested; )
    {
        int c = getc();
        if( c < 0 )
            break;
        bytes[i] = c;
        i++;
    }
    
    return i;
        
}


int SerialBuffered::_getc()
{
    m_timer.reset();
    m_timer.start();
    while (m_contentStart == m_contentEnd) {
        if (m_timeout >= 0 &&  m_timer.read() >= m_timeout )
            return EOF;
        wait_ms( 1 );
    }

    m_timer.stop();
   
    int     nbp = (m_contentStart + 1) % m_buffSize;
    uint8_t result = m_buff[m_contentStart];
    m_contentStart = nbp;
    return result;    
}

//int Serial::_getc() {
//    return _base_getc();
//}

int SerialBuffered::_putc(int c) {
    return _base_putc(c);
}

int SerialBuffered::readable()
{
    return m_contentStart != m_contentEnd ;
}

void SerialBuffered::handleInterrupt()
{
    while (serial_readable(&_serial)) {
        char c_in = _base_getc();
        int  nbp = (m_contentEnd + 1) % m_buffSize;
        
        if (nbp != m_contentStart) {
            m_buff[m_contentEnd] = c_in;
            m_contentEnd = nbp;
        }
    }
}
