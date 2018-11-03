#ifndef ALHELPERS_H
#define ALHELPERS_H

#include "AL/alc.h"
#include "AL/al.h"
#include "AL/alext.h"

#include "threads.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Some helper functions to get the name from the format enums. */
const char *FormatName(ALenum type);

/* Easy device init/deinit functions. InitAL returns 0 on success. */
int InitAL(char ***argv, int *argc);
void CloseAL(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ALHELPERS_H */
