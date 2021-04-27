#pragma once

#include "ICriticalSection.h"
#include "IEvent.h"
#include "IInterlockedLong.h"

class IReadWriteLock
{
	public:
		IReadWriteLock();
		~IReadWriteLock();

		void	StartRead(void);
		void	EndRead(void);
		void	StartWrite(void);
		void	EndWrite(void);

	private:
		IEvent				readBlocker;
		IEvent				writeBlocker;
		ICriticalSection	enterBlocker;
		ICriticalSection	writeMutex;
		IInterlockedLong	readCount;
};
