#ifndef __OBSE_VERSION_H__
#define __OBSE_VERSION_H__

// these have to be macros so they can be used in the .rc
#define OBSE_VERSION_INTEGER		22
#define OBSE_VERSION_INTEGER_MINOR	11   //Avoid using revision 0  (at least 1 mod break for this)
#define OBSE_VERSION_INTEGER_HOTIFX 0
#define OBSE_VERSION_VERSTRING		"0, 22, 11, 0"
#define OBSE_VERSION_PADDEDSTRING	"0022"

// build numbers are the month and date each build was made
#define MAKE_OBLIVION_VERSION(major, minor, build) (((major & 0xFF) << 24) | ((minor & 0xFF) << 16) | (build & 0xFFFF))

#define OBLIVION_VERSION_1_2_416 MAKE_OBLIVION_VERSION(1, 2, 416)	// 0x010201A0

#define CS_VERSION_1_0 MAKE_OBLIVION_VERSION(1, 0, 303)				// 0x0100012F
#define CS_VERSION_1_2 MAKE_OBLIVION_VERSION(1, 2, 0)				// 0x01020000

#endif /* __OBSE_VERSION_H__ */
