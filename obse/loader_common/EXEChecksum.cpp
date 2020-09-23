#include "EXEChecksum.h"
#include "common/IFileStream.h"
#include "Options.h"
#include <direct.h>

// based on some code from wikipedia
static UInt32 Adler32(UInt8 * buf, UInt32 len)
{
	static const UInt32 kModAdler = 65521;

	UInt32 a = 1, b = 0;

	while (len) {
		unsigned tlen = len > 5550 ? 5550 : len;
		len -= tlen;

		do {
			a += *buf++;
			b += a;
		} while (--tlen);

		a = (a & 0xffff) + (a >> 16) * (65536-kModAdler);
		b = (b & 0xffff) + (b >> 16) * (65536-kModAdler);
	}

	/* It can be shown that a <= 0x1013a here, so a single subtract will do. */
	if (a >= kModAdler) a -= kModAdler;

	/* It can be shown that b can reach 0xffef1 here. */
	b = (b & 0xffff) + (b >> 16) * (65536-kModAdler);
	if (b >= kModAdler) b -= kModAdler;

	return (b << 16) | a;
}

// from pefile
static UInt32 CalcEXEChecksum(UInt8 * buf, UInt32 length, UInt32 checksumOffset)
{
	UInt64	checksum = 0;
	UInt32	* buf32 = (UInt32 *)buf;

	checksumOffset /= 4;

	for(UInt32 i = 0; i < length / 4; i++)
	{
		if(i == checksumOffset) continue;	// checksum field ignored

		checksum = (checksum & 0xFFFFFFFF) + buf32[i] + (checksum >> 32);
		checksum = (checksum & 0xFFFFFFFF) + (checksum >> 32);
	}

	checksum = (checksum & 0xFFFF) + (checksum >> 16);
	checksum = checksum + (checksum >> 16);
	checksum = checksum & 0xFFFF;

	checksum = checksum + length;

	return checksum;
}

static void Clear2GBAware(UInt8 * buf, UInt32 bufLen)
{
	// *really* clear 2GB+ address-aware flag (this version actually works)
	if(bufLen > 0x40)
	{
		UInt32	headerOffset = *((UInt32 *)(buf + 0x3C)) + 4;	// +4 to skip 'PE\0\0'
		UInt32	flagsOffset = headerOffset + 0x12;
		UInt32	checksumOffset = headerOffset + 0x14 + 0x40;	// +14 to skip COFF header

		if(bufLen > checksumOffset + 2)
		{
			UInt16	* flagsPtr = (UInt16 *)(buf + flagsOffset);
			UInt32	* checksumPtr = (UInt32 *)(buf + checksumOffset);

			if(*flagsPtr & 0x0020)
			{
				_MESSAGE("clearing large-address-aware flag (flags offset = %08X checksum offset = %08X)", flagsOffset, checksumOffset);

				// clear it, recalculate the exe checksum
				*flagsPtr &= ~0x0020;

				UInt32	newChecksum = CalcEXEChecksum(buf, bufLen, checksumOffset);

				// did the tool fix up the checksum?
				if(*checksumPtr != newChecksum)
				{
					// yes, set it back
					_MESSAGE("recorrecting exe checksum (%08X -> %08X)", *checksumPtr, newChecksum);

					*checksumPtr = newChecksum;
				}
			}
		}
	}

	// ### stupid original total hack attempt to clear 2GB+ address flag
	// ### this doesn't work correctly, but we have to leave it here
	// ### otherwise the CRCs may change
	if(bufLen > 0x13E)
		buf[0x13E] &= ~0x20;
}

bool TestChecksum(const char * procName, std::string * dllSuffix, ProcHookInfo * hookInfo)
{
	bool		result = false;
	IFileStream	src;

	hookInfo->hookCallAddr = 0;
	hookInfo->loadLibAddr = 0;
	hookInfo->steamVersion = false;

	if(src.Open(procName))
	{
		UInt8	* buf = new UInt8[src.GetLength()];
		ASSERT(buf);
		src.ReadBuf(buf, src.GetLength());

		Clear2GBAware(buf, src.GetLength());

		UInt32	crc = Adler32(buf, src.GetLength());

		delete [] buf;

		// make sure the crc shows up in the console when run with -crconly
		if(g_options.m_crcOnly)
			_FATALERROR("crc = %08X", crc);
		else
			_MESSAGE("crc = %08X", crc);

		if(g_options.m_launchCS)
		{
			switch(crc)
			{
				case 0x96ED4409:	// 1.0.0.303
					*dllSuffix = "1_0";
					hookInfo->hookCallAddr = 0x008AE5CB;
					hookInfo->loadLibAddr = 0x0091C108;
					result = true;
					break;

				case 0x8F05C3AD:	// 1.2.0.0 - first release, had source control enabled
				case 0xE380C3AF:	// 1.2.0.0 - second release (almost binary-identical)
				case 0x2F9AC10C:	// 1.2.0.404 - third release, again almost binary-identical
				case 0x6D1A8291:	// 1.2.0.404 - japanese unofficial translation
				case 0x5BF7D114:	// 1.2.0.404 - modified to enable visual styles, bundled with CSE plugin
					*dllSuffix = "1_2";
					hookInfo->hookCallAddr = 0x00892BDE;
					hookInfo->loadLibAddr = 0x00924158;
					result = true;
					break;

				default:	// unknown checksum
					PrintError("You have an unknown version of the CS. Please check http://obse.silverlock.org to make sure you're using the latest version of OBSE, then send an email to the contact addresses listed in obse_readme.txt if this version is not supported in the latest release. (CRC = %08X)", crc);
					break;
			}
		}
		else
		{
			switch(crc)
			{
				case 0x4998C72A:	// 1.1.0.511 english official
				case 0x8174C8F4:	// 1.1.0.511 english w/ additional patch
				case 0xC857DB78:	// 1.1.0.511 english w/ no-CD patch by TNT
				case 0xE512C810:	// 1.1.0.511 english w/ no-CD patch by SYNapSiS
				case 0x31722297:	// 1.1.0.511 cheesy russian patch (doesn't seem to be official)
				case 0x08FFC506:	// 1.1.0.511 english w/ no-CD patch by cirdan patch #2
				case 0xD240C4E6:	// 1.1.0.511 english w/ no-CD patch
				case 0x9329CBBB:	// 1.1.0.511 english w/ no-CD patch
				case 0x278B2356:	// 1.1.0.511 japanese unofficial v5
				case 0xAC232487:	// 1.1.0.511 polish unofficial
				case 0x29F82D28:	// 1.1.0.511 russian unofficial w/ no-CD "<B@ZiK was here!>"
				case 0x7F601478:	// 1.1.0.511 japanese unofficial v6
				case 0x5D4D091D:	// 1.1.0.511 japanese unofficial v7c
				case 0x9F9F923B:	// 1.1.0.511 japanese unofficial v9 (apparently no v8)
				case 0x292A8DA6:	// 1.1.0.511 japanese unofficial v9a
				case 0x0ACF90F4:	// 1.1.0.511 japanese unofficial v9b
				case 0x344D33A1:	// 1.1.0.511 japanese unofficial v10
				case 0x14379736:	// 1.1.0.511 japanese unofficial v11
				case 0x3E5322AD:	// 1.1.0.511 japanese unofficial v12, v12a
				case 0x14A722A7:	// 1.1.0.511 japanese unofficial v12b
				case 0xD43DCD39:	// 1.1.0.511 japanese unofficial v13, v13a
				case 0xE82804EE:	// 1.1.0.511 japanese unofficial v14
				case 0xEDBE0C74:	// 1.1.0.511 japanese unofficial v14a
				case 0xEA7E7121:	// 1.1.0.511 japanese unofficial v15
					PrintError("You are running the 1.1.0.511 version of Oblivion which is not supported by versions of OBSE from v0017 onward. Please upgrade to 1.2.0.416 or use an older version of OBSE.");
					break;

				case 0x3F6987B1:	// 1.1.0.425 beta patch
					PrintError("You are using the 1.1.0.425 beta patch of Oblivion, please upgrade to 1.2.0.416.");
					break;

				case 0x92FE2FC9:	// 0.1.0.228 original w/ no-CD patch by *!ReLOADeD!*
				case 0x01D42F77:	// 0.1.0.228 original
				case 0x2CDBAC93:	// 0.1.0.228 japanese unofficial v7c
				case 0xB9572CEF:	// 0.1.0.228 original w/ no-CD patch by cirdan
					PrintError("You are using the original 0.1.0.228 version of Oblivion, please upgrade to 1.2.0.416.");
					break;

				case 0x608CA512:	// 1.1.0.511 english direct2drive
				case 0xAE9944C0:	// 1.2.something english direct2drive
				case 0x619C941B:	// 1.2.0.416 shivering isles direct2drive
				case 0x8D5DB3CC:	// 1.2.0.416 direct2drive
					PrintError("Direct2drive versions of Oblivion cannot be supported due to copy protection applied to the executable. If you have a retail version of Oblivion (a physical disk), go to http://obse.silverlock.org and use the autopatcher to extract a usable EXE.");
					break;

				case 0xACB74E24:	// 1.2.0.214 english official - installed by the 1.2 patch
				case 0x46A74FEE:	// 1.2.0.214 english w/ no-CD patch by unknown
				case 0x10D34E25:	// 1.2.0.214 english w/ no-CD patch by refraction
				case 0xBF924BD4:	// 1.2.0.214 english w/ no-CD patch by unknown
				case 0x33F353A2:	// 1.2.0.214 english w/ no-CD patch by unleashed
				case 0xBD765745:	// 1.2.0.214 english w/ no-CD patch by TNT
				case 0xE116E23D:	// 1.2.0.214 japanese unofficial v7c
				case 0x78AEE28E:	// 1.2.0.214 japanese unofficial v7d (hello 2ch)
				case 0x960EE2AE:	// 1.2.0.214 japanese unofficial v7e
				case 0x7B5ED597:	// 1.2.0.214 polish unofficial
					PrintError("You are using the 1.2.0.214 version of Oblivion.exe, which is not supported by versions of OBSE from v0017 onward. Please upgrade to 1.2.0.416 or use an older version of OBSE.");
					break;

				case 0xE8B71FA4:	// 1.2.0.201 english official - installed by Shivering Isles
				case 0xF6E56D02:	// 1.2.0.201 w/ no-CD patch
					PrintError("You are using the 1.2.0.201 version of Oblivion installed by Shivering Isles, please upgrade to 1.2.0.416.");
					break;

				case 0x6C9D751B:	// 1.2.0.410 beta patch (bugfix for refid generation)
				case 0xE17F0878:	// 1.2.0.410 japanese unofficial v7d
				case 0xFEDF0898:	// 1.2.0.410 japanese unofficial v7e
					PrintError("You are using the 1.2.0.410 beta patch, which is not supported. There is no way to directly patch from 1.2.0.410 to any other version of Oblivion, so please check the readme for instructions on using the autopatcher to upgrade to 1.2.0.416.");
					break;

				case 0x7934C86C:	// 1.2.0.416 patch - released as both beta and final
				case 0x3E51C65C:	// 1.2.0.416 w/ no-CD patch by pwz "osi6"
				case 0x59F0C64A:	// 1.2.0.416 w/ no-CD patch by pwz "osi6" (with proper PE checksum, only shows up when people set large-address-aware)
				case 0x9339C8B3:	// 1.2.0.416 w/ no-CD patch by pwz "si46"
				case 0xEEA8C8FA:	// 1.2.0.416 w/ no-CD patch by pwz "si46" (proper PE checksum)
				case 0xE7BBCA0E:	// 1.2.0.416 w/ no-CD patch by pwz "si40" - dude, it's the SAME BASE EXE, WHY CRACK IT DIFFERENTLY THREE TIMES?
				case 0x01B0C9B2:	// 1.2.0.416 w/ no-CD patch by pwz "si40" (proper PE checksum)
				case 0x3ECC5E59:	// 1.2.0.416 japanese unofficial v7e
				case 0x3D4EC785:	// 1.2.0.416 german unofficial
				case 0x6E81C79D:	// 1.2.0.416 german unofficial (needed proper PE checksum)
				case 0x2B4DD302:	// 1.2.0.416 russian official?
				case 0x9EAAD2A3:	// 1.2.0.416 russian official? (needed proper PE checksum, wtf)
				case 0xD90B4B42:	// 1.2.0.416 polish
				case 0x07954D69:	// 1.2.0.416 polish (no idea how to calc checksum? just zero it. way to go guys)
				case 0x181B8C89:	// 1.2.0.416 japanese unofficial v9 (apparently no v8)
				case 0xE56C9816:	// 1.2.0.416 japanese unofficial v9a
				case 0x3E9FC1F7:	// 1.2.0.416 japanese unofficial v9b
				case 0xC9A9C9AD:	// 1.2.0.416 w/ no-CD patch by unknown
				case 0x1087C9EF:	// 1.2.0.416 w/ no-CD patch by unknown (proper PE checksum)
				case 0x52AE527E:	// 1.2.0.416 japanese unofficial v10
				case 0xABD5C927:	// 1.2.0.416 german unofficial
				case 0x1EDBC9E2:	// 1.2.0.416 german unofficial (laa)
				case 0x97D6D510:	// 1.2.0.416 russian official? http://games.1c.ru/oblivion_gold/
				case 0x81AAD4C1:	// 1.2.0.416 russian official? (proper PE checksum, wtf)
				case 0xAD9CD911:	// 1.2.0.416 russian official + no-CD
				case 0xAC0BD8C7:	// 1.2.0.416 russian official + no-CD (proper PE checksum)
				case 0xFB9FC7DF:	// 1.2.0.416 w/ no-CD patch by firstwave
				case 0xA43CC851:	// 1.2.0.416 w/ no-CD patch by firstwave (proper PE checksum)
				case 0x345A515B:	// 1.2.0.416 japanese unofficial v11
				case 0xA83413FD:	// 1.2.0.416 japanese unofficial v12, v12a
				case 0x7EE813F7:	// 1.2.0.416 japanese unofficial v12b
				case 0x9B20B8E1:	// 1.2.0.416 japanese unofficial v13
				case 0x175EB699:	// 1.2.0.416 japanese unofficial v13a
				case 0x8DFFDD6C:	// 1.2.0.416 japanese unofficial v14
				case 0xCF5AE53E:	// 1.2.0.416 japanese unofficial v14a
				case 0x2908CA36:	// 1.2.0.416 w/ no-CD patch by dockery
				case 0xE9A7CA02:	// 1.2.0.416 w/ no-CD patch by dockery (proper PE checksum)
				case 0x3DBB32D1:	// 1.2.0.416 japanese unofficial v15
				case 0x7CD1332D:	// 1.2.0.416 japanese unofficial v15 (proper PE checksum)
				case 0xDA1EC605:	// 1.2.0.416 hungarian unofficial
				case 0x4F11C5C5:	// 1.2.0.416 hungarian unofficial v2
				case 0xE243684D:    // 1.2.0.416 GOG version
				case 0x299C4CB4:    // 1.2.0.416 GOTY Polish version by RPGowo 
					*dllSuffix = "1_2_416";
					result = true;
					break;

				case 0x86005CC4:	// 1.2.0.416 polish reprotected/packed no-CD patch by urbi
					// this might actually work, but I'm not going anywhere near the EXE
					// why bother packing it again?! it's probably a FIVE BYTE PATCH
					PrintError("You are using a very strange packed version of Oblivion. Please check the readme for instructions on using the autopatcher to replace your EXE with a clean version of 1.2.0.416.");
					break;

				case 0xD9C8B395:	// 1.2.0.416 steam (old version)
					PrintError("Please enable updates on your Steam copy of Oblivion, then relaunch the game to add support for OBSE.");
					break;

				case 0xA2408F04:	// 1.2.0.416 steam version with steamworks compatibility mode (effectively the same as retail)
					*dllSuffix = "1_2_416";
					result = true;
					hookInfo->steamVersion = true;
					break;

				default:	// unknown checksum
					PrintError("You have an unknown version of Oblivion. Please check http://obse.silverlock.org to make sure you're using the latest version of OBSE, then send an email to the contact addresses listed in obse_readme.txt if this version is not supported in the latest release. (CRC = %08X)", crc);
					break;
			}
		}
	}
	else
	{
		PrintError("Couldn't find %s, make sure you're running this from the same folder as your oblivion.exe.", procName);
	}

	return result;
}

// this doesn't really belong here, need to see how best to report errors when being launched via steam
void PrintError(const char * fmt, ...)
{
	va_list	args;

	va_start(args, fmt);

	gLog.Log(IDebugLog::kLevel_FatalError, fmt, args);

	char	buf[4096];
	vsprintf_s(buf, sizeof(buf), fmt, args);

	MessageBox(NULL, buf, "OBSE Loader", MB_OK | MB_ICONEXCLAMATION);

	va_end(args);
}

std::string GetCWD(void)
{
	char	cwd[4096];

	ASSERT(_getcwd(cwd, sizeof(cwd)));

	return cwd;
}
