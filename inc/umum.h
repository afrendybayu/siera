/*
 * umum.h
 *
 *  Created on: Jul 5, 2013
 *      Author: Afrendy Bayu
 */

#ifndef UMUM_H_
#define UMUM_H_

#include "adl_global.h"

s8 ValidateIPAddress ( ascii *IP );
ascii * ConvertIPitoa ( u32 iIP, ascii * aIP );
u32 ConvertIPatoi ( ascii * aIP );

#endif /* UMUM_H_ */
