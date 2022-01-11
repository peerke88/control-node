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

MStatus	RigSystemControlNode::compute(const MPlug& p, MDataBlock& b){
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorA_L3, "MPxLocator::compute()");
	
	if (p != worldS) return MS::kUnknownParameter;
	if (handle == 0) {
		handle = NodeHash(thisMObject());
		instances.emplace(handle, this);
	}
	MMatrix ofstMatrix = b.inputValue(offsetMatrix).asMatrix();
	double sizeVal = b.inputValue(size).asDouble();
	ud->lineThick = b.inputValue(lineThickness).asFloat();

	// ud->fColor = MHWRender::MGeometryUtilities::wireframeColor(objPath); ;
	ud->drawInFront = b.inputValue(front).asBool();
	ud->fillObject = b.inputValue(fill).asBool(); ;

	int enumInt = b.inputValue(controlList).asInt();
	std::vector<MPointArray> shapePoints;
	for ( const auto &myPair : control_shapes ) {
    	shapePoints.push_back(myPair.second);
	}

	ud->fLineList.clear();
	for (unsigned int c = 0; c < shapePoints[enumInt].length(); c++) {
		if (c%3 != 0){
			ud->fLineList.append((shapePoints[enumInt][c] * sizeVal) * ofstMatrix);
		}
	}

	ud->fTriangleList.clear();
	if (ud->fillObject == true) {
		for (unsigned int c = 0; c < shapePoints[enumInt].length(); c++) {
			ud->fTriangleList.append((shapePoints[enumInt][c] * sizeVal) * ofstMatrix);
		}
	}
	b.outputValue(worldS).setFloat((float)sizeVal);
	b.setClean(worldS);
	return MS::kSuccess;
}
 
// MStatus RigSystemControlNode::preEvaluation( const MDGContext& context, const MEvaluationNode& evaluationNode){
// 	if (context.isNormal())	{
// 		MStatus status;
// 		if (evaluationNode.dirtyPlugExists(size, &status) && status){
// 			MHWRender::MRenderer::setGeometryDrawDirty(thisMObject());
// 		}
// 	}

// 	return MStatus::kSuccess;
// }

 
// --- MPxDrawOverride
RigSystemControlDrawOverride::RigSystemControlDrawOverride(const MObject& obj):MHWRender::MPxDrawOverride(obj, NULL, false), fRigSystemControl(obj) {
	fModelEditorChangedCbId = MEventMessage::addEventCallback( "modelEditorChanged", OnModelEditorChanged, this);

	MStatus status;
	MFnDependencyNode node(obj, &status);
	fRigSystemControlNode = status ? dynamic_cast<RigSystemControlNode*>(node.userNode()) : NULL;
}


RigSystemControlDrawOverride::~RigSystemControlDrawOverride(){
	fRigSystemControlNode = NULL;

	if (fModelEditorChangedCbId != 0){
		MMessage::removeCallback(fModelEditorChangedCbId);
		fModelEditorChangedCbId = 0;
	}
}


void RigSystemControlDrawOverride::OnModelEditorChanged(void *clientData){
	RigSystemControlDrawOverride *ovr = static_cast<RigSystemControlDrawOverride*>(clientData);
	if (ovr && ovr->fRigSystemControlNode){
		MHWRender::MRenderer::setGeometryDrawDirty(ovr->fRigSystemControlNode->thisMObject());
	}
}


MBoundingBox RigSystemControlDrawOverride::boundingBox( const MDagPath& objPath, const MDagPath& cameraPath) const{
	double sizeVal = getPlugValue<float>(objPath, RigSystemControlNode::size, 1.0);
	int enumInt = getPlugValue<int>(objPath, RigSystemControlNode::controlList, 0);
	MPlug plug(fRigSystemControl, RigSystemControlNode::offsetMatrix);
	MMatrix offsetMatrix = plug.asMDataHandle().asMatrix();
	
	std::vector<MPointArray> shapePoints;
	for ( const auto &myPair : control_shapes ) {
    	shapePoints.push_back(myPair.second);
	}
	MBoundingBox bbox =  getBoundingBox(shapePoints[enumInt], sizeVal, offsetMatrix);
	return bbox;
}

MUserData* RigSystemControlDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData){
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorA_L1, "MPxDrawOverride::prepareForDraw()");
	size_t handle = NodeHash(objPath.node());
	auto it = RigSystemControlNode::instances.find(handle);
	if (it == RigSystemControlNode::instances.end()) return 0;
	return (MUserData*)it->second->ud;
}

void RigSystemControlDrawOverride::addUIDrawables( const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data){
	
	MProfilingScope profilerScope(gProfilerCategory, MProfiler::kColorD_L2, "MPxDrawOverride::addUIDrawables()");

	const RigSystemControlNode::RigSystemControlData* pLocatorData = (const RigSystemControlNode::RigSystemControlData*)data;
	if (!pLocatorData) return;
	
	drawManager.beginDrawable();
	if (pLocatorData->drawInFront) {
		drawManager.beginDrawInXray();
	}
	drawManager.setLineWidth(pLocatorData->lineThick);
	// drawManager.setColor(MHWRender::MGeometryUtilities::wireframeColor(objPath));
	drawManager.setDepthPriority(5);

	if (pLocatorData->fillObject ){
		drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, pLocatorData->fTriangleList);
	}

	drawManager.mesh(MHWRender::MUIDrawManager::kLines, pLocatorData->fLineList);
	if (pLocatorData->drawInFront) {
		drawManager.endDrawInXray();
	}
	drawManager.endDrawable();
}

MStatus RigSystemControlNode::initialize()
{
	MFnUnitAttribute uAttr;
	MFnEnumAttribute eAttr;
	MFnMatrixAttribute mAttr;
	MFnNumericAttribute nAttr;

	offsetMatrix = mAttr.create("offsetMatrix", "om");
	mAttr.setKeyable(false);
	mAttr.setReadable(true);
	mAttr.setStorable(true);
	addAttribute(offsetMatrix);

	controlList = eAttr.create("controlList", "cl", 0);
	int enumindex = 0;
	std::vector<MString> keys;
	for ( const auto &myPair : control_shapes ) {
		eAttr.addField(myPair.first.c_str(), enumindex);
		enumindex++;
    }
	eAttr.setReadable(true);
	addAttribute(controlList);

	size = nAttr.create("size", "sz", MFnNumericData::kDouble, 1.0);
	uAttr.setSoftMin(0.0);
	uAttr.setSoftMax(10.0);
	addAttribute(size);
	
	lineThickness = nAttr.create("lineThickness", "ltn", MFnNumericData::kFloat, 1.0);
	nAttr.setSoftMin(0.0);
	nAttr.setSoftMax(10.0);
	addAttribute(lineThickness);

	fill = nAttr.create("fill", "fl", MFnNumericData::kBoolean, 0);
	nAttr.setDefault(false);
	nAttr.setWritable(true);
	nAttr.setReadable(true);
	addAttribute(fill);

	front = nAttr.create("drawInFront", "dif", MFnNumericData::kBoolean, 0);
	nAttr.setDefault(false);
	nAttr.setWritable(true);
	nAttr.setReadable(true);
	addAttribute(front);

	worldS = uAttr.create("worldS", "ws", MFnUnitAttribute::kDistance, 1.0);
	uAttr.setWritable(true);
	uAttr.setCached(false);
	uAttr.setHidden(true);
	addAttribute(worldS);

	attributeAffects(size, worldS);
	attributeAffects(controlList, worldS);
	attributeAffects(offsetMatrix, worldS);
	attributeAffects(fill, worldS);
	attributeAffects(front, worldS);
	attributeAffects(lineThickness, worldS);

	return MS::kSuccess;
}
