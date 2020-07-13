#include "Commands_PathGrid.h"
#include "ParamInfos.h"
#include "ScriptUtils.h"

#if OBLIVION
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "ArrayVar.h"

/* Path grid nodes don't have formIDs. We need a way for scripts to identify nodes, but scripts
	may be interested in nodes outside of a single pathgrid when dealing with exterior cells.
	So NodeID provides a uniform way for scripts to refer to nodes by integer ID, relative to the pathgrid
	of the player's current cell. ID includes index into the local pathgrid's array of nodes, plus
	offset from the current pathgrid.
	For obvious reasons NodeID's become invalid when the player changes cells.
*/

TESObjectCELL* GetCurrentCell()
{
	return (*g_thePlayer)->parentCell;
}

TESObjectCELL* GetAbsoluteCell(float x, float y)
{
	TESWorldSpace* world = GetCurrentCell()->worldSpace;
	if (world) {
		SInt32 sx = x;
		SInt32 sy = y;
		return world->LookupCell(sx >> 12, sy >> 12);
	}
	else {
		return NULL;
	}
}

TESObjectCELL* GetRelativeCell(SInt32 dx, SInt32 dy)
{
	TESObjectCELL* cur = GetCurrentCell();
	if (cur->IsInterior() || !cur->coords) {
		return (dx == 0 && dy == 0) ? cur : NULL;
	}
	else if (dx == 0 && dy == 0) {
		return cur;
	}
	else if (cur->worldSpace) {
		dx += cur->coords->x;
		dy += cur->coords->y;
		return cur->worldSpace->LookupCell(dx, dy);
	}

	return NULL;
}

class NodeID
{
	struct  {
		// cell offset is sxxxsyyy where xxx is x offset, yyy is y offset, s is sign (1=negative)
		UInt16		localIndex;
		UInt8		cellOffset;
		UInt8		pad;		// don't want to use upper 8 bits to prevent bits getting chopped off on conversion to double

		UInt32	ToInteger() const { return *((UInt32*)this); }
	}	m_id;

public:
	NodeID(UInt16 idx, SInt8 cellDX, SInt8 cellDY) {
		if (idx == -1) {	// identifies an invalid node index e.g. returned by TESPathGrid::AddNode()
			m_id.localIndex = -1;
			m_id.cellOffset = -1;
			m_id.pad = -1;
			return;
		}

		m_id.localIndex = idx;
		m_id.cellOffset = 0;
		if (cellDX < 0) {
			m_id.cellOffset |= 0x80;
			cellDX *= -1;
		}
		if (cellDY < 0) {
			m_id.cellOffset |= 0x08;
			cellDY *= -1;
		}

		m_id.cellOffset |= cellDY | (cellDX << 4);
		m_id.pad = 0;
	}

	NodeID() {
		// init to node ID -1
		m_id.localIndex = -1;
		m_id.cellOffset = -1;
		m_id.pad = -1;
	}

	UInt32 GetIntegerID() const { return  m_id.ToInteger(); }

	UInt16 GetIndex() const { return m_id.localIndex; }

	TESPathGridPoint* GetPoint() const {
		TESPathGrid* grid = GetParentGrid();
		UInt16 idx = GetIndex();
		if (grid && idx < grid->nodeCount) {
			return grid->nodes->data[idx];
		}

		return NULL;
	}

	SInt8 GetYOffset() const {
		SInt8 y = m_id.cellOffset & 0x07;
		if (m_id.cellOffset & 0x08) {
			y *= -1;
		}
		return y;
	}

	SInt8 GetXOffset() const {
		SInt8 x = (m_id.cellOffset >> 4) & 0x07;
		if (m_id.cellOffset & 0x80) {
			x *= -1;
		}
		return x;
	}

	TESObjectCELL* GetParentCell() const {
		return GetRelativeCell(GetXOffset(), GetYOffset());
	}

	TESPathGrid* GetParentGrid() const {
		TESObjectCELL* cell = GetParentCell();
		return cell ? cell->pathGrid : NULL;
	}

	bool IsValidPoint() {
		return GetIntegerID() != -1;
	}

	bool operator<(const NodeID& rhs) const { return GetIntegerID() < rhs.GetIntegerID(); }
	bool operator==(const NodeID& rhs) const { return GetIntegerID() == rhs.GetIntegerID(); }
};

STATIC_ASSERT(sizeof(NodeID) == sizeof(UInt32));

/* Cmds */

static bool Cmd_IsPathNodeDisabled_Execute(COMMAND_ARGS)
{
	NodeID node;
	*result = -1.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &node)) {
		TESPathGridPoint* pt = node.GetPoint();
		if (pt) {
			*result = pt->IsDisabled() ? 1.0 : 0.0;
		}
	}

	return true;
}

static bool Cmd_GetPathNodePos_Execute(COMMAND_ARGS)
{
	NodeID node;
	UInt32 axis = 0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &node, &axis)) {
		TESPathGridPoint* pt = node.GetPoint();
		if (pt) {
			switch (axis) {
				case 'X':
				case 'x':
					*result = pt->x;
					break;
				case 'Y':
				case 'y':
					*result = pt->y;
					break;
				case 'Z':
				case 'z':
					*result = pt->z;
					break;
			}
		}
	}

	return true;
}

static bool Cmd_SetPathNodeDisabled_Execute(COMMAND_ARGS)
{
	NodeID node;
	UInt32 bDisable = 1;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &node, &bDisable)) {
		TESPathGrid* grid = node.GetParentGrid();
		if (grid) {
			*result = grid->SetPointDisabled(node.GetIndex(), bDisable ? true : false) ? 1.0 : 0.0;
		}
	}

	return true;
}

static bool Cmd_SetPathNodePreferred_Execute(COMMAND_ARGS)
{
	NodeID node;
	UInt32 bPreferred = 1;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &node, &bPreferred)) {
		TESPathGrid* grid = node.GetParentGrid();
		if (grid) {
			*result = grid->SetPointPreferred(node.GetIndex(), bPreferred ? true : false) ? 1.0 : 0.0;
		}
	}

	return true;
}

static bool Cmd_GetAllPathNodes_Execute(COMMAND_ARGS)
{
	// a test command
	SInt32 cellDepth = 0;
	UInt32 bIncludeDisabled = 0;
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cellDepth, &bIncludeDisabled) && cellDepth >= 0) {
		TESObjectCELL* cur = GetCurrentCell();
		SInt32 cellX = 0;
		SInt32 cellY = 0;
		if (cur->IsInterior()) {
			cellDepth = 0;
			cellX = 0;
			cellY = 0;
		}
		else {
			cellX = cur->coords->x;
			cellY = cur->coords->y;
		}

		double curIdx = 0.0;
		for (SInt32 dx = -cellDepth; dx <= cellDepth; dx++) {
			for (SInt32 dy = -cellDepth; dy <= cellDepth; dy++) {
				cur = GetRelativeCell(dx, dy);
				if (cur && cur->pathGrid && cur->pathGrid->nodeCount) {
					for (UInt32 i = 0; i < cur->pathGrid->nodeCount; i++) {
						TESPathGridPoint* pt = cur->pathGrid->nodes->data[i];
						if (pt && (bIncludeDisabled || !pt->IsDisabled())) {
							g_ArrayMap.SetElementNumber(arr, curIdx, NodeID(i, dx, dy).GetIntegerID());
							curIdx += 1.0;
						}
						if (!pt) {
							DEBUG_PRINT("	* Found a null path grid point");
						}
					}
				}
			}
		}
	}

	return true;
}

static bool Cmd_PathEdgeExists_Execute(COMMAND_ARGS)
{
	NodeID a;
	NodeID b;
	*result = 0.0;
	if (ExtractArgs(PASS_EXTRACT_ARGS, &a, &b)) {
		TESPathGridPoint* pa = a.GetPoint();
		TESPathGridPoint* pb = b.GetPoint();

		if (a.GetParentGrid() == b.GetParentGrid()) {
			tList<TESPathGridPoint>::Iterator iter = pa->edges.Begin();
			while (!iter.End()) {
				if (iter.Get() == pb) {
					break;
				}
				++iter;
			}

			if (!iter.End()) {
				*result = 1.0;
			}
		}
		else {
			TESPathGrid* grid = a.GetParentGrid();
			UInt16 aID = a.GetIndex();
			if (grid && !grid->externalEdges.IsNull()) {
				TESPathGrid::ExternalEdgeList::Iterator iter = grid->externalEdges->Begin();
				while (!iter.End()) {
					TESPathGrid::ExternalEdge* edge = iter.Get();
					if (edge->localNodeID == aID) {
						if (FloatEqual(edge->x, pb->x) && FloatEqual(edge->y, pb->y) && FloatEqual(edge->z, pb->z)) {
							break;
						}
					}
					++iter;
				}

				if (!iter.End()) {
					*result = 1.0;
				}
			}
		}
	}

	return true;
}

static bool Cmd_GetPathNodeLinkedRef_Execute(COMMAND_ARGS)
{
	NodeID node;
	UInt32* refResult = (UInt32*)result;
	*refResult = 0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &node)) {
		TESPathGrid* grid = node.GetParentGrid();
		if (grid) {
			TESObjectREFR* refr = grid->GetLinkedRef(node.GetPoint());
			if (refr) {
				*refResult = refr->refID;
			}
		}
	}

	return true;
}

static bool Cmd_AddPathNode_Execute(COMMAND_ARGS)
{
	float x, y, z;
	UInt32 bPreferred = 0;
	NodeID newNode;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &x, &y, &z, &bPreferred)) {
		TESObjectCELL* cell = GetAbsoluteCell(x, y);
		if (cell && cell->pathGrid) {
			SInt32 dx = 0;
			SInt32 dy = 0;
			if (!cell->IsInterior() && cell->coords) {
				TESObjectCELL* curCell = (*g_thePlayer)->parentCell;
				if (curCell && curCell->coords && curCell != cell) {
					dx = cell->coords->x - curCell->coords->x;
					dy = cell->coords->y - curCell->coords->y;
				}
			}

			UInt16 idx = cell->pathGrid->AddNode(x, y, z, bPreferred ? true : false);
			if (idx != -1) {
				newNode = NodeID(idx, dx, dy);
			}
		}
	}

	return true;
}

// TESTING cmd
struct Edge {
	NodeID	from;
	NodeID	to;
	Edge(NodeID f, NodeID t) {
		if (f < t) {
			from = f;
			to = t;
		}
		else {
			from = t;
			to = f;
		}
	}

	bool operator==(const Edge& rhs) const {
		return (from == rhs.from) && (to == rhs.to);
	}

	bool operator<(const Edge& rhs) const {
		if (from == rhs.from) {
			return to < rhs.to;
		}
		else {
			return from < rhs.from;
		}
	}
};

static bool Cmd_SetPathEdgeExists_Execute(COMMAND_ARGS)
{
	NodeID nodeA;
	NodeID nodeB;
	UInt32 bEnableEdge = 0;
	*result = 0.0;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &nodeA, &nodeB, &bEnableEdge)) {
		// ###TODO: check null grid ptrs, invalid node IDs
		if (nodeA.GetParentGrid()->SetEdge(nodeA.GetIndex(), nodeB.GetIndex(), bEnableEdge ? true : false, nodeB.GetParentGrid())) {
			*result = 1.0;
		}
	}

	return true;
}

#include <set>

class EdgeRecorder {
	std::set<Edge> &	m_edges;
	NodeID				m_nodeID;
	TESPathGrid			* m_grid;
	TESObjectCELL		* m_cell;
public:
	EdgeRecorder(std::set<Edge> & edges, UInt16 nodeID, TESObjectCELL* cell) : m_edges(edges), m_cell(cell) {
		m_grid = cell->pathGrid;
		m_nodeID = NodeID(nodeID, 0, 0);
	}

	bool Accept(const TESPathGridPoint* pt) {
		TESPathGridPoint* to = const_cast<TESPathGridPoint*>(pt);
		TESObjectCELL* cell = (m_cell->IsInterior()) ? m_cell : GetAbsoluteCell(to->x, to->y);
		if (cell && cell->pathGrid) {
			SInt32 dx = 0;
			SInt32 dy = 0;
			if (!cell->IsInterior() && cell->coords) {
				dx = cell->coords->x - m_cell->coords->x;
				dy = cell->coords->y - m_cell->coords->y;
			}
			m_edges.insert(Edge(m_nodeID, NodeID(cell->pathGrid->IndexOf(to), dx, dy)));
		}

		return true;
	}
};

static bool Cmd_GetAllPathEdges_Execute(COMMAND_ARGS)
{
	ArrayID arr = g_ArrayMap.Create(kDataType_String, false, scriptObj->GetModIndex());
	*result = arr;

	TESPathGrid* grid = (*g_thePlayer)->parentCell->pathGrid;
	if (grid) {
		ArrayID inArr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
		if (grid->nodes) {
			std::set<Edge> internalEdges;
			for (UInt32 i = 0; i < grid->nodeCount; i++) {
				TESPathGridPoint* pt = grid->nodes->data[i];
				if (pt) {
					pt->edges.Visit(EdgeRecorder(internalEdges, i, (*g_thePlayer)->parentCell));
				}
			}

			double idx = 0.0;
			for (std::set<Edge>::iterator iter = internalEdges.begin(); iter != internalEdges.end(); ++iter) {
				ArrayID edgeArr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
				g_ArrayMap.SetElementNumber(edgeArr, 0.0, iter->from.GetIntegerID());
				g_ArrayMap.SetElementNumber(edgeArr, 1.0, iter->to.GetIntegerID());
				g_ArrayMap.SetElementArray(inArr, idx, edgeArr);
				idx += 1.0;
			}
		}

		g_ArrayMap.SetElementArray(arr, "internal", inArr);

		// External edges
		/*
		ArrayID exArr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
		if (grid->externalEdges) {
			double idx = 0.0;
			for (tList<TESPathGrid::ExternalEdge>::Iterator iter = grid->externalEdges->Begin(); !iter.End(); ++iter) {
				ArrayID edgeArr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
				g_ArrayMap.SetElementNumber(edgeArr, 0.0, iter->localNodeID);
				g_ArrayMap.SetElementNumber(edgeArr, 1.0, iter->x);
				g_ArrayMap.SetElementNumber(edgeArr, 2.0, iter->y);
				g_ArrayMap.SetElementNumber(edgeArr, 3.0, iter->z);
				g_ArrayMap.SetElementArray(exArr, idx, edgeArr);
				idx += 1.0;
			}
		}
		g_ArrayMap.SetElementArray(arr, "external", exArr);
		*/
	}

	return true;
}

static const float fCellExtent = 2048.0;
static const float fCellRadius = 2896.31;
static const float fCellRadiusSquared = fCellExtent * fCellExtent * 2;
static const float fCellDimension = 4096.0;

static const float fPartitionExtent = 256.0;
static const float fPartitionRadius = 362.04;
static const float fPartitionRadiusSquared = fPartitionExtent * fPartitionExtent * 2;
static const float fPartitionDimension = 512.0;

struct Rect
{
	Rect(const Vector2& c, const Vector2& e, float a) : center(c), extents(e), rot(a) { }
	Rect(float x, float y, float ex, float ey, float a) : center(x, y), extents(ex, ey), rot(a) { }

	Vector2		center;
	Vector2		extents;
	float		rot;
};

struct AABBPathGridArea
{
	AABBPathGridArea(const Rect& area) : m_area(area) { }

	Rect		m_area;

	Rect GetBoundingRect() const { return m_area; }
	bool LooselyIntersectsAABB(const Rect& aabb) const {
		if ((m_area.center.x + m_area.extents.x) < (aabb.center.x - aabb.extents.x) ||
			(m_area.center.y + m_area.extents.y) < (aabb.center.y - aabb.extents.y) ||
			(m_area.center.x - m_area.extents.x) > (aabb.center.x + aabb.extents.y) ||
			(m_area.center.y - m_area.extents.y) > (aabb.center.y + aabb.extents.y)) {
				return false;
		}
		else {
			return true;
		}
	}

	bool IntersectsPoint(const Vector2& point) const {
		return	(point.x > (m_area.center.x - m_area.extents.x)) &&
				(point.x < (m_area.center.x + m_area.extents.x)) &&
				(point.y > (m_area.center.y - m_area.extents.y)) &&
				(point.y < (m_area.center.y + m_area.extents.y));
	}

	const Vector2& GetCenter() const { return m_area.center; }
};

struct OBBPathGridArea
{
	OBBPathGridArea(const Rect& area) : m_area(area)
	{
		// precompute sin/cos
		m_cosNegTheta = cos(-area.rot);
		m_sinNegTheta = sin(-area.rot);

		m_centerLocal = Vector2(area.center.x*m_cosNegTheta - area.center.y*m_sinNegTheta,
								area.center.x*m_sinNegTheta + area.center.y*m_cosNegTheta);

		// precompute and store min/max intervals along x/y axes for later use
		float cosTheta = cos(area.rot);
		float sinTheta = sin(area.rot);
		Vector2 X(cosTheta*area.extents.x, sinTheta*area.extents.x);
		Vector2 Y((-sinTheta)*area.extents.y, cosTheta*area.extents.y);
		Vector2 corners[4] = {
			Vector2(area.center.x - X.x - Y.x, area.center.y - X.y - Y.y),
			Vector2(area.center.x + X.x - Y.x, area.center.y + X.y - Y.y),
			Vector2(area.center.x + X.x + Y.x, area.center.y + X.y + Y.y),
			Vector2(area.center.x - X.x + Y.x, area.center.y - X.y + Y.y)
		};

		m_minX = corners[0].x;
		m_minY = corners[0].y;
		m_maxX = m_minX;
		m_maxY = m_minY;

		for (UInt32 i = 1; i < 4; i++) {
			float x = corners[i].x;
			float y = corners[i].y;
			if (x < m_minX)			m_minX = x;
			else if (x > m_maxX)	m_maxX = x;

			if (y < m_minY)			m_minY = y;
			else if (y > m_maxY)	m_maxY = y;
		}
	}

	Rect GetBoundingRect() const {
		return Rect(m_area.center, Vector2((m_maxX-m_minX)/2, (m_maxY-m_minY)/2), 0);
	}

	const Vector2& GetCenter() const { return m_area.center; }

	bool IntersectsPoint(const Vector2& point) const {
		// rotate point into OBB coordinate system
		float x = point.x*m_cosNegTheta - point.y*m_sinNegTheta - m_centerLocal.x;
		float y = point.x*m_sinNegTheta + point.y*m_cosNegTheta - m_centerLocal.y;
		return (x > -m_area.extents.x && x < m_area.extents.x && y > -m_area.extents.y && y < m_area.extents.y);
	}

	bool LooselyIntersectsAABB(const Rect& aabb) const {
		// test against aabb axes
		if (m_maxX < aabb.center.x - aabb.extents.x ||
			m_minX > aabb.center.x + aabb.extents.x ||
			m_maxY < aabb.center.y - aabb.extents.y ||
			m_minY > aabb.center.y + aabb.extents.y) {
				return false;
		}

		// worth projecting aabb onto obb axes? for now, no.
		return true;
	}

	Rect		m_area;
	Vector2		m_centerLocal;						// center pt in local coordinates
	float		m_minX, m_minY, m_maxX, m_maxY;
	float		m_cosNegTheta, m_sinNegTheta;		// for rotating vectors into OBB local space
};

struct Circle
{
	Circle(Vector2& c, float r) : center(c), radius(r) { }
	Circle(float x, float y, float r) : center(x, y), radius(r) { }

	Vector2		center;
	float		radius;

	Rect GetBoundingRect() const { return Rect(center, Vector2(radius, radius), 0); }
	float GetRadiusSquared() const { return radius*radius; }

	bool IntersectsPoint(const Vector2& point) const {
		Vector2 diff(point.x - center.x, point.y - center.y);
		return Vector2_Dot(diff, diff) < GetRadiusSquared();
	}

	bool LooselyIntersectsAABB(const Rect& rect) const {
		// for quick, rough checks. Treat rect as a bounding radius
		Vector2 diff(rect.center.x - center.x, rect.center.y - center.y);
		float rectRadiusSquared = rect.extents.x*rect.extents.x + rect.extents.y*rect.extents.y;
		return Vector2_Dot(diff, diff) < rectRadiusSquared + GetRadiusSquared();
	}

	const Vector2& GetCenter() const { return center; }
};

struct Pos {
	SInt32	x;
	SInt32	y;

	Pos(SInt32 _x, SInt32 _y) : x(_x), y(_y) { }
	Pos() : x(0), y(0) { }
};

// for iterating over cells which intersect a rectangular area
class CellIterator
{
public:
	CellIterator(const Rect* worldArea = NULL) {
		TESObjectCELL* initialCell = GetCurrentCell();
		m_worldSpace = initialCell->worldSpace;
		m_bInterior = initialCell->IsInterior();
		if (!m_bInterior) {
			// for exteriors, store the cell containing the center point of the area instead
			initialCell = GetAbsoluteCell(worldArea->center.x, worldArea->center.y);
		}

		// Pos members are already default initialized to (0, 0)
		if (!m_bInterior) {
			if (worldArea) {
				m_topLeft = Pos((worldArea->center.x - worldArea->extents.x) / 4096,
					(worldArea->center.y - worldArea->extents.y) / 4096);
				m_bottomRight = Pos((worldArea->center.x + worldArea->extents.x) / 4096,
					(worldArea->center.y + worldArea->extents.y) / 4096);
				m_cur = m_topLeft;
			}
			else {
				m_topLeft = Pos(initialCell->coords->x, initialCell->coords->y);
				m_bottomRight = m_topLeft;
				m_cur = m_topLeft;
			}
		}
	}

	bool Done() const {
		return m_cur.x > m_bottomRight.x || m_cur.y > m_bottomRight.y;
	}

	TESObjectCELL* Get() const {
		if (!Done()) {
			if (m_bInterior) {
				return GetCurrentCell();
			}
			else if (m_worldSpace) {
				return m_worldSpace->LookupCell(m_cur.x, m_cur.y);
			}
		}

		return NULL;
	}

	void Next() {
		m_cur.x += 1;
		if (m_cur.x > m_bottomRight.x) {
			m_cur.x = m_topLeft.x;
			m_cur.y += 1;
		}
	}

private:
	Pos				m_topLeft;
	Pos				m_bottomRight;
	Pos				m_cur;
	TESWorldSpace	* m_worldSpace;
	bool			m_bInterior;
};

typedef std::vector<NodeID> NodeList;

// get a rect defining the bounds of a cell in its worldspace
Rect GetExteriorCellRect(TESObjectCELL* cell)
{
	static Vector2 s_extents(Vector2(fCellExtent, fCellExtent));
	return Rect(Vector2(cell->coords->x * fCellDimension + fCellExtent, cell->coords->y * fCellDimension + fCellExtent), s_extents, 0);
}

// keeps track of which partitions in a cell have been tested for intersection with an area
// for exterior cells could use a couple of bitfields instead since number of partitions is known - but this is actually
// quite fast enough
class CellPartitionSet
{
public:
	// rect unused, retained to allow compile-time polymorphic instantiation in GetNodesInArea()
	CellPartitionSet(const Rect& cellRect) { }

	void MarkTested(float x, float y, bool bIntersects) {
		UInt32 key = TESPathGrid::GetPartitionKey(x, y);
		m_tested.insert(key);
		if (bIntersects) {
			m_intersect.insert(key);
		}
	}

	bool IsTested(float x, float y) { return m_tested.find(TESPathGrid::GetPartitionKey(x, y)) != m_tested.end(); }
	bool Intersects(float x, float y) { return m_intersect.find(TESPathGrid::GetPartitionKey(x, y)) != m_intersect.end(); }

private:
	std::set<UInt32>	m_tested;
	std::set<UInt32>	m_intersect;
};

// _Shape should be some class that describes the area we're interested in
// Should implement:
//	Rect GetBoundingRect() returning a rectangle fully enclosing the area
//	bool LooselyIntersectsAABB(Rect) returns true if area possibly intersects square AABB, never returning a false negative.
//	bool IntersectsPoints(Vector2) returns true iff point is within area
//  Vector2 GetCenter() returns center point of area
// This populates a NodeList with nodes contained within the specified area, optionally excluding disabled nodes.
template <class _Shape>
class AreaNodeFinder
{
public:
	AreaNodeFinder(_Shape& area, bool bIncludeDisabledNodes)
		: m_area(area), m_playerCellX(0), m_playerCellY(0), m_bIncludeDisabled(bIncludeDisabledNodes)  {
		m_centerCell = (*g_thePlayer)->parentCell;
		if (m_centerCell && !m_centerCell->IsInterior()) {
			m_playerCellX = m_centerCell->coords->x;
			m_playerCellY = m_centerCell->coords->y;

			const Vector2& center = area.GetCenter();
			m_centerCell = GetAbsoluteCell(center.x, center.y);
		}
	}

	bool FindNodes() {
		static const Rect s_interiorCellRect(Vector2(0, 0), Vector2(fCellExtent, fCellExtent), 0);
		static const Vector2 s_partitionExtents(fPartitionExtent, fPartitionExtent);
		if (!m_centerCell) {
			return false;
		}

		// iterate through all cells that could potentially intersect the area
		Rect boundingRect = m_area.GetBoundingRect();
		for (CellIterator cellIter(&boundingRect); !cellIter.Done(); cellIter.Next()) {
			TESObjectCELL* curCell = cellIter.Get();
			if (curCell && curCell->pathGrid && curCell->pathGrid->nodes) {
				SInt32 curCellDX = 0;
				SInt32 curCellDY = 0;
				if (!curCell->IsInterior()) {
					curCellDX = curCell->coords->x - m_playerCellX;
					curCellDY = curCell->coords->y - m_playerCellY;

					// NodeID can't support distances greater than 7 from the current cell
					// But cells that far away are not likely to be loaded in memory anyway
					if (abs(curCellDX) > 7 || abs(curCellDY) > 7) {
						continue;
					}
				}

				// only check cells which intersect the area we're interested in
				// ###TODO: I'd like to separate out the behavior that varies based on interior/exterior cell
				Rect cellRect = curCell->IsInterior() ? s_interiorCellRect : GetExteriorCellRect(curCell);
				if (curCell->IsInterior() || m_area.LooselyIntersectsAABB(cellRect)) {
					// test each point, keeping track of the partitions we've already tested for intersections
					CellPartitionSet partitions(cellRect);
					for (UInt32 i = 0; i < curCell->pathGrid->nodeCount; i++) {
						TESPathGridPoint* pt = curCell->pathGrid->nodes->data[i];
						if (pt && (m_bIncludeDisabled || !pt->IsDisabled())) {
							// does point's partition intersect our area?
							if (!partitions.IsTested(pt->x, pt->y)) {
								Rect partitionRect(Vector2(floor(pt->x/512)*512 + fPartitionExtent, floor(pt->y/512)*512 + fPartitionExtent),
									s_partitionExtents, 0);
								partitions.MarkTested(pt->x, pt->y, m_area.LooselyIntersectsAABB(partitionRect));
							}

							// test point for intersection
							if (partitions.Intersects(pt->x, pt->y)) {
								if (m_area.IntersectsPoint(Vector2(pt->x, pt->y))) {
									m_nodes.push_back(NodeID(i, curCellDX, curCellDY));
								}
							}
						}
					}
				}
			}
		}

		return true;
	}

	bool PopulateNodeArray(ArrayID arr) {
		for (UInt32 i = 0; i < m_nodes.size(); i++) {
			DEBUG_PRINT("(%.0f, %.0f)", m_nodes[i].GetPoint()->x, m_nodes[i].GetPoint()->y);
			g_ArrayMap.SetElementNumber(arr, double(i), m_nodes[i].GetIntegerID());
		}

		return true;
	}

private:
	_Shape			m_area;
	SInt32			m_playerCellX;
	SInt32			m_playerCellY;
	TESObjectCELL	* m_centerCell;
	NodeList		m_nodes;
	bool			m_bIncludeDisabled;
};

static bool Cmd_GetPathNodesInRadius_Execute(COMMAND_ARGS)
{
	float x, y, r;
	UInt32 bIncludeDisabled = false;
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &x, &y, &r, &bIncludeDisabled) && r > 0) {
		Circle area(Vector2(x, y), r);
		bool bSuccess = false;
		AreaNodeFinder<Circle> finder(area, bIncludeDisabled ? true : false);
		if (finder.FindNodes()) {
			DEBUG_PRINT("Retrieved nodes in radius %.0f from (%.0f, %.0f)", r, x, y);
			finder.PopulateNodeArray(arr);
		}
	}

	return true;
}

static bool Cmd_GetPathNodesInRect_Execute(COMMAND_ARGS)
{
	float cx, cy, ex, ey;
	float theta = 0.0;
	UInt32 bIncludeDisabled = false;
	ArrayID arr = g_ArrayMap.Create(kDataType_Numeric, true, scriptObj->GetModIndex());
	*result = arr;

	if (ExtractArgs(PASS_EXTRACT_ARGS, &cx, &cy, &ex, &ey, &theta, &bIncludeDisabled) && ex > 0 && ey > 0) {
		Rect areaRect(cx, cy, ex, ey, theta);
		if (FloatEqual(theta, 0.0)) {
			AreaNodeFinder<AABBPathGridArea> finder(AABBPathGridArea(areaRect), bIncludeDisabled ? true : false);
			if (finder.FindNodes()) {
				DEBUG_PRINT("Got path nodes in rect");
				finder.PopulateNodeArray(arr);
			}
		}
		else {
			AreaNodeFinder<OBBPathGridArea> finder(OBBPathGridArea(areaRect), bIncludeDisabled ? true : false);
			if (finder.FindNodes()) {
				DEBUG_PRINT("Got path nodes in rect");
				finder.PopulateNodeArray(arr);
			}
		}
	}

	return true;
}

#endif

static ParamInfo kParams_GetAllPathNodes[2] =
{
	{	"cell depth",				kParamType_Integer,	1	},
	{	"include disabled nodes",	kParamType_Integer, 1	},
};

DEFINE_COMMAND(GetAllPathNodes, returns an array of all path nodes within a cell, 0, 2, kParams_GetAllPathNodes);
DEFINE_COMMAND(IsPathNodeDisabled, returns true if the node is disabled, 0, 1, kParams_OneInt);
DEFINE_COMMAND(SetPathNodeDisabled, sets the disabled flag for the node, 0, 2, kParams_TwoInts);

static ParamInfo kParams_GetPathNodePos[2] =
{
	{	"nodeID",	kParamType_Integer,	0	},
	{	"axis",		kParamType_Axis,	0	},
};

DEFINE_COMMAND(GetPathNodePos, gets the position of the node, 0, 2, kParams_GetPathNodePos);
DEFINE_COMMAND(SetPathNodePreferred, sets the preferred flag for the node, 0, 2, kParams_TwoInts);
DEFINE_COMMAND(PathEdgeExists, returns 1 if an edge exists between the two nodes, 0, 2, kParams_TwoInts);

static ParamInfo kParams_SetPathEdgeExists[3] =
{
	{	"nodeID",	kParamType_Integer, 0 },
	{	"nodeID",	kParamType_Integer, 0 },
	{	"edgeExists",	kParamType_Integer, 0 },
};

DEFINE_COMMAND(SetPathEdgeExists, enables or disables edge between two nodes, 0, 3, kParams_SetPathEdgeExists);

DEFINE_COMMAND(GetAllPathEdges, testing, 0, 0, NULL);

DEFINE_COMMAND(GetPathNodeLinkedRef, returns the linked ref for the node, 0, 1, kParams_OneInt);

static ParamInfo kParams_GetPathNodesInRadius[4] =
{
	{ "x",				kParamType_Float,	0	},
	{ "y",				kParamType_Float,	0	},
	{ "radius",			kParamType_Float,	0	},
	{ "includeDisabled",kParamType_Integer,	1	},
};

DEFINE_COMMAND(GetPathNodesInRadius, returns a list of nodes in the radius, 0, 4, kParams_GetPathNodesInRadius);

static ParamInfo kParams_GetPathNodesInRect[6] =
{
	{ "x",				kParamType_Float,	0	},
	{ "y",				kParamType_Float,	0	},
	{ "extentX",		kParamType_Float,	0	},
	{ "extentY",		kParamType_Float,	0	},
	{ "angle",			kParamType_Float,	1	},
	{ "includeDisabled",kParamType_Integer,	1	},
};

DEFINE_COMMAND(GetPathNodesInRect, returns a list of nodes within the rectangular area, 0, 6, kParams_GetPathNodesInRect);