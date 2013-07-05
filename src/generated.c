// Principal ADL header include
#include "adl_global.h"
// List of embedded components
const char mos_headerSEList[] =
"Developer Studio\0" "2.2.0.201205091434-R9457\0"
"Open AT Framework package\0" "2.35.5.201206121412\0"
"Open AT OS Package\0" "6.37.0.201202060950\0"
"Firmware Package\0" "7.45.5.201201230652\0"
"\0";

#if __OAT_API_VERSION__ >= 636
// Application debug/release mode tag (only supported from Open AT OS 6.36)
#ifdef DS_DEBUG
const adl_CompilationMode_e adl_CompilationMode = ADL_COMPILATION_MODE_DEBUG;
#else
const adl_CompilationMode_e adl_CompilationMode = ADL_COMPILATION_MODE_RELEASE;
#endif
#endif

// Application name definition
const ascii adl_InitApplicationName[] = "berpaket_lancar";

// Company name definition
const ascii adl_InitCompanyName[] = "dbe.Abadhy";

// Application version definition
const ascii adl_InitApplicationVersion[] = "1.0.0";
