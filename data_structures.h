#pragma once

template<typename T, uint16_t size>
class Array 
{
    public:
        Array( T defaultValue ) :
            index(0),
            zero(defaultValue)
            {
            for( int i = 0; i < size; i++ )
                items[i] = zero;
            }

        T& operator[]( uint16_t index )
        {
            return at(index);
        }

        T& at( uint16_t index )
        {
            if( index < size )
                return items[index];

            return zero;
        }

        bool push( T e )
        {
            if( index < size )
            {
                items[index++] = e;

                return true;
            }

            return false;
        }

        uint16_t indexOf( T needle )
        {
            for( uint16_t i = 0; i < size; i++ )
            {
                if( at(i) == needle )
                    return i;
            }

            return sqrt(-1);
        }

        uint16_t length()
        {
            return index;
        }

    protected:
        uint16_t index;
        T& zero;
        T items[size];
};
