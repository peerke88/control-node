#include "rigSystemControlNode.h"

MTypeId RigSystemControlNode::id(0x00131741);

MObject RigSystemControlNode::offsetMatrix;
MObject RigSystemControlNode::controlList;
MObject RigSystemControlNode::size;
MObject RigSystemControlNode::lineThickness;
MObject RigSystemControlNode::fill;
MObject RigSystemControlNode::front;
MObject RigSystemControlNode::worldS;

const MString RigSystemControlNode::kSelectionMaskName = "RigSystemControlNodeSelection";
MString	RigSystemControlNode::drawDbClassification("drawdb/geometry/ControlNode");
MString	RigSystemControlNode::drawRegistrantId("ControlNodePlugin");


std::unordered_map<size_t, RigSystemControlNode*> RigSystemControlNode::instances;

size_t NodeHash(const MObject& p) {
    MUuid uuid = MFnDependencyNode(p).uuid();
    std::string guid = "0000000000000000";
    uuid.get((unsigned char*)&guid[0]);
    MString ns = MFnDependencyNode(p).parentNamespace();
    std::string namesp = ns.asChar();
    return std::hash<std::string>()(namesp + guid);
}


// --- MPxLocatorNode
MStatus RigSystemControlNode::initialize()
{
	MFnNumericAttribute fn;
	MFnEnumAttribute en;
	MFnMatrixAttribute mn;
	MStatus stat;

	offsetMatrix = mn.create("offsetMatrix", "om");
	addAttribute(offsetMatrix);

	size = fn.create("size", "sz", MFnNumericData::kDouble, 1.0, &stat); CHECK_MSTATUS_AND_RETURN_IT(stat);
	addAttribute(size);

	front = fn.create("drawInFront", "dif", MFnNumericData::kBoolean, 0, &stat); CHECK_MSTATUS_AND_RETURN_IT(stat);
	addAttribute(front);

	worldS = fn.create("worldS", "worldS", MFnNumericData::kDouble, 0.0, &stat); CHECK_MSTATUS_AND_RETURN_IT(stat);
	fn.setAffectsAppearance(true);
	fn.setAffectsWorldSpace(true);
	fn.setWorldSpace(true);
	addAttribute(worldS);

	lineThickness = fn.create("lineThickness", "ltn", MFnNumericData::kDouble, 1.0, &stat); CHECK_MSTATUS_AND_RETURN_IT(stat);
	fn.setSoftMin(0.0);
	fn.setSoftMax(10.0);
	addAttribute(lineThickness);

	fill = fn.create("fill", "fl", MFnNumericData::kBoolean, 0, &stat); CHECK_MSTATUS_AND_RETURN_IT(stat);
	addAttribute(fill);

	controlList = en.create("controlList", "cl", 0);
	int enumindex = 0;
	std::vector<MString> keys;
	for ( const auto &myPair : control_shapes ) {
		en.addField(myPair.first.c_str(), enumindex);
		enumindex++;
    }
	addAttribute(controlList);

	attributeAffects(front, worldS);
	attributeAffects(size, worldS);
	attributeAffects(lineThickness, worldS);
	attributeAffects(offsetMatrix, worldS);
	attributeAffects(fill, worldS);
	attributeAffects(controlList, worldS);
	return MS::kSuccess;
}

MStatus	RigSystemControlNode::compute(const MPlug& p, MDataBlock& b){
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorA_L3, "MPxLocator::compute()");
	
	if (handle == 0) {
		handle = NodeHash(thisMObject());
		instances.emplace(handle, this);
	}
	double d = b.inputValue(size).asDouble();
	ud->size = d;
	bool xray = b.inputValue(front).asBool();
	ud->front = xray;
	ud->fillObject = b.inputValue(fill).asBool(); 
	ud->lineThick = b.inputValue(lineThickness).asDouble();
	MMatrix ofstMatrix = b.inputValue(offsetMatrix).asMatrix();

	int enumInt = b.inputValue(controlList).asInt();
	std::vector<MPointArray> shapePoints;
	for ( const auto &myPair : control_shapes ) {
    	shapePoints.push_back(myPair.second);
	}

	ud->fLineList.clear();
	for (unsigned int c = 0; c < shapePoints[enumInt].length(); c++) {
		if (c%3 != 0){
			ud->fLineList.append((shapePoints[enumInt][c] * d)* ofstMatrix);
		}
	}
	if (ud->fillObject == true) {
		for (unsigned int c = 0; c < shapePoints[enumInt].length(); c++) {
			ud->fTriangleList.append((shapePoints[enumInt][c] * d) * ofstMatrix);
		}
	}
	b.outputValue(worldS).setDouble(d);
	b.setClean(worldS);
	return MS::kSuccess;
}

 
// --- MPxDrawOverride

MUserData* RigSystemControlDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData){
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorA_L1, "MPxDrawOverride::prepareForDraw()");
	size_t handle = NodeHash(objPath.node());
	auto it = RigSystemControlNode::instances.find(handle);
	if (it == RigSystemControlNode::instances.end()) return 0;
	return (MUserData*)it->second->ud;
}

void RigSystemControlDrawOverride::addUIDrawables( const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data){
	
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorD_L2, "MPxDrawOverride::addUIDrawables()");

	const RigSystemControlNode::UD* u = (const RigSystemControlNode::UD*)data;
	if (!u) return;
	drawManager.beginDrawable();
	if (u->front) {
		drawManager.beginDrawInXray();
	}
	drawManager.setLineWidth(u->lineThick);
	drawManager.setColor(MHWRender::MGeometryUtilities::wireframeColor(objPath));
	if (u->fillObject){
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, u->fTriangleList);
	}

	drawManager.mesh(MHWRender::MUIDrawManager::kLines, u->fLineList);

	if (u->front) {
		drawManager.endDrawInXray();
	}
	drawManager.endDrawable();
}

