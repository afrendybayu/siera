/*
 * umum.c
 *
 *  Created on: Jul 5, 2013
 *      Author: Afrendy Bayu
 */


#include "at_cmd_service.h"
#include "umum.h"



ascii * ConvertIPitoa ( u32 iIP, ascii * aIP )
{
    wm_sprintf ( aIP, "%d.%d.%d.%d", ( iIP >> PG_SHIFT_THREE_BYTES ) & PG_MASK,
                    ( iIP >> PG_SHIFT_TWO_BYTES ) & PG_MASK, ( iIP
                          >> PG_SHIFT_ONE_BYTES ) & PG_MASK, ( iIP & PG_MASK ) );
    return aIP;
}


u32 ConvertIPatoi ( ascii * aIP )
{
    ascii pTmp [ PG_FOUR ] = { PG_ZERO };
    u8 Counter1 = PG_ZERO, Counter2 = PG_ZERO, Counter3 = PG_ZERO;
    u8 pDest [ PG_FOUR ] = { PG_ZERO };
    u32 Native = PG_ZERO;

    /* Octet 1 a N of the IP address */
    while ( aIP [ Counter1 ] != PG_ZERO )
    {
        while ( ( aIP [ Counter1 ] != PG_IP_FIRST_BYTE_COMPARATOR )
                        && ( aIP [ Counter1 ] != PG_ZERO ) )
        {
            pTmp [ Counter2 ] = aIP [ Counter1 ];
            Counter2++;
            Counter1++;
        }

        pTmp [ Counter2 ] = PG_ZERO;
        pDest [ Counter3 ] = ( u8 ) wm_atoi ( ( void* ) pTmp );
        Counter3++;
        if ( aIP [ Counter1 ] == PG_IP_FIRST_BYTE_COMPARATOR )
        {
            Counter1++;
        }
        Counter2 = PG_ZERO;
    }

    /* Convert into u32 */
    Native = PG_ZERO;

    Native = pDest [ PG_ZERO ] << PG_SHIFT_THREE_BYTES;
    Native |= pDest [ PG_ONE ] << PG_SHIFT_TWO_BYTES;
    Native |= pDest [ PG_TWO ] << PG_SHIFT_ONE_BYTES;
    Native |= pDest [ PG_THREE ];

    return Native;
}
