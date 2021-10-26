//
//  vas_fir_fixedFirChannel.h
//  vas_partconv~
//
//  Created by Harvey Keitel on 26.04.21.
//  Copyright © 2021 Intrinsic Audio. All rights reserved.
//

#ifndef vas_fixedFirChannel_h
#define vas_fixedFirChannel_h

#include <pthread.h>
#include "vas_util.h"
#include "vas_dynamicFirChannel.h"

void vas_fixedFirChannel_process(vas_dynamicFirChannel *x, VAS_INPUTBUFFER *in, VAS_OUTPUTBUFFER *out, int vectorSize, int flags, void *data);

#endif /* vas_fixedFirChannel_h */
