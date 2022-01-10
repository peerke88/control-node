#ifndef RIGSYSTEM_CONTROL_NODE


#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef size_t SizeType; }

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
//#include <maya/MGlobal.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/MFnPlugin.h>
#include <maya/MDistance.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MArrayDataBuilder.h>
#include <maya/MFnTypedAttribute.h>

#include <maya/MEvaluationNode.h>
#include <maya/MHardwareRenderer.h>
#include <maya/MDrawRegistry.h>
#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MDrawContext.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MPointArray.h>
#include <maya/MGlobal.h>
#include <maya/MEventMessage.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSelectionList.h>
#include <maya/MQuaternion.h>
#include "utils.hpp"
#include "rapidjson/document.h"

#include <string>
#include <iostream>
#include <resources.h>

using namespace std;


using namespace rapidjson;
 
namespace {
	std::map<std::string, MPointArray> jsonDataParse() {
		Document js;
		js.Parse(Resources::controls.c_str());

		std::map<std::string, MPointArray> shapes;
		const char* key;
		for (Value::ConstMemberIterator itr = js.MemberBegin();itr != js.MemberEnd(); ++itr){
			key = itr->name.GetString();

			auto amount = js[key].Size();
			for (size_t i = 0; i < amount; ++i) {
				size_t am = js[key][i].Size()-1;
				for (size_t j = 0; j < am; ++j) {
					MPoint pt0(js[key][i][0][0].GetDouble(), js[key][i][0][1].GetDouble(), js[key][i][0][2].GetDouble());
					MPoint pt1(js[key][i][j][0].GetDouble(), js[key][i][j][1].GetDouble(), js[key][i][j][2].GetDouble());
					MPoint pt2(js[key][i][j+1][0].GetDouble(), js[key][i][j+1][1].GetDouble(), js[key][i][j+1][2].GetDouble());
					shapes[key].append(pt0);
					shapes[key].append(pt1);
					shapes[key].append(pt2);
				};
			};
		};
		return shapes;

	}
	std::map<std::string, MPointArray> control_shapes = jsonDataParse();
};


class RigSystemControlNode : public MPxLocatorNode{
public:
	MStatus compute(const MPlug& plug, MDataBlock& data) override { 
		return MS::kSuccess;
	};
	
	bool isBounded() const override { 
		return true; 
	};
	
	SchedulingType schedulingType() const override { 
		return SchedulingType::kParallel; 
	}
	
	MStatus preEvaluation(const MDGContext& context, const MEvaluationNode& evaluationNode) override;

	static  void *  creator() { 
		return new RigSystemControlNode(); 
	};
	static  MStatus initialize();
	
	static const MString kSelectionMaskName;
  	virtual MSelectionMask getShapeSelectionMask() const override { 
  		return MSelectionMask(kSelectionMaskName); 
  	};
	
	static	MTypeId	id;
	static MString	drawDbClassification;
	static MString	drawRegistrantId;

	static MObject offsetMatrix;
	static MObject controlList;
	static MObject size;      
	static MObject lineThickness;
	static MObject fill;
	static MObject front;
	static MObject worldS;
};


class RigSystemControlData : public MUserData{
public:
	float lineThick;
	MColor fColor;
	bool drawInFront;
	bool fillObject;
	unsigned int fDepthPriority;
	MPointArray fLineList;
	MPointArray fTriangleList;
};

class RigSystemControlDrawOverride : public MHWRender::MPxDrawOverride{
public:
	static MHWRender::MPxDrawOverride* Creator(const MObject& obj) { 
		return new RigSystemControlDrawOverride(obj); 
	}

	~RigSystemControlDrawOverride() override;

	MHWRender::DrawAPI supportedDrawAPIs() const override{ 
		return (MHWRender::kOpenGL | MHWRender::kDirectX11 | MHWRender::kOpenGLCoreProfile); 
	};

	bool isBounded( const MDagPath& objPath, const MDagPath& cameraPath) const override { 
		return true; 
	};
	MBoundingBox boundingBox( const MDagPath& objPath, const MDagPath& cameraPath) const override;

	MUserData* prepareForDraw( const MDagPath& objPath, const MDagPath& cameraPath, const MHWRender::MFrameContext& frameContext, MUserData* oldData) override;
	bool hasUIDrawables() const override { 
		return true; 
	}
	void addUIDrawables( const MDagPath& objPath, MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext& frameContext, const MUserData* data) override;

	bool traceCallSequence() const override{ 
		return false; 
	}
	void handleTraceMessage(const MString &message) const override {
		MGlobal::displayInfo("RigSystemControlDrawOverride: " + message);
	}


private:
	RigSystemControlDrawOverride(const MObject& obj);

	static void OnModelEditorChanged(void *clientData);

	RigSystemControlNode*  fRigSystemControlNode;
	MCallbackId fModelEditorChangedCbId;
	MObject fRigSystemControl;
	
};

#define RIGSYSTEM_CONTROL_NODE
#endif