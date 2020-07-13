#pragma once

// 1C
class BSThread
{
public:
	virtual void Destroy(bool bFreeMem) = 0;
	virtual void Run(UInt32 arg0, UInt32 arg1) = 0;

	BSThread();
	~BSThread();

	// void			** vtbl;
	LPCRITICAL_SECTION		criticalSection;	// 04
	UInt32					unk08;				// 08
	UInt32					unk0C;				// 0C
	UInt32					unk10;				// 10
	UInt32					unk14;				// 14
	UInt8					unk18;				// 18
	UInt8					pad19[3];
};

// 44
class BackgroundLoaderThread : public BSThread
{
public:
	BackgroundLoaderThread();
	~BackgroundLoaderThread();

	struct Semaphore {
		LONG		initialCount;
		LONG		maximumCount;
		HANDLE		handle;
	};

	UInt32			unk1C;			// 1C init'd to c'tor arg0
	Semaphore		semaphores[3];	// 20
};

// 10
class BackgroundLoader
{
public:
	virtual void Destroy(bool bFreeMem) = 0;
	virtual void Unk_01(void) = 0;
	virtual void Unk_02(void) = 0;
	virtual void Unk_03(void) = 0;

	BackgroundLoader();
	~BackgroundLoader();

	// void			** vtbl
	UInt8					unk04;			// 04
	UInt8					pad05[3];
	UInt32					unk08;			// 08
	BackgroundLoaderThread	* thread;		// 0C
};

