#pragma once

// thanks to intel for the free license
// oblivion is built with havok 311, which is earlier than the public version (550)
// so let's hope there wasn't much code flux between the two versions

// looks like prefixes were added to class names, for example:
// hkp = physics
// hka = animation
// remove these so the source matches

class hkStatisticsCollector;

// 04
class hkBaseObject
{
public:
	virtual ~hkBaseObject();

//	void	** m_vtbl;	// 00
};

// 08
class hkReferencedObject : public hkBaseObject
{
public:
	virtual void	calcStatistics(hkStatisticsCollector * collector);

	UInt16	m_sizeAndFlags;	// 04
	UInt16	m_refCount;		// 06
};

STATIC_ASSERT(sizeof(hkReferencedObject) == 0x08);
