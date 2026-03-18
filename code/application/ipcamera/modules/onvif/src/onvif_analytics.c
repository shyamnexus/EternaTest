#include "onvif_main.h"
#include "wsaapi.h"
#include "soapH.h"
#include "soapStub.h"
#include "wsddapi.h"
#include <sqlite3.h>
// #include "model.h"  // TODO: Replace with Novatek model API

// TODO: Replace with Novatek access control
#define ACCESS_ACTUATE 1
#define ACCESS_READ_MEDIA 2
static int ValidateAccessibility(struct soap* soap, int access_type, void* p1, void* p2) {
    // Stub - always allow access for now
    return SOAP_OK;
}

// TODO: Replace with Novatek model update
static void model_update(void) {
    // Stub - do nothing for now
}

typedef char*				CHARPTR;
typedef char                CHAR;
#define INFO_BUFFER_LENGTH 1024
#define INFO_LENGTH 100
#define MOTION_REGION "MotionRegion"
#define MOTION_REGION_CONFIG "axt:MotionRegionConfig"
#define MOTION_VIDEO_SOURCE "VideoSource"
#define REFERENCE_TOK "tt:ReferenceToken"
#define MOTION_RULL_NAME "RuleName"
#define MOTION_STRING_TYPE "xsd:string"
#define STATE_VALUE "State"
#define RULEENGINE_MOTIONREGIONDETECT_MOTION    "tns1:RuleEngine/MotionRegionDetector/Motion"
#define MOTION_REGION_DETECTOR                    "tt:MotionRegionDetector"
#define MOTION_BOOL_TYPE                          "xsd:boolean"
#define SPRINTF(s, f, a...) (sprintf((char *)s, (const char *)f, ##a))
#define SNPRINTF(s,n, f, a...) (snprintf((char *)s,n, (const char *)f, ##a))
#define MEMSET(s, c, n) (memset((void *)s, (int)c, (size_t)n))
#define MOTION_PLOYGON "tt:Polygon"
#define POLYGON_POINT "tt:Point"
#define POLYGON_ITEM "hon:ItemSize"
/*****************************************************************************************************
 *                                onvif_analytics function Definition
 *****************************************************************************************************/




// /** Web service operation '__tan__CreateRules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateRules(struct soap * soap, struct _tan__CreateRules *tan__CreateRules, struct _tan__CreateRulesResponse *tan__CreateRulesResponse)
{
	printf("\n---------------------__tan__CreateRules-----------------\n");
	int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_ACTUATE, NULL, NULL);
	if (l_i32_ret != SOAP_OK)
		return l_i32_ret;


	return SOAP_OK;
}

// SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules(struct soap * soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse)
// {
// 	printf("\n---------------------__tan__GetRules-----------------\n");
// 	// CHAR *name_motionConfig[] = {"Armed","Sensitivity"};
// 	// CHAR *name_motionVector[] = {"x","y"};

// 	tan__GetRulesResponse->__sizeRule =1;

// 	tan__GetRulesResponse->Rule = (struct  tt__Config *) soap_malloc(soap,  tan__GetRulesResponse->__sizeRule * sizeof(struct tt__Config));
// 	MEMSET(tan__GetRulesResponse->Rule, 0x00, tan__GetRulesResponse->__sizeRule  * sizeof(struct tt__Config));

// 	for(int i = 0; i < tan__GetRulesResponse->__sizeRule; i++)
// 	{
// 		tan__GetRulesResponse->Rule[i].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
// 		MEMSET(tan__GetRulesResponse->Rule[i].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
// 		SNPRINTF(tan__GetRulesResponse->Rule[i].Name,TOKEN_LENGTH,"%s","ObjectDetectionRule");

// 		tan__GetRulesResponse->Rule[i].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
// 		MEMSET(tan__GetRulesResponse->Rule[i].Type, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
// 		SNPRINTF(tan__GetRulesResponse->Rule[i].Type,TOKEN_LENGTH,"%s","tt:ObjectDetector");

// 		tan__GetRulesResponse->Rule[i].Parameters = (struct  tt__ItemList *) soap_malloc(soap, sizeof(struct tt__ItemList));
// 		MEMSET(tan__GetRulesResponse->Rule[i].Parameters, 0x00, sizeof(struct tt__ItemList));

// 		tan__GetRulesResponse->Rule[i].Parameters->__sizeSimpleItem = 1;
// 		tan__GetRulesResponse->Rule[i].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap, tan__GetRulesResponse->Rule[i].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
// 		MEMSET(tan__GetRulesResponse->Rule[i].Parameters->SimpleItem, 0x00, tan__GetRulesResponse->Rule[i].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

// 		tan__GetRulesResponse->Rule[i].Parameters->SimpleItem->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
// 		MEMSET(tan__GetRulesResponse->Rule[i].Parameters->SimpleItem->Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
// 		SNPRINTF(tan__GetRulesResponse->Rule[i].Parameters->SimpleItem->Name, TOKEN_LENGTH,"%s","SimpleItem1");

// 		tan__GetRulesResponse->Rule[i].Parameters->SimpleItem->Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
// 		MEMSET(tan__GetRulesResponse->Rule[i].Parameters->SimpleItem->Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
// 		SNPRINTF(tan__GetRulesResponse->Rule[i].Parameters->SimpleItem->Value, TOKEN_LENGTH,"%s","SimpleItem1Value");
		

// 		tan__GetRulesResponse->Rule[i].Parameters->__sizeElementItem = 0;
// 		// tan__GetRulesResponse->Rule[i].Parameters->ElementItem = (struct  _tt__ItemList_ElementItem *) soap_malloc(soap, tan__GetRulesResponse->Rule[i].Parameters->__sizeElementItem * sizeof(struct _tt__ItemList_ElementItem));
// 		// MEMSET(tan__GetRulesResponse->Rule[i].Parameters->ElementItem, 0x00, tan__GetRulesResponse->Rule[i].Parameters->__sizeElementItem * sizeof(struct _tt__ItemList_ElementItem));

// 		// tan__GetRulesResponse->Rule[i].Parameters->ElementItem->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
// 		// MEMSET(tan__GetRulesResponse->Rule[i].Parameters->ElementItem->Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
// 		// SNPRINTF(tan__GetRulesResponse->Rule[i].Parameters->ElementItem->Name, TOKEN_LENGTH,"%s",MOTION_REGION);

// 		// if(l_x_analyticsRules.m_x_ruleEngine[i].m_b_armed == 1)
// 		// {
// 		// 	SNPRINTF(motionConfigArmed,INFO_BUFER_LENGTH,"%s","true");
// 		// }
// 		// else
// 		// {
// 		// 	SNPRINTF(motionConfigArmed,INFO_LENGTH,"%s","false");
// 		// }
// 		tan__GetRulesResponse->Rule[i].Parameters->Extension = NULL;
// 	}
// 	return SOAP_OK;
// }




/** Web service operation '__tan__GetServiceCapabilities' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetServiceCapabilities(struct soap *soap, struct _tan__GetServiceCapabilities *tan__GetServiceCapabilities, struct _tan__GetServiceCapabilitiesResponse *tan__GetServiceCapabilitiesResponse)
{
    printf("\n\nCalled GetServiceCapabilities \n");
	int32_t l_i32_ret = ValidateAccessibility(soap, ACCESS_READ_MEDIA, NULL, NULL);
    if (l_i32_ret != SOAP_OK)
        return l_i32_ret;


    tan__GetServiceCapabilitiesResponse->Capabilities = (struct  tan__Capabilities *) soap_malloc(soap, sizeof(struct tan__Capabilities));
	MEMSET(tan__GetServiceCapabilitiesResponse->Capabilities, 0x00, sizeof(struct tan__Capabilities));

    tan__GetServiceCapabilitiesResponse->Capabilities->__size = 1;
    tan__GetServiceCapabilitiesResponse->Capabilities->RuleSupport = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tan__GetServiceCapabilitiesResponse->Capabilities->RuleSupport) = xsd__boolean__true_; //l_x_caps.m_b_analyticRuleSupport ? xsd__boolean__true_ : xsd__boolean__false_;

    tan__GetServiceCapabilitiesResponse->Capabilities->AnalyticsModuleSupport = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tan__GetServiceCapabilitiesResponse->Capabilities->AnalyticsModuleSupport) = xsd__boolean__true_; //l_x_caps.m_b_analyticsModuleSupport ? xsd__boolean__true_ : xsd__boolean__false_;

    tan__GetServiceCapabilitiesResponse->Capabilities->CellBasedSceneDescriptionSupported = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tan__GetServiceCapabilitiesResponse->Capabilities->CellBasedSceneDescriptionSupported) = xsd__boolean__false_ ; //l_x_caps.i_b_CellBasedSceneDescriptorSupport ? xsd__boolean__true_ : xsd__boolean__false_;

    tan__GetServiceCapabilitiesResponse->Capabilities->RuleOptionsSupported = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tan__GetServiceCapabilitiesResponse->Capabilities->RuleOptionsSupported) = xsd__boolean__true_ ; //l_x_caps.i_b_RuleOptionsSupported ? xsd__boolean__true_ : xsd__boolean__false_;

    tan__GetServiceCapabilitiesResponse->Capabilities->AnalyticsModuleOptionsSupported = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tan__GetServiceCapabilitiesResponse->Capabilities->AnalyticsModuleOptionsSupported) = xsd__boolean__true_;//l_x_caps.i_b_AnalyticsModuleOptionsSupported ? xsd__boolean__true_ : xsd__boolean__false_;

    tan__GetServiceCapabilitiesResponse->Capabilities->SupportedMetadata = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
    *(tan__GetServiceCapabilitiesResponse->Capabilities->SupportedMetadata) = xsd__boolean__false_;

    tan__GetServiceCapabilitiesResponse->Capabilities->ImageSendingType = (char *) soap_malloc(soap, INFO_LENGTH*sizeof(char));
	MEMSET(tan__GetServiceCapabilitiesResponse->Capabilities->ImageSendingType , 0x00, sizeof(char)*INFO_LENGTH);
	SNPRINTF(tan__GetServiceCapabilitiesResponse->Capabilities->ImageSendingType,INFO_BUFFER_LENGTH,"%s","false");


	tan__GetServiceCapabilitiesResponse->Capabilities->__any = NULL;
	// tan__GetServiceCapabilitiesResponse->Capabilities->__anyAttribute = NULL;
    dom_att(&tan__GetServiceCapabilitiesResponse->Capabilities->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

    return SOAP_OK;
}


// /** Web service operation '__tan__GetSupportedAnalyticsModules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(struct soap * soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse)
{
	printf("\nCalled __tan__GetSupportedAnalyticsModules\n");

    // tan__GetSupportedAnalyticsModules->ConfigurationToken
	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules = (struct  tt__SupportedAnalyticsModules *) soap_malloc(soap, sizeof(struct tt__SupportedAnalyticsModules));
	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules, 0x00, sizeof(struct tt__SupportedAnalyticsModules));


	// printf("\nCalled __tan__GetSupportedAnalyticsModules Limit\n");
	
	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->Limit = (int *) soap_malloc(soap, sizeof(int));
	*(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->Limit) = 1;
	
	//Content ScheMa Location
	// printf("\nCalled __tan__GetSupportedAnalyticsModules contentSchema\n");
	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleContentSchemaLocation = 1;
	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation = (CHARPTR *)soap_malloc(soap, sizeof(CHARPTR));
	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation[0] = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);

	// MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation[0], 0x00, sizeof(CHAR) * INFO_LENGTH);
	// SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation[0],INFO_LENGTH,"%s","http://www.onvif.org/ver20/analytics");
	// // SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation[0],INFO_LENGTH,"%s","http://www.example.com/schema/anyURI");

	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation = NULL;

	// printf("\nCalled __tan__GetSupportedAnalyticsModules Extension\n");
	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->Extension = NULL;

	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription = 4;

	const char* AnalyticsModuleTypes[] = {"tt:TripWireDetectionEngine","tt:TrespassingDetectionEngine","tt:MotionDetectionEngine","tt:TamperDetectionEngine"}; 
	const char* ParentTopics[] = {"tns1:RuleEngine/TripWire/Event","tns1:RuleEngine/Trespassing/Event","tns1:RuleEngine/Motion/Event","tns1:RuleEngine/Tamper/Event"};

	// AnalyticsModuleDescription
    tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription = (struct tt__ConfigDescription *) soap_malloc(soap, (tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription)*sizeof(struct tt__ConfigDescription));
	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription, 0x00, (tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription)*sizeof(struct tt__ConfigDescription));

	for (int index=0; index<tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription; index++)
	{

	
		//AnalyticsModuleDescription->fixed
		// printf("\nCalled __tan__GetSupportedAnalyticsModules Fixed\n");

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].fixed = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
		*(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].fixed) = xsd__boolean__false_;

		//AnalyticsModuleDescription->Maxinstances
		// printf("\nCalled __tan__GetSupportedAnalyticsModules maxInstances\n");
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].maxInstances = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].maxInstances, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].maxInstances,INFO_LENGTH,"%d",1);

		//AnalyticsModuleDescription->Name
		// printf("\nCalled __tan__GetSupportedAnalyticsModules Name\n");

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Name ,INFO_LENGTH,"%s",AnalyticsModuleTypes[index]);


		//AnalyticsModuleDescription->Parameters
		// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters = NULL;
		// printf("\nCalled __tan__GetSupportedAnalyticsModules Parameters\n");
		
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters, 0x00, sizeof(struct tt__ItemListDescription));

		
		// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->__anyAttribute = NULL;
		// dom_att(&tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

		// AnalyticsModuleDescription->Parameters->SimpleItemDescription
		// printf("\nCalled __tan__GetSupportedAnalyticsModules SimpleItemDescription\n");
		
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->__sizeSimpleItemDescription = 1;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
		
		// printf("\nCalled __tan__GetSupportedAnalyticsModules SimpleItemDescription Name\n");
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription->Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription->Name,INFO_LENGTH,"%s","active");
		// printf("\nCalled __tan__GetSupportedAnalyticsModules SimpleItemDescription Type\n");

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription->Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->SimpleItemDescription->Type,INFO_LENGTH,"%s","xsd:boolean");

		// AnalyticsModuleDescription->Parameters->ElementItemDescription
		// printf("\nCalled __tan__GetSupportedAnalyticsModules ElementItemDescription \n");
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->__sizeElementItemDescription = 0;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription = NULL;
		// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription = (struct _tt__ItemListDescription_ElementItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_ElementItemDescription));
		// MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
		
		// printf("\nCalled __tan__GetSupportedAnalyticsModules ElementItemDescription Name\n");

		// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription->Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription->Name,INFO_LENGTH,"%s","ElementItem1");
		// printf("\nCalled __tan__GetSupportedAnalyticsModules ElementItemDescription Type\n");

		// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription->Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->ElementItemDescription->Type,INFO_LENGTH,"%s","tt:params");



		// AnalyticsModuleDescription[0].Parameters->Extension
		// printf("\nCalled __tan__GetSupportedAnalyticsModules Extension\n");
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Parameters->Extension = NULL;


		//AnalyticsModuleDescription->Messages
		// printf("\nCalled __tan__GetSupportedAnalyticsModules Messages\n");
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].__sizeMessages = 1;


		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages = (struct _tt__ConfigDescription_Messages *) soap_malloc(soap, sizeof(struct _tt__ConfigDescription_Messages));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages, 0x00, sizeof(struct _tt__ConfigDescription_Messages));
		
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->IsProperty = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
		*(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->IsProperty) = xsd__boolean__true_;

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source, 0x00, sizeof(struct tt__ItemListDescription));

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->__sizeSimpleItemDescription = 3;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[0].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[0].Name,INFO_LENGTH,"%s","VideoSourceConfigurationToken");

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[0].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[0].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[0].Type,INFO_LENGTH,"%s","tt:ReferenceToken");
		

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[1].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[1].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[1].Name,INFO_LENGTH,"%s","VideoAnalyticsConfigurationToken");

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[1].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[1].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[1].Type,INFO_LENGTH,"%s","tt:ReferenceToken");

			tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[2].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[2].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[2].Name,INFO_LENGTH,"%s","Rule");

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[2].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[2].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->SimpleItemDescription[2].Type,INFO_LENGTH,"%s","xsd:string");

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->__sizeElementItemDescription = 0;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->ElementItemDescription = NULL;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Source->Extension = NULL;


		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data = (struct tt__ItemListDescription*)soap_malloc(soap, sizeof(struct tt__ItemListDescription));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data, 0x00, sizeof(struct tt__ItemListDescription));

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->__sizeSimpleItemDescription = 1;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription->Name,0x00, sizeof(CHAR)*INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription->Name,INFO_LENGTH,"%s","IsMotion");


		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription->Type,0x00, sizeof(CHAR)*INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->SimpleItemDescription->Type,INFO_LENGTH,"%s","xsd:boolean");


		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->ElementItemDescription = NULL;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Data->Extension = NULL;



		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->ParentTopic = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
		MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->ParentTopic, 0x00, sizeof(CHAR)*INFO_LENGTH);
		SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->ParentTopic,INFO_LENGTH,"%s",ParentTopics[index]);

		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Extension = NULL;
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Messages->Key = NULL;
		//AnalyticsModuleDescription->Extension
		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[index].Extension = NULL;

		
		printf("\nCalled __tan__GetSupportedAnalyticsModules END\n");
	}

    return SOAP_OK;
}

// SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(struct soap * soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse)
// {
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules\n");

//     return SOAP_OK;
// }


/** Web service operation '__tan__GetSupportedAnalyticsModules' implementation, should return SOAP_OK or error code */
// SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedAnalyticsModules(struct soap * soap, struct _tan__GetSupportedAnalyticsModules *tan__GetSupportedAnalyticsModules, struct _tan__GetSupportedAnalyticsModulesResponse *tan__GetSupportedAnalyticsModulesResponse)
// {
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules\n");

//     // tan__GetSupportedAnalyticsModules->ConfigurationToken
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules = (struct  tt__SupportedAnalyticsModules *) soap_malloc(soap, sizeof(struct tt__SupportedAnalyticsModules));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules, 0x00, sizeof(struct tt__SupportedAnalyticsModules));


// 	printf("\nCalled __tan__GetSupportedAnalyticsModules Limit\n");
	
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->Limit = (int *) soap_malloc(soap, sizeof(int));
// 	*(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->Limit) = 1;
	
// 	//Content ScheMa Location
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules contentSchema\n");
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleContentSchemaLocation = 1;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation = (CHARPTR *)soap_malloc(soap, sizeof(CHARPTR));
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation[0] = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);

// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation[0], 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation[0],INFO_BUFFER_LENGTH,"%s","http://www.example.com/schema/anyURI");

// 	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleContentSchemaLocation = NULL;

// 	printf("\nCalled __tan__GetSupportedAnalyticsModules Extension\n");
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->Extension = NULL;

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription = 1;
// 	// AnalyticsModuleDescription
//     tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription = (struct tt__ConfigDescription *) soap_malloc(soap, (tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription)*sizeof(struct tt__ConfigDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription, 0x00, (tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->__sizeAnalyticsModuleDescription)*sizeof(struct tt__ConfigDescription));


// 	//AnalyticsModuleDescription->fixed
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules Fixed\n");

//     tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].fixed = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
// 	// MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].fixed, 0x00, sizeof(enum xsd__boolean) * INFO_BUFFER_LENGTH);
// 	// SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].fixed ,INFO_BUFFER_LENGTH,"%d",xsd__boolean__true_);
//     *(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].fixed) = xsd__boolean__true_;

// 	//AnalyticsModuleDescription->Maxinstances
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules maxInstances\n");
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].maxInstances = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].maxInstances, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].maxInstances,INFO_BUFFER_LENGTH,"%d",1);

// 	//AnalyticsModuleDescription->Name
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules Name\n");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Name , 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Name ,INFO_BUFFER_LENGTH,"%s","tt:CellMotionEngine");


// 	//AnalyticsModuleDescription->Parameters
// 	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters = NULL;
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules Parameters\n");
	
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters, 0x00, sizeof(struct tt__ItemListDescription));

	
// 	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->__anyAttribute = NULL;
// 	// dom_att(&tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

// 	// AnalyticsModuleDescription->Parameters->SimpleItemDescription
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules SimpleItemDescription\n");
	
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->__sizeSimpleItemDescription = 1;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
	
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules SimpleItemDescription Name\n");
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription->Name, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription->Name,INFO_BUFFER_LENGTH,"%s","Sensitivity");
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules SimpleItemDescription Type\n");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription->Type, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->SimpleItemDescription->Type,INFO_BUFFER_LENGTH,"%s","xsd:integer");

// 	// AnalyticsModuleDescription->Parameters->ElementItemDescription
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules ElementItemDescription \n");
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->__sizeElementItemDescription = 1;
// 	// tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription = NULL;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription = (struct _tt__ItemListDescription_ElementItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_ElementItemDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
	
// 	// printf("\nCalled __tan__GetSupportedAnalyticsModules ElementItemDescription Name\n");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription->Name, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription->Name,INFO_BUFFER_LENGTH,"%s","Layout");
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules ElementItemDescription Type\n");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription->Type, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->ElementItemDescription->Type,INFO_BUFFER_LENGTH,"%s","tt:CellLayout");



// 	// AnalyticsModuleDescription[0].Parameters->Extension
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules Extension\n");
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Parameters->Extension = NULL;


// 	//AnalyticsModuleDescription->Messages
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules Messages\n");
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].__sizeMessages = 1;


// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages = (struct _tt__ConfigDescription_Messages *) soap_malloc(soap, sizeof(struct _tt__ConfigDescription_Messages));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages, 0x00, sizeof(struct _tt__ConfigDescription_Messages));
	
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->IsProperty = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
// 	*(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->IsProperty) = xsd__boolean__true_;

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source, 0x00, sizeof(struct tt__ItemListDescription));

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->__sizeSimpleItemDescription = 3;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[0].Name, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[0].Name,INFO_BUFFER_LENGTH,"%s","VideoSourceConfigurationToken");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[0].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[0].Type, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[0].Type,INFO_BUFFER_LENGTH,"%s","tt:ReferenceToken");
	

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[1].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[1].Name, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[1].Name,INFO_BUFFER_LENGTH,"%s","VideoAnalyticsConfigurationToken");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[1].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[1].Type, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[1].Type,INFO_BUFFER_LENGTH,"%s","tt:ReferenceToken");

// 		tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[2].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[2].Name, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[2].Name,INFO_BUFFER_LENGTH,"%s","Rule");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[2].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[2].Type, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->SimpleItemDescription[2].Type,INFO_BUFFER_LENGTH,"%s","xsd:string");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->__sizeElementItemDescription = 0;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->ElementItemDescription = NULL;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Source->Extension = NULL;


// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data = (struct tt__ItemListDescription*)soap_malloc(soap, sizeof(struct tt__ItemListDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data, 0x00, sizeof(struct tt__ItemListDescription));

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->__sizeSimpleItemDescription = 1;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription, 0x00, tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription->Name,0x00, sizeof(CHAR)*INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription->Name,INFO_BUFFER_LENGTH,"%s","IsMotion");


// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription->Type,0x00, sizeof(CHAR)*INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->SimpleItemDescription->Type,INFO_BUFFER_LENGTH,"%s","xsd:boolean");


// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->ElementItemDescription = NULL;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Data->Extension = NULL;



// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->ParentTopic = (CHARPTR)soap_malloc(soap, INFO_BUFFER_LENGTH*sizeof(CHAR));
// 	MEMSET(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->ParentTopic, 0x00, sizeof(CHAR)*INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->ParentTopic,INFO_BUFFER_LENGTH,"%s","tns1:RuleEngine/CellMotionDetector/Motion");

// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Extension = NULL;
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Messages->Key = NULL;


// 	//AnalyticsModuleDescription->Extension
// 	tan__GetSupportedAnalyticsModulesResponse->SupportedAnalyticsModules->AnalyticsModuleDescription[0].Extension = NULL;

	
// 	printf("\nCalled __tan__GetSupportedAnalyticsModules END\n");
	

//     return SOAP_OK;
// }





// /** Web service operation '__tan__CreateAnalyticsModules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__CreateAnalyticsModules(struct soap * soap, struct _tan__CreateAnalyticsModules *tan__CreateAnalyticsModules, struct _tan__CreateAnalyticsModulesResponse *tan__CreateAnalyticsModulesResponse)
{
	printf("\n---------------------__tan__CreateAnalyticsModules-----------------\n");
    return SOAP_OK;
}

// /** Web service operation '__tan__DeleteAnalyticsModules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteAnalyticsModules(struct soap * soap, struct _tan__DeleteAnalyticsModules *tan__DeleteAnalyticsModules, struct _tan__DeleteAnalyticsModulesResponse *tan__DeleteAnalyticsModulesResponse)
{
	printf("\n---------------------__tan__DeleteAnalyticsModules-----------------\n");

    return SOAP_OK;
}
/** Web service operation '__tan__GetAnalyticsModules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules(struct soap * soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse)
{
	printf("\n---------------------__tan__GetAnalyticsModules-----------------\n");

	
	sqlite3 *db;
	char *sql;
	int rc;
	sqlite3_stmt *res;
	int active = 0;
	/* LEGACY DB DISABLED: /oem/usr/share/usermng.db no longer exists.
	 * Analytics module config should use modern config system.
	 * For now, returning default values without DB access. */
	db = NULL;
	rc = SQLITE_OK;  // Skip DB open
   // rc = sqlite3_open("/oem/usr/share/usermng.db", &db);
	if( rc ) {
		printf("==========Can't open database: %s==========\n", sqlite3_errmsg(db));
		return SOAP_ERR;
	}

	sql = "SELECT active FROM analytics_modules WHERE name = ?";


	tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule = 4;

	tan__GetAnalyticsModulesResponse->AnalyticsModule = (struct  tt__Config*) soap_malloc(soap, tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule*sizeof(struct tt__Config));
	printf("\n---------------------__tan__GetAnalyticsModules-----------------1\n");
	    
	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule, 0x00, tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule*sizeof(struct tt__Config));
	printf("\n---------------------__tan__GetAnalyticsModules-----------------2\n");

	const char* AnalyticsModuleTypes[] = {"tt:TripWireDetectionEngine","tt:TrespassingDetectionEngine","tt:MotionDetectionEngine","tt:TamperDetectionEngine"};
	const char* AnalyticsModuleName[] = {"TripWireDetectionModule","TrespassingDetectionModule","MotionDetectionModule","TamperDetectionModule"};

	for(int index = 0; index<tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule; index++)
	{
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Name ,INFO_LENGTH,"%s",AnalyticsModuleName[index]);

		printf("\n---------------------__tan__GetAnalyticsModules-----------------5\n");
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Type , 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Type ,INFO_LENGTH,"%s",AnalyticsModuleTypes[index]);

		
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
		MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters, 0x00, sizeof( struct tt__ItemList));
		
		//tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->SimpleItem 
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->__sizeSimpleItem = 1;
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*)soap_malloc(soap, (tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof(struct _tt__ItemList_SimpleItem));
		MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem, 0x00, (tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->__sizeSimpleItem)*sizeof( struct _tt__ItemList_SimpleItem));
		//tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->SimpleItem->Name
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem[0].Name = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
		MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem[0].Name, 0x00, INFO_LENGTH*sizeof( CHAR));
		SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem[0].Name ,INFO_LENGTH,"%s","active");
		////tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->SimpleItem->Value
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem[0].Value = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
		MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem[0].Value, 0x00, INFO_LENGTH*sizeof( CHAR));
		
		if (db != NULL) {
			rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
			if (rc != SQLITE_OK) {
				fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
				sqlite3_close(db);
				return SOAP_ERR;
			}
			sqlite3_bind_text(res, 1, AnalyticsModuleName[index], -1, SQLITE_TRANSIENT);

			rc = sqlite3_step(res);
			if (rc == SQLITE_ROW) {
				active = sqlite3_column_int(res, 0);
				printf("Active value: %d\n", active);
			} else {
				printf("No record found\n");
			}
		} else {
			/* DB disabled - use default inactive value */
			active = 0;
		}

		SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->SimpleItem[0].Value ,INFO_LENGTH,"%d",active);

		// tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->__sizeElementItem 
		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->__sizeElementItem = 0;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem = (struct _tt__ItemList_ElementItem *)soap_malloc(soap, (tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->__sizeElementItem)*sizeof(struct _tt__ItemList_ElementItem));
		// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem, 0x00, (tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->__sizeElementItem)*sizeof( struct _tt__ItemList_ElementItem));
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].Name = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(char));
		// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].Name, 0x00, INFO_LENGTH*sizeof( char));
		// SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].Name ,INFO_LENGTH,"%s","ElementItem1");


		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.next = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.prnt = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.elts = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
		// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.elts, 0x00, sizeof(struct soap_dom_element));
		
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.atts = (struct soap_dom_attribute *)soap_malloc(soap, sizeof(struct soap_dom_attribute));
		// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.atts, 0x00, sizeof(struct soap_dom_attribute));
		// dom_att(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.atts, 1, name_analytics, motionConfigArmed, NULL, NULL, soap);
		
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.nstr = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.name,INFO_LENGTH,"%s","tt:params");
		
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.text = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.type = 0;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.node = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.tail = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.soap = soap;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.code = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->ElementItem[0].__any.lead = NULL;


		tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->Extension = NULL;
		// tan__GetAnalyticsModulesResponse->AnalyticsModule[index].Parameters->__anyAttribute = NULL;

		printf("\n---------------------__tan__GetAnalyticsModules-----------------9\n");
		if (db != NULL) {
			sqlite3_finalize(res);
		}
	}

	
    // Close the database connection
    if (db != NULL) {
        sqlite3_close(db);
    }

    return SOAP_OK;
}

// SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules(struct soap * soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse)
// {
// 	printf("\n---------------------__tan__GetAnalyticsModules-----------------\n");

// 		return SOAP_OK;
// }

// SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModules(struct soap * soap, struct _tan__GetAnalyticsModules *tan__GetAnalyticsModules, struct _tan__GetAnalyticsModulesResponse *tan__GetAnalyticsModulesResponse)
// {
// 	printf("\n---------------------__tan__GetAnalyticsModules-----------------\n");

	

// 	tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule = 1;

// 	tan__GetAnalyticsModulesResponse->AnalyticsModule = (struct  tt__Config*) soap_malloc(soap, tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule*sizeof(struct tt__Config));
// 	printf("\n---------------------__tan__GetAnalyticsModules-----------------1\n");
	    
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule, 0x00, tan__GetAnalyticsModulesResponse->__sizeAnalyticsModule*sizeof(struct tt__Config));
// 	printf("\n---------------------__tan__GetAnalyticsModules-----------------2\n");

	


// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Name , 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Name ,INFO_BUFFER_LENGTH,"%s","MyCellMotionModule");

// 	printf("\n---------------------__tan__GetAnalyticsModules-----------------5\n");
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Type , 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Type ,INFO_BUFFER_LENGTH,"%s","tt:CellMotionEngine");

	
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters = (struct tt__ItemList*)soap_malloc(soap, sizeof(struct tt__ItemList));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters, 0x00, sizeof( struct tt__ItemList));
	
// 	//tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->SimpleItem 
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->__sizeSimpleItem = 1;
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem = (struct _tt__ItemList_SimpleItem*)soap_malloc(soap, (tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->__sizeSimpleItem)*sizeof(struct _tt__ItemList_SimpleItem));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem, 0x00, (tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->__sizeSimpleItem)*sizeof( struct _tt__ItemList_SimpleItem));
// 	//tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->SimpleItem->Name
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem[0].Name = (char *)soap_malloc(soap, INFO_BUFFER_LENGTH*sizeof(char));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem[0].Name, 0x00, INFO_BUFFER_LENGTH*sizeof( char));
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem[0].Name ,INFO_BUFFER_LENGTH,"%s","Sensitivity");
// 	////tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->SimpleItem->Value
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem[0].Value = (char *)soap_malloc(soap, INFO_BUFFER_LENGTH*sizeof(char));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem[0].Value, 0x00, INFO_BUFFER_LENGTH*sizeof( char));
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->SimpleItem[0].Value ,INFO_BUFFER_LENGTH,"%s","100");



// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule->Parameters->__sizeElementItem 
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->__sizeElementItem = 1;
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem = (struct _tt__ItemList_ElementItem *)soap_malloc(soap, (tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->__sizeElementItem)*sizeof(struct _tt__ItemList_ElementItem));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem, 0x00, (tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->__sizeElementItem)*sizeof( struct _tt__ItemList_ElementItem));
// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].Name = (char *)soap_malloc(soap, INFO_BUFFER_LENGTH*sizeof(char));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].Name, 0x00, INFO_BUFFER_LENGTH*sizeof( char));
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].Name ,INFO_BUFFER_LENGTH,"%s","Layout");


// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name ,INFO_BUFFER_LENGTH,"%s","tt:CellLayout");

// 	soap_att_text(soap_att(&tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any, NULL, "Columns"), "22");
// 	soap_att_text(soap_att(&tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any, NULL, "Rows"), "18");

// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts, 0x00, sizeof(struct soap_dom_element));

// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->name ,INFO_BUFFER_LENGTH,"%s","tt:Transformation");

// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts, 0x00, sizeof(struct soap_dom_element));

// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->name ,INFO_BUFFER_LENGTH,"%s","tt:Translate");

// 	soap_att_text(soap_att(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts, NULL, "x"), "-1.000");
// 	soap_att_text(soap_att(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts, NULL, "y"), "1.000");


// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->next = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->next, 0x00, sizeof(struct soap_dom_element));


// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->next->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->next->name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->next->name ,INFO_BUFFER_LENGTH,"%s","tt:Scale");

// 	soap_att_text(soap_att(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->next, NULL, "x"), "0.0909");
// 	soap_att_text(soap_att(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->elts->next, NULL, "y"), "-0.111");


	


// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.next = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.prnt = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts = (struct soap_dom_element *)soap_malloc(soap, sizeof(struct soap_dom_element));
// 	// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts, 0x00, sizeof(struct soap_dom_element));
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.elts->name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	// SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name,INFO_BUFFER_LENGTH,"%s","tt:params");

// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.atts = (struct soap_dom_attribute *)soap_malloc(soap, sizeof(struct soap_dom_attribute));
// 	// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.atts, 0x00, sizeof(struct soap_dom_attribute));
// 	// dom_att(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.atts, 1, name_analytics, motionConfigArmed, NULL, NULL, soap);
	
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.nstr = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	// MEMSET(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name, 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	// SNPRINTF(tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.name,INFO_BUFFER_LENGTH,"%s","tt:params");
	
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.text = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.type = 0;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.node = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.tail = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.soap = soap;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.code = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->ElementItem[0].__any.lead = NULL;


// 	tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->Extension = NULL;
// 	// tan__GetAnalyticsModulesResponse->AnalyticsModule[0].Parameters->__anyAttribute = NULL;


// 	printf("\n---------------------__tan__GetAnalyticsModules-----------------9\n");

//     return SOAP_OK;
// }




/** Web service operation '__tan__GetAnalyticsModuleOptions' implementation, should return SOAP_OK or error code */
// SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModuleOptions(struct soap * soap, struct _tan__GetAnalyticsModuleOptions *tan__GetAnalyticsModuleOptions, struct _tan__GetAnalyticsModuleOptionsResponse *tan__GetAnalyticsModuleOptionsResponse)
// {

// 	printf("\n---------------------__tan__GetAnalyticsModuleOptions-----------------\n");

//     tan__GetAnalyticsModuleOptionsResponse->__sizeOptions = 1;
// 	tan__GetAnalyticsModuleOptionsResponse->Options = (struct tan__ConfigOptions*)soap_malloc(soap, tan__GetAnalyticsModuleOptionsResponse->__sizeOptions*sizeof(struct tan__ConfigOptions));
// 	MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options, 0x00, tan__GetAnalyticsModuleOptionsResponse->__sizeOptions*sizeof( struct tan__ConfigOptions));

// 	tan__GetAnalyticsModuleOptionsResponse->Options[0].AnalyticsModule = (char*)soap_malloc(soap, sizeof(char));
// 	MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options[0].AnalyticsModule, 0x00, sizeof( char));
// 	SNPRINTF(tan__GetAnalyticsModuleOptionsResponse->Options[0].AnalyticsModule,INFO_BUFFER_LENGTH,"%s","tt:CellMotionEngine");

// 	tan__GetAnalyticsModuleOptionsResponse->Options[0].Name = (char*)soap_malloc(soap, sizeof(char));
// 	MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options[0].Name, 0x00, sizeof( char));
// 	SNPRINTF(tan__GetAnalyticsModuleOptionsResponse->Options[0].Name,INFO_BUFFER_LENGTH,"%s","Sensitivity");

// 	// struct _tt__StringItems *types = (struct _tt__StringItems*)soap_malloc(soap, sizeof(struct _tt__StringItems));
// 	// types->__sizeItem = 3;
// 	// types->Item =  (char **)soap_malloc(soap, types->__sizeItem*sizeof(char*));
// 	// MEMSET(types->Item ,0x00, types->__sizeItem*sizeof(char*));
// 	// const char *strings[] = {"Motion Detection","Object Tracking","SimpleItem1Value"};

// 	// for (int index = 0; index < types->__sizeItem; index++) 
// 	// {
// 	// 	types->Item[index] = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * SMALL_BUFFER_LENGTH);
// 	// 	MEMSET(types->Item[index], 0x00, sizeof(CHAR) * SMALL_BUFFER_LENGTH);
// 	// 	SNPRINTF(types->Item[index], sizeof(CHAR) * SMALL_BUFFER_LENGTH, "%s", strings[index]);
// 	// }


// 	struct tt__IntRange *types = (struct tt__IntRange*)soap_malloc(soap, sizeof(struct tt__IntRange));
// 	types->Max = 100;
// 	types->Min = 0;

// 	soap_elt_set(&tan__GetAnalyticsModuleOptionsResponse->Options[0].__any,NULL,"tt:IntRange");
// 	soap_elt_node(&tan__GetAnalyticsModuleOptionsResponse->Options[0].__any, types,SOAP_TYPE_tt__IntRange);
// 	// soap_att(&tan__GetAnalyticsModuleOptionsResponse->Options[0].__any,NULL,"tt:StringItems");

// 	// tan__GetAnalyticsModuleOptionsResponse->Options[0].__any = 


// 	// tan__GetAnalyticsModuleOptionsResponse->Options[0].RuleType = NULL;
// 	// tan__GetAnalyticsModuleOptionsResponse->Options[0].RuleType = (char*)soap_malloc(soap, sizeof(char));
// 	// MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options[0].RuleType, 0x00, sizeof( char));
// 	// SNPRINTF(tan__GetAnalyticsModuleOptionsResponse->Options[0].RuleType,INFO_BUFFER_LENGTH,"%s","tt:PerimeterBreach");

// 	tan__GetAnalyticsModuleOptionsResponse->Options[0].maxOccurs = (int*)soap_malloc(soap, sizeof(int));
// 	*(tan__GetAnalyticsModuleOptionsResponse->Options[0].maxOccurs) = 1;

// 	tan__GetAnalyticsModuleOptionsResponse->Options[0].minOccurs = (int*)soap_malloc(soap, sizeof(int));
// 	*(tan__GetAnalyticsModuleOptionsResponse->Options[0].minOccurs) = 1;


// 	// tan__GetAnalyticsModuleOptionsResponse->Options[0].__any = NULL;
// 	// tan__GetAnalyticsModuleOptionsResponse->Options[0].__anyAttribute = NULL;


// 	return SOAP_OK;
// }



SOAP_FMAC5 int SOAP_FMAC6 __tan__GetAnalyticsModuleOptions(struct soap * soap, struct _tan__GetAnalyticsModuleOptions *tan__GetAnalyticsModuleOptions, struct _tan__GetAnalyticsModuleOptionsResponse *tan__GetAnalyticsModuleOptionsResponse)
{

	printf("\n---------------------__tan__GetAnalyticsModuleOptions-----------------\n");

    tan__GetAnalyticsModuleOptionsResponse->__sizeOptions = 4;
	tan__GetAnalyticsModuleOptionsResponse->Options = (struct tan__ConfigOptions*)soap_malloc(soap, tan__GetAnalyticsModuleOptionsResponse->__sizeOptions*sizeof(struct tan__ConfigOptions));
	MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options, 0x00, tan__GetAnalyticsModuleOptionsResponse->__sizeOptions*sizeof( struct tan__ConfigOptions));

	const char* AnalyticsModuleTypes[] = {"tt:TripWireDetectionEngine","tt:TrespassingDetectionEngine","tt:MotionDetectionEngine","tt:TamperDetectionEngine"};

	for(int index = 0; index< tan__GetAnalyticsModuleOptionsResponse->__sizeOptions; index++)
	{

		tan__GetAnalyticsModuleOptionsResponse->Options[index].AnalyticsModule = (char*)soap_malloc(soap, sizeof(char));
		MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options[index].AnalyticsModule, 0x00, sizeof( char));
		SNPRINTF(tan__GetAnalyticsModuleOptionsResponse->Options[index].AnalyticsModule,INFO_BUFFER_LENGTH,"%s",AnalyticsModuleTypes[index]);

		tan__GetAnalyticsModuleOptionsResponse->Options[index].Name = (char*)soap_malloc(soap, sizeof(char));
		MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options[index].Name, 0x00, sizeof( char));
		SNPRINTF(tan__GetAnalyticsModuleOptionsResponse->Options[index].Name,INFO_BUFFER_LENGTH,"%s","active");

		struct _tt__StringItems *types = (struct _tt__StringItems*)soap_malloc(soap, sizeof(struct _tt__StringItems));
		types->__sizeItem = 2;
		types->Item =  (char **)soap_malloc(soap, types->__sizeItem*sizeof(char*));
		MEMSET(types->Item ,0x00, types->__sizeItem*sizeof(char*));
		const char *strings[] = {"True","False"};

		for (int index = 0; index < types->__sizeItem; index++) 
		{
			types->Item[index] = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * SMALL_BUFFER_LENGTH);
			MEMSET(types->Item[index], 0x00, sizeof(CHAR) * SMALL_BUFFER_LENGTH);
			SNPRINTF(types->Item[index], sizeof(CHAR) * SMALL_BUFFER_LENGTH, "%s", strings[index]);
		}


		// struct tt__IntRange *types = (struct tt__IntRange*)soap_malloc(soap, sizeof(struct tt__IntRange));
		// types->Max = 100;
		// types->Min = 0;

		// soap_elt_set(&tan__GetAnalyticsModuleOptionsResponse->Options[index].__any,NULL,"tt:IntRange");
		// soap_elt_node(&tan__GetAnalyticsModuleOptionsResponse->Options[index].__any, types,SOAP_TYPE_tt__IntRange);
		soap_att(&tan__GetAnalyticsModuleOptionsResponse->Options[index].__any,NULL,"tt:StringItems");
		soap_elt_node(&tan__GetAnalyticsModuleOptionsResponse->Options[index].__any, types,SOAP_TYPE__tt__StringItems);

		// tan__GetAnalyticsModuleOptionsResponse->Options[index].__any = 


		// tan__GetAnalyticsModuleOptionsResponse->Options[index].RuleType = NULL;
		// tan__GetAnalyticsModuleOptionsResponse->Options[index].RuleType = (char*)soap_malloc(soap, sizeof(char));
		// MEMSET(tan__GetAnalyticsModuleOptionsResponse->Options[index].RuleType, 0x00, sizeof( char));
		// SNPRINTF(tan__GetAnalyticsModuleOptionsResponse->Options[index].RuleType,INFO_BUFFER_LENGTH,"%s","tt:PerimeterBreach");

		tan__GetAnalyticsModuleOptionsResponse->Options[index].maxOccurs = (int*)soap_malloc(soap, sizeof(int));
		*(tan__GetAnalyticsModuleOptionsResponse->Options[index].maxOccurs) = 1;

		tan__GetAnalyticsModuleOptionsResponse->Options[index].minOccurs = (int*)soap_malloc(soap, sizeof(int));
		*(tan__GetAnalyticsModuleOptionsResponse->Options[index].minOccurs) = 1;
	}


	// tan__GetAnalyticsModuleOptionsResponse->Options[index].__any = NULL;
	// tan__GetAnalyticsModuleOptionsResponse->Options[index].__anyAttribute = NULL;


	return SOAP_OK;
}




/** Web service operation '__tan__ModifyAnalyticsModules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyAnalyticsModules(struct soap * soap, struct _tan__ModifyAnalyticsModules *tan__ModifyAnalyticsModules, struct _tan__ModifyAnalyticsModulesResponse *tan__ModifyAnalyticsModulesResponse)
{

	printf("\n---------------------__tan__ModifyAnalyticsModules-----------------\n");
	sqlite3 *db;
	// char sql[1024];
	
	int rc;
	sqlite3_stmt *res;
	int active = 0;
	char active_str[10];
	int new_len;
	/* LEGACY DB DISABLED: /oem/usr/share/usermng.db no longer exists.
	 * Analytics module config should use modern config system.
	 * For now, returning default values without DB access. */
	db = NULL;
	rc = SQLITE_OK;  // Skip DB open
   	// rc = sqlite3_open("/oem/usr/share/usermng.db", &db);
	if( rc ) {
		printf("==in func %s========Can't open database: %s==========\n", __func__, sqlite3_errmsg(db));
		return SOAP_ERR;
	}

	/* LEGACY DB DISABLED - skip update when db is NULL */
	if (db == NULL) {
		printf("==in func %s======== DB disabled, skipping analytics module update ==========\n", __func__);
		model_update();
		return SOAP_OK;
	}

	/* Whitelist of allowed column names to prevent SQL injection */
	static const char *allowed_analytics_columns[] = {"active", NULL};

	const char * base_sql = "UPDATE analytics_modules SET ";
	char* sql = (char*)malloc(INFO_BUFFER_LENGTH);
	if (sql == NULL) {
        printf("==in func %s========Failed to allocate Memory for base sql==========\n", __func__);
		sqlite3_close(db);
        return SOAP_ERR;
    }
	strcpy(sql, base_sql);

    // const char *column_name = tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[0].Name;
    // SNPRINTF(sql, sizeof(sql), "UPDATE analytics_modules SET %s = ? WHERE name = ? AND type = ?", column_name);

    // Prepare the SQL statement
	printf("\n==in func %s====parameter %s %s, size %d \n",__func__,tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[0].Name,tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[0].Value, tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->__sizeSimpleItem);
	char *placeholder = (char*)malloc(TOKEN_LENGTH);
	if (placeholder == NULL) {
        printf("==in func %s========Failed to allocate Memory for placeholder==========\n", __func__);
		free(sql);
		sqlite3_close(db);
        return SOAP_ERR;
    }
	strcpy(placeholder," = ?");

	/* Use parameterized WHERE clause to prevent SQL injection */
	char *end_query = (char*)malloc(INFO_LENGTH);
	if (end_query == NULL) {
        printf("==in func %s========Failed to allocate Memory for end_query==========\n", __func__);
		free(sql);
		free(placeholder);
		sqlite3_close(db);
        return SOAP_ERR;
    }
	strcpy(end_query, " WHERE name = ? AND type = ?;");

	int param_count = 0;  /* Track number of SET clause parameters for bind offsets */
	for (int index = 0; index < tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->__sizeSimpleItem; index++) {
		/* Validate column name against whitelist */
		int allowed = 0;
		for (int w = 0; allowed_analytics_columns[w] != NULL; w++) {
			if (strcmp(tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[index].Name, allowed_analytics_columns[w]) == 0) {
				allowed = 1;
				break;
			}
		}
		if (!allowed) {
			printf("==in func %s========Rejected invalid column name: %s==========\n", __func__,
				tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[index].Name);
			continue;
		}

		if (param_count > 0) {
			strcat(sql, ", ");
		}
		if (strcmp(tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[index].Name, "active") == 0) {
			if (strcmp(tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[index].Value, "false") == 0) {
				active = 0;

			}
			else if (strcmp(tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[index].Value, "true") == 0) {
				active = 1;
			}
			strcat(sql, "active");
			strcat(sql, placeholder);
			param_count++;
			continue;
		}

		strcat(sql, tan__ModifyAnalyticsModules->AnalyticsModule->Parameters->SimpleItem[index].Name);
		strcat(sql, placeholder);
		param_count++;
	}

	if (param_count == 0) {
		printf("==in func %s========No valid columns to update==========\n", __func__);
		free(sql);
		free(placeholder);
		free(end_query);
		sqlite3_close(db);
		return SOAP_OK;
	}

	strcat(sql, end_query);

	printf("\n===sql query: %s =======\n",sql);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if (rc != SQLITE_OK) {
        printf("==in func %s========Failed to prepare statement: %s==========\n", __func__, sqlite3_errmsg(db));
        sqlite3_close(db);
		free(sql);
		free(placeholder);
		free(end_query);
        return SOAP_ERR;
    }

	

	sqlite3_bind_int(res, 1, active);
    /* Bind WHERE clause parameters (after SET params) */
    sqlite3_bind_text(res, param_count + 1, tan__ModifyAnalyticsModules->AnalyticsModule->Name, -1, SQLITE_STATIC);
    sqlite3_bind_text(res, param_count + 2, tan__ModifyAnalyticsModules->AnalyticsModule->Type, -1, SQLITE_STATIC);

	rc = sqlite3_step(res);
    if (rc != SQLITE_DONE) {
        printf("==in func %s========Failed to execute statement: %s==========\n", __func__, sqlite3_errmsg(db));
        // sqlite3_finalize(res);
        sqlite3_close(db);
		free(sql);
		free(placeholder);
		free(end_query);
        return SOAP_ERR;
    }

    // Finalize and close
    sqlite3_finalize(res);
    sqlite3_close(db);
	free(sql);
	free(placeholder);
	free(end_query);
	printf("\n===sql query update success =======\n");
	model_update();
    return SOAP_OK;
}
/** Web service operation '__tan__GetSupportedMetadata' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedMetadata(struct soap * soap, struct _tan__GetSupportedMetadata *tan__GetSupportedMetadata, struct _tan__GetSupportedMetadataResponse *tan__GetSupportedMetadataResponse)
{
	printf("\n---------------------__tan__GetSupportedMetadata-----------------\n");
	
    return SOAP_OK;
}


/** Web service operation '__tan__GetSupportedRules' implementation, should return SOAP_OK or error code */
// SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedRules(struct soap * soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse)
// {
// 	printf("\n\n=====in __tan__GetSupportedRules ======\n\n");

	
// 	if ((NULL == tan__GetSupportedRules->ConfigurationToken) || (0 == strlen(tan__GetSupportedRules->ConfigurationToken)))
// 	{
// 		printf("\n\n=====Exiting in __tan__GetSupportedRules. No Token ======\n\n");
// 		return SOAP_FAULT;
// 	}


//     // tan__GetSupportedAnalyticsModules->ConfigurationToken
// 	tan__GetSupportedRulesResponse->SupportedRules = (struct  tt__SupportedAnalyticsModules *) soap_malloc(soap, sizeof(struct tt__SupportedAnalyticsModules));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules, 0x00, sizeof(struct tt__SupportedRules));


// 	printf("\nCalled __tan__GetSupportedRules Limit\n");
	
// 	tan__GetSupportedRulesResponse->SupportedRules->Limit = (int *) soap_malloc(soap, sizeof(int));
// 	*(tan__GetSupportedRulesResponse->SupportedRules->Limit) = 1;
	
// 	//Content ScheMa Location
// 	printf("\nCalled __tan__GetSupportedRules contentSchema\n");
// 	// tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleContentSchemaLocation = 1;
// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation = (CHARPTR *)soap_malloc(soap, sizeof(CHARPTR));
// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0] = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);

// 	// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0], 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
// 	// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0],INFO_BUFFER_LENGTH,"%s","http://www.example.com/schema/anyURI");

// 	// tan__GetSupportedRulesResponse->SupportedRules->AnalyticsModuleContentSchemaLocation = NULL;

// 	printf("\nCalled __tan__GetSupportedRules Extension\n");
// 	tan__GetSupportedRulesResponse->SupportedRules->Extension = NULL;

// 	tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription = 1;
// 	// AnalyticsModuleDescription
//     tan__GetSupportedRulesResponse->SupportedRules->RuleDescription = (struct tt__ConfigDescription *) soap_malloc(soap, (tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription)*sizeof(struct tt__ConfigDescription));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription, 0x00, (tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription)*sizeof(struct tt__ConfigDescription));


// 	//RuleDescription->fixed
// 	printf("\nCalled __tan__GetSupportedRules Fixed\n");

//     tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].fixed = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
//     *(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].fixed) = xsd__boolean__true_;

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Name ,INFO_LENGTH,"%s","tt:MotionRegionDetector");

// 	//RuleDescription->Maxinstances
// 	printf("\nCalled __tan__GetSupportedRules maxInstances\n");
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].maxInstances = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].maxInstances, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].maxInstances,INFO_LENGTH,"%d",1);

// 	//RuleDescription->Name
// 	printf("\nCalled __tan__GetSupportedRules Name\n");




// 	//RuleDescription->Parameters
// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters = NULL;
// 	printf("\nCalled __tan__GetSupportedRules Parameters\n");
	
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters, 0x00, sizeof(struct tt__ItemListDescription));

	
// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->__anyAttribute = NULL;
// 	// dom_att(&tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

// 	// RuleDescription->Parameters->SimpleItemDescription
// 	printf("\nCalled __tan__GetSupportedRules SimpleItemDescription\n");
	
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->__sizeSimpleItemDescription = 0;
// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
// 	// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
	
// 	// printf("\nCalled __tan__GetSupportedRules SimpleItemDescription Name\n");
// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription->Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription->Name,INFO_LENGTH,"%s","SimpleItem1");
// 	// printf("\nCalled __tan__GetSupportedRules SimpleItemDescription Type\n");

// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription->Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->SimpleItemDescription->Type,INFO_LENGTH,"%s","xsd:string");

// 	// RuleDescription->Parameters->ElementItemDescription
// 	printf("\nCalled __tan__GetSupportedRules ElementItemDescription \n");
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->__sizeElementItemDescription = 1;
// 	// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription = NULL;
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription = (struct _tt__ItemListDescription_ElementItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_ElementItemDescription));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
	
// 	// printf("\nCalled __tan__GetSupportedRules ElementItemDescription Name\n");

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription->Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription->Name,INFO_LENGTH,"%s","MotionRegion");
// 	printf("\nCalled __tan__GetSupportedRules ElementItemDescription Type\n");

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription->Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->ElementItemDescription->Type,INFO_LENGTH,"%s","axt:MotionRegionConfig");



// 	// RuleDescription[0].Parameters->Extension
// 	printf("\nCalled __tan__GetSupportedRules Extension\n");
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Parameters->Extension = NULL;


// 	//RuleDescription->Messages
// 	printf("\nCalled __tan__GetSupportedRules Messages\n");
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].__sizeMessages = 1;


// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages = (struct _tt__ConfigDescription_Messages *) soap_malloc(soap, sizeof(struct _tt__ConfigDescription_Messages));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages, 0x00, sizeof(struct _tt__ConfigDescription_Messages));
	
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->IsProperty = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
// 	*(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->IsProperty) = xsd__boolean__true_;

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source, 0x00, sizeof(struct tt__ItemListDescription));

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->__sizeSimpleItemDescription = 3;
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[0].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[0].Name,INFO_LENGTH,"%s","VideoSource");

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[0].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[0].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[0].Type,INFO_LENGTH,"%s","tt:ReferenceToken");
	

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[1].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[1].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[1].Name,INFO_LENGTH,"%s","VideoAnalyticsConfigurationToken");

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[1].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[1].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[1].Type,INFO_LENGTH,"%s","tt:ReferenceToken");

// 		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[2].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[2].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[2].Name,INFO_LENGTH,"%s","RuleName");

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[2].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[2].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->SimpleItemDescription[2].Type,INFO_LENGTH,"%s","xsd:string");

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->__sizeElementItemDescription = 0;
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->ElementItemDescription = NULL;
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Source->Extension = NULL;


// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data = (struct tt__ItemListDescription*)soap_malloc(soap, sizeof(struct tt__ItemListDescription));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data, 0x00, sizeof(struct tt__ItemListDescription));

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->__sizeSimpleItemDescription = 1;
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription->Name,0x00, sizeof(CHAR)*INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription->Name,INFO_LENGTH,"%s","State");


// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription->Type,0x00, sizeof(CHAR)*INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->SimpleItemDescription->Type,INFO_LENGTH,"%s","xsd:boolean");


// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->ElementItemDescription = NULL;
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Data->Extension = NULL;



// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->ParentTopic = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
// 	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->ParentTopic, 0x00, sizeof(CHAR)*INFO_LENGTH);
// 	SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->ParentTopic,INFO_LENGTH,"%s","tns1:RuleEngine/MotionRegionDetector/Motion");

// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Extension = NULL;
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Messages->Key = NULL;


// 	//RuleDescription->Extension
// 	tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[0].Extension = NULL;

	
// 	printf("\nCalled __tan__GetSupportedRules END\n");
	

//     return SOAP_OK;
// }

SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRules(struct soap * soap, struct _tan__GetRules *tan__GetRules, struct _tan__GetRulesResponse *tan__GetRulesResponse)
{
	printf("\n---------------------__tan__GetRules-----------------\n");

	sqlite3 *db;
	char *sql;
	int rc;
	int rule_count = 0, column_count = 0;
	const char *column_name;
	sqlite3_stmt *res;
	int active = 0;
	/* LEGACY DB DISABLED */
	db = NULL;
	rc = SQLITE_OK;
	// rc = sqlite3_open("/oem/usr/share/usermng.db", &db);
	if( rc ) {
		printf("==========Can't open database: %s==========\n", sqlite3_errmsg(db));
		return SOAP_ERR;
	}

	/* When DB is disabled, return 0 rules */
	if (db == NULL) {
		tan__GetRulesResponse->__sizeRule = 0;
		tan__GetRulesResponse->Rule = NULL;
		return SOAP_OK;
	}

	sql = "SELECT COUNT(*) FROM analytics_rules";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return SOAP_ERR;
    }

    // Execute the SQL statement and fetch results
    rc = sqlite3_step(res);
    if (rc == SQLITE_ROW) {
        rule_count = sqlite3_column_int(res, 0);
        printf("Number of entries in analytics_modules: %d\n", rule_count);
    } else {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
		sqlite3_finalize(res); // Finalize the prepared statement
    	sqlite3_close(db);
		return SOAP_ERR;
    }
	sqlite3_finalize(res);


	tan__GetRulesResponse->__sizeRule = rule_count;

	tan__GetRulesResponse->Rule = (struct  tt__Config *) soap_malloc(soap,  tan__GetRulesResponse->__sizeRule * sizeof(struct tt__Config));
	MEMSET(tan__GetRulesResponse->Rule, 0x00, tan__GetRulesResponse->__sizeRule  * sizeof(struct tt__Config));


	sql = "SELECT * FROM analytics_rules";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return SOAP_ERR;
    }

    // // Iterate through the result set
    // while (sqlite3_step(res) == SQLITE_ROW) {
    //     column_count = sqlite3_column_count(res);
    //     for (int i = 0; i < column_count; i++) 
	// 	{
    //         column_name = sqlite3_column_name(res, i);
    //         const char *column_text = (const char *)sqlite3_column_text(res, i);
    //         printf("%s: %s\n", column_name, column_text ? column_text : "NULL");
    //     }
    //     printf("\n");
    // }
	const char *parameters[] = {"label","active","coordinates"};
	size_t parameter_length = sizeof(parameters) / sizeof(parameters[0]);
	for(int index = 0; index < tan__GetRulesResponse->__sizeRule; index++)
	{
		rc = sqlite3_step(res);
		if (rc != SQLITE_ROW) {
			fprintf(stderr, "%s Failed to step statement in RuleEngineConfiguration: %s\n", __func__, sqlite3_errmsg(db));
			sqlite3_finalize(res);
			sqlite3_close(db);
			return SOAP_ERR;
		}
		
		printf("\nsql column count: %s\n", sqlite3_column_text(res, 1));
		tan__GetRulesResponse->Rule[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
		MEMSET(tan__GetRulesResponse->Rule[index].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
		// SNPRINTF(tan__GetRulesResponse->Rule[index].Name,TOKEN_LENGTH,"%s","MotionRegionRule");
		SNPRINTF(tan__GetRulesResponse->Rule[index].Name,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 1));

		tan__GetRulesResponse->Rule[index].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
		MEMSET(tan__GetRulesResponse->Rule[index].Type, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
		// SNPRINTF(tan__GetRulesResponse->Rule[index].Type,TOKEN_LENGTH,"%s","tt:MotionRegionDetector");
		SNPRINTF(tan__GetRulesResponse->Rule[index].Type,TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 2));

		tan__GetRulesResponse->Rule[index].Parameters = (struct  tt__ItemList *) soap_malloc(soap, sizeof(struct tt__ItemList));
		MEMSET(tan__GetRulesResponse->Rule[index].Parameters, 0x00, sizeof(struct tt__ItemList));

		tan__GetRulesResponse->Rule[index].Parameters->__sizeSimpleItem = 3;
		tan__GetRulesResponse->Rule[index].Parameters->SimpleItem = (struct  _tt__ItemList_SimpleItem *) soap_malloc(soap, tan__GetRulesResponse->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));
		MEMSET(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem, 0x00, tan__GetRulesResponse->Rule[index].Parameters->__sizeSimpleItem * sizeof(struct _tt__ItemList_SimpleItem));

		for(int index_i=0; index_i<tan__GetRulesResponse->Rule[index].Parameters->__sizeSimpleItem; index_i++)
		{
			tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
			MEMSET(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
			// SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Name, TOKEN_LENGTH,"%s","MotionRegion");
			SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Name, TOKEN_LENGTH,"%s",parameters[index_i]);

			tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
			MEMSET(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
			// SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Value, TOKEN_LENGTH,"%s","SimpleItem1Value");
			SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[index_i].Value, TOKEN_LENGTH,"%s",(const char *)sqlite3_column_text(res, 4+index_i));
		}
		// tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
		// MEMSET(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[0].Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
		// SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[0].Name, TOKEN_LENGTH,"%s","MotionRegion");

		// tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[0].Value = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
		// MEMSET(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[0].Value, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
		// SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->SimpleItem[0].Value, TOKEN_LENGTH,"%s","SimpleItem1Value");
		

		// tan__GetRulesResponse->Rule[index].Parameters->__sizeElementItem = 1;
		// tan__GetRulesResponse->Rule[index].Parameters->ElementItem = (struct  _tt__ItemList_ElementItem *) soap_malloc(soap, tan__GetRulesResponse->Rule[index].Parameters->__sizeElementItem * sizeof(struct _tt__ItemList_ElementItem));
		// MEMSET(tan__GetRulesResponse->Rule[index].Parameters->ElementItem, 0x00, tan__GetRulesResponse->Rule[index].Parameters->__sizeElementItem * sizeof(struct _tt__ItemList_ElementItem));

		// tan__GetRulesResponse->Rule[index].Parameters->ElementItem->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
		// MEMSET(tan__GetRulesResponse->Rule[index].Parameters->ElementItem->Name, 0x00, sizeof(CHAR) * TOKEN_LENGTH);
		// SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->ElementItem->Name, TOKEN_LENGTH,"%s",MOTION_REGION);


		// tan__GetRulesResponse->Rule[index].Parameters->ElementItem->__any.name =  (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetRulesResponse->Rule[index].Parameters->ElementItem->__any.name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetRulesResponse->Rule[index].Parameters->ElementItem->__any.name,INFO_LENGTH,"%s","axt:MotionRegionConfig");

		// struct _axt__MotionRegionConfig *motion_region_config = soap_new__axt__MotionRegionConfig(soap,-1);
		// motion_region_config->Armed = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
		// *(motion_region_config->Armed) = xsd__boolean__true_;


		// motion_region_config->Sensitivity = (float *)soap_malloc(soap, sizeof(float));
		// *(motion_region_config->Sensitivity) = 0.5;

		// soap_elt_set(&tan__GetRulesResponse->Rule[index].Parameters->ElementItem->__any, NULL, "axt:MotionRegionConfig");
		// soap_elt_node(&tan__GetRulesResponse->Rule[index].Parameters->ElementItem->__any, motion_region_config, SOAP_TYPE__axt__MotionRegionConfig);

	}
	// Finalize the statement to clean up
    sqlite3_finalize(res);

    // Close the database connection
    sqlite3_close(db);
	return SOAP_OK;
}



SOAP_FMAC5 int SOAP_FMAC6 __tan__GetSupportedRules(struct soap * soap, struct _tan__GetSupportedRules *tan__GetSupportedRules, struct _tan__GetSupportedRulesResponse *tan__GetSupportedRulesResponse)
{
	printf("\n\n=====in __tan__GetSupportedRules ======\n\n");

	
	if ((NULL == tan__GetSupportedRules->ConfigurationToken) || (0 == strlen(tan__GetSupportedRules->ConfigurationToken)))
	{
		printf("\n\n=====Exiting in __tan__GetSupportedRules. No Token ======\n\n");
		return SOAP_FAULT;
	}


	sqlite3 *db;
	char *sql;
	int rc;
	int column_count = 0;
	const char *column_name;
	sqlite3_stmt *res;
	int active = 0;
	/* LEGACY DB DISABLED */
	db = NULL;
	rc = SQLITE_OK;
	// rc = sqlite3_open("/oem/usr/share/usermng.db", &db);
	if( rc ) {
		printf("==========Can't open database: %s==========\n", sqlite3_errmsg(db));
		return SOAP_ERR;
	}

	/* Only run PRAGMA when DB is available */
	if (db != NULL) {
		sql = "PRAGMA table_info(analytics_rules)";

		rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
		if (rc != SQLITE_OK) {
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
			sqlite3_close(db);
			return SOAP_ERR;
		}
	}


    // tan__GetSupportedAnalyticsModules->ConfigurationToken
	tan__GetSupportedRulesResponse->SupportedRules = (struct  tt__SupportedAnalyticsModules *) soap_malloc(soap, sizeof(struct tt__SupportedAnalyticsModules));
	MEMSET(tan__GetSupportedRulesResponse->SupportedRules, 0x00, sizeof(struct tt__SupportedRules));


	printf("\nCalled __tan__GetSupportedRules Limit\n");
	
	tan__GetSupportedRulesResponse->SupportedRules->Limit = (int *) soap_malloc(soap, sizeof(int));
	*(tan__GetSupportedRulesResponse->SupportedRules->Limit) = 1;
	
	//Content ScheMa Location
	// printf("\nCalled __tan__GetSupportedRules contentSchema\n");
	// tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleContentSchemaLocation = 1;
	// tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation = (CHARPTR *)soap_malloc(soap, sizeof(CHARPTR));
	// tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0] = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_BUFFER_LENGTH);

	// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0], 0x00, sizeof(CHAR) * INFO_BUFFER_LENGTH);
	// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleContentSchemaLocation[0],INFO_BUFFER_LENGTH,"%s","http://www.example.com/schema/anyURI");

	// tan__GetSupportedRulesResponse->SupportedRules->AnalyticsModuleContentSchemaLocation = NULL;

	// printf("\nCalled __tan__GetSupportedRules Extension\n");
	tan__GetSupportedRulesResponse->SupportedRules->Extension = NULL;

	tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription = 4;
	// AnalyticsModuleDescription
    tan__GetSupportedRulesResponse->SupportedRules->RuleDescription = (struct tt__ConfigDescription *) soap_malloc(soap, (tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription)*sizeof(struct tt__ConfigDescription));
	MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription, 0x00, (tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription)*sizeof(struct tt__ConfigDescription));

	const char* AnalyticsRuleTypes[] = {"tt:TripWireDetectionRuleEngine","tt:TrespassingDetectionRuleEngine","tt:MotionDetectionRuleEngine","tt:TamperDetectionRuleEngine"}; 
	const char* ParentTopics[] = {"tns1:RuleEngine/TripWire/Event","tns1:RuleEngine/Trespassing/Event","tns1:RuleEngine/Motion/Event","tns1:RuleEngine/Tamper/Event"};
	const char *parameters[] = {"label","active","coordinates"};
	const char *parameter_types[] = {"xsd:string","xsd:boolean","xsd:string"};
	// sqlite3_step(res);
	

	//RuleDescription->fixed
	// printf("\nCalled __tan__GetSupportedRules Fixed\n");

	for (int index = 0; index < tan__GetSupportedRulesResponse->SupportedRules->__sizeRuleDescription; index++)
	{

		if (db != NULL) {
			sqlite3_step(res);
		}
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].fixed = (enum  xsd__boolean *) soap_malloc(soap, sizeof(enum xsd__boolean));
		*(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].fixed) = xsd__boolean__true_;

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Name , 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Name ,INFO_LENGTH,"%s",AnalyticsRuleTypes[index]);

		//RuleDescription->Maxinstances
		// printf("\nCalled __tan__GetSupportedRules maxInstances\n");
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].maxInstances = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].maxInstances, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].maxInstances,INFO_LENGTH,"%d",1);

		//RuleDescription->Name
		// printf("\nCalled __tan__GetSupportedRules Name\n");




		//RuleDescription->Parameters
		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters = NULL;
		// printf("\nCalled __tan__GetSupportedRules Parameters\n");
		
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters, 0x00, sizeof(struct tt__ItemListDescription));

		
		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__anyAttribute = NULL;
		// dom_att(&tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__anyAttribute, 0, NULL, NULL, NULL, NULL, soap);

		// RuleDescription->Parameters->SimpleItemDescription
		// printf("\nCalled __tan__GetSupportedRules SimpleItemDescription\n");
		
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__sizeSimpleItemDescription = 3;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

		for(int index_i=0; index_i<tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__sizeSimpleItemDescription; index_i++)
		{
			tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription[index_i].Name  = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
			MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription[index_i].Name , 0x00, sizeof(CHAR) * TOKEN_LENGTH);
			SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription[index_i].Name , TOKEN_LENGTH,"%s",parameters[index_i]);

			tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription[index_i].Type  = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * TOKEN_LENGTH);
			MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription[index_i].Type , 0x00, sizeof(CHAR) * TOKEN_LENGTH);
			SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription[index_i].Type , TOKEN_LENGTH,"%s",parameter_types[index_i]);

		}




		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
		// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
		
		// printf("\nCalled __tan__GetSupportedRules SimpleItemDescription Name\n");
		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription->Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription->Name,INFO_LENGTH,"%s","SimpleItem1");
		// printf("\nCalled __tan__GetSupportedRules SimpleItemDescription Type\n");

		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription->Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->SimpleItemDescription->Type,INFO_LENGTH,"%s","xsd:string");




		// RuleDescription->Parameters->ElementItemDescription
		// printf("\nCalled __tan__GetSupportedRules ElementItemDescription \n");
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__sizeElementItemDescription = 0;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription = NULL;
		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription = (struct _tt__ItemListDescription_ElementItemDescription*)soap_malloc(soap, sizeof(struct _tt__ItemListDescription_ElementItemDescription));
		// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->__sizeSimpleItemDescription*sizeof( struct _tt__ItemListDescription_SimpleItemDescription));
		
		// // printf("\nCalled __tan__GetSupportedRules ElementItemDescription Name\n");

		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription->Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription->Name,INFO_LENGTH,"%s","MotionRegion");
		// printf("\nCalled __tan__GetSupportedRules ElementItemDescription Type\n");

		// tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		// MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription->Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		// SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->ElementItemDescription->Type,INFO_LENGTH,"%s","axt:MotionRegionConfig");



		// RuleDescription[0].Parameters->Extension
		// printf("\nCalled __tan__GetSupportedRules Extension\n");
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Parameters->Extension = NULL;


		//RuleDescription->Messages
		// printf("\nCalled __tan__GetSupportedRules Messages\n");
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].__sizeMessages = 1;


		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages = (struct _tt__ConfigDescription_Messages *) soap_malloc(soap, sizeof(struct _tt__ConfigDescription_Messages));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages, 0x00, sizeof(struct _tt__ConfigDescription_Messages));
		
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->IsProperty = (enum xsd__boolean *)soap_malloc(soap, sizeof(enum xsd__boolean));
		*(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->IsProperty) = xsd__boolean__true_;

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source = (struct tt__ItemListDescription *) soap_malloc(soap, sizeof(struct tt__ItemListDescription));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source, 0x00, sizeof(struct tt__ItemListDescription));

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->__sizeSimpleItemDescription = 3;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[0].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[0].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[0].Name,INFO_LENGTH,"%s","VideoSource");

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[0].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[0].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[0].Type,INFO_LENGTH,"%s","tt:ReferenceToken");
		

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[1].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[1].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[1].Name,INFO_LENGTH,"%s","VideoAnalyticsConfigurationToken");

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[1].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[1].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[1].Type,INFO_LENGTH,"%s","tt:ReferenceToken");

			tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[2].Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[2].Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[2].Name,INFO_LENGTH,"%s","RuleName");

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[2].Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[2].Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->SimpleItemDescription[2].Type,INFO_LENGTH,"%s","xsd:string");

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->__sizeElementItemDescription = 0;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->ElementItemDescription = NULL;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Source->Extension = NULL;


		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data = (struct tt__ItemListDescription*)soap_malloc(soap, sizeof(struct tt__ItemListDescription));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data, 0x00, sizeof(struct tt__ItemListDescription));

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->__sizeSimpleItemDescription = 1;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription = (struct _tt__ItemListDescription_SimpleItemDescription *) soap_malloc(soap, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription, 0x00, tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->__sizeSimpleItemDescription*sizeof(struct _tt__ItemListDescription_SimpleItemDescription));

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription->Name,0x00, sizeof(CHAR)*INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription->Name,INFO_LENGTH,"%s","State");


		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR)*INFO_LENGTH);
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription->Type,0x00, sizeof(CHAR)*INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->SimpleItemDescription->Type,INFO_LENGTH,"%s","xsd:boolean");


		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->ElementItemDescription = NULL;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Data->Extension = NULL;



		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->ParentTopic = (CHARPTR)soap_malloc(soap, INFO_LENGTH*sizeof(CHAR));
		MEMSET(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->ParentTopic, 0x00, sizeof(CHAR)*INFO_LENGTH);
		SNPRINTF(tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->ParentTopic,INFO_LENGTH,"%s",ParentTopics[index]);

		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Extension = NULL;
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Messages->Key = NULL;


		//RuleDescription->Extension
		tan__GetSupportedRulesResponse->SupportedRules->RuleDescription[index].Extension = NULL;
	}
	
	printf("\nCalled __tan__GetSupportedRules END\n");
	if (db != NULL) {
		sqlite3_finalize(res);
		sqlite3_close(db);
	}

    return SOAP_OK;
}





/** Web service operation '__tan__GetRuleOptions' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__GetRuleOptions(struct soap *soap, struct _tan__GetRuleOptions *tan__GetRuleOptions, struct _tan__GetRuleOptionsResponse *tan__GetRuleOptionsResponse)
{
	printf( "Called GetRuleOptions ");
 
	// CHAR *name_motionConfig[] = {"namespace", "processContents", "minOccurs","maxOccurs"};

	// CHAR *motionConfigNamespace = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	// MEMSET(motionConfigNamespace, 0x00, sizeof(CHAR) * INFO_LENGTH);
	// SNPRINTF(motionConfigNamespace,INFO_LENGTH,"%s","##any");

	// CHAR *motionConfigProcessContent = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	// MEMSET(motionConfigProcessContent, 0x00, sizeof(CHAR) * INFO_LENGTH);
	// SNPRINTF(motionConfigProcessContent,INFO_LENGTH,"%s","blabla");

	// CHAR *motionConfigMinOccurs = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	// MEMSET(motionConfigMinOccurs, 0x00, sizeof(CHAR) * INFO_LENGTH);
	// SNPRINTF(motionConfigMinOccurs,INFO_LENGTH,"%s",1);

	// CHAR *motionConfigMaxOccurs = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	// MEMSET(motionConfigMaxOccurs, 0x00, sizeof(CHAR) * INFO_LENGTH);
	// SNPRINTF(motionConfigMaxOccurs,INFO_LENGTH,"%s",1);

	// CHAR *data_motionConfig[] = { motionConfigNamespace,motionConfigProcessContent,motionConfigMinOccurs,motionConfigMaxOccurs};

	tan__GetRuleOptionsResponse->__sizeRuleOptions = 1;

	tan__GetRuleOptionsResponse->RuleOptions = (struct  tan__ConfigOptions *) soap_malloc(soap,  tan__GetRuleOptionsResponse->__sizeRuleOptions * sizeof(struct tan__ConfigOptions));
	MEMSET(tan__GetRuleOptionsResponse->RuleOptions, 0x00, tan__GetRuleOptionsResponse->__sizeRuleOptions * sizeof(struct tan__ConfigOptions));

	tan__GetRuleOptionsResponse->RuleOptions->RuleType = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	MEMSET(tan__GetRuleOptionsResponse->RuleOptions->RuleType, 0x00, sizeof(CHAR) * INFO_LENGTH);
	SNPRINTF(tan__GetRuleOptionsResponse->RuleOptions->RuleType, INFO_LENGTH,"%s","tt:MotionRegionDetector");

	tan__GetRuleOptionsResponse->RuleOptions->Name = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	MEMSET(tan__GetRuleOptionsResponse->RuleOptions->Name, 0x00, sizeof(CHAR) * INFO_LENGTH);
	SNPRINTF(tan__GetRuleOptionsResponse->RuleOptions->Name, INFO_LENGTH,"%s","MotionRegion");

	tan__GetRuleOptionsResponse->RuleOptions->Type = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	MEMSET(tan__GetRuleOptionsResponse->RuleOptions->Type, 0x00, sizeof(CHAR) * INFO_LENGTH);
	SNPRINTF(tan__GetRuleOptionsResponse->RuleOptions->Type, INFO_LENGTH,"%s","axt:MotionRegionConfigOptions");

	tan__GetRuleOptionsResponse->RuleOptions->AnalyticsModule = NULL;

	// tan__GetRuleOptionsResponse->RuleOptions->AnalyticsModule = (CHARPTR)soap_malloc(soap, sizeof(CHAR) * INFO_LENGTH);
	// MEMSET(tan__GetRuleOptionsResponse->RuleOptions->AnalyticsModule, 0x00, sizeof(CHAR) * INFO_LENGTH);
	// SNPRINTF(tan__GetRuleOptionsResponse->RuleOptions->AnalyticsModule, INFO_LENGTH,"%s","");

	tan__GetRuleOptionsResponse->RuleOptions->minOccurs = (int *)soap_malloc(soap, sizeof(int));
	*(tan__GetRuleOptionsResponse->RuleOptions->minOccurs) = 1;

	tan__GetRuleOptionsResponse->RuleOptions->maxOccurs = (int *)soap_malloc(soap, sizeof(int));
	*(tan__GetRuleOptionsResponse->RuleOptions->maxOccurs) = 1;



	return SOAP_OK;
}

/** Web service operation '__tan__ModifyRules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__ModifyRules(struct soap * soap, struct _tan__ModifyRules *tan__ModifyRules, struct _tan__ModifyRulesResponse *tan__ModifyRulesResponse)
{
	printf("\n-----------Called ModifyRules-------\n");

	sqlite3 *db;
	int rc;
	sqlite3_stmt *res;
	int active = 0;
	int new_len;
	/* Open database */
	/* LEGACY DB DISABLED */
	db = NULL;
	rc = SQLITE_OK;
   	// rc = sqlite3_open("/oem/usr/share/usermng.db", &db);
	if( rc ) {
		printf("==in func %s========Can't open database: %s==========\n", __func__, sqlite3_errmsg(db));
		return SOAP_ERR;
	}

	/* LEGACY DB DISABLED - skip update when db is NULL */
	if (db == NULL) {
		printf("==in func %s======== DB disabled, skipping rules update ==========\n", __func__);
		model_update();
		return SOAP_OK;
	}

	/* Whitelist of allowed column names to prevent SQL injection */
	static const char *allowed_rule_columns[] = {"label", "active", "coordinates", NULL};

	const char * base_sql = "UPDATE analytics_rules SET ";
	char* sql = (char*)malloc(INFO_BUFFER_LENGTH);
	if (sql == NULL) {
        printf("==in func %s========Failed to allocate Memory for base sql==========\n", __func__);
		sqlite3_close(db);
        return SOAP_ERR;
    }
	//Create Base Query
	strcpy(sql, base_sql);


	// Create Separater for parameters
	char *placeholder = (char*)malloc(TOKEN_LENGTH);
	if (placeholder == NULL) {
        printf("==in func %s========Failed to allocate Memory for placeholder==========\n", __func__);
		free(sql);
		sqlite3_close(db);
        return SOAP_ERR;
    }
	strcpy(placeholder," = ?");

	// Create End Query - use parameterized WHERE to prevent SQL injection
	char *end_query = (char*)malloc(INFO_LENGTH);
	if (end_query == NULL) {
        printf("==in func %s========Failed to allocate Memory for end_query==========\n", __func__);
		free(sql);
		free(placeholder);
		sqlite3_close(db);
        return SOAP_ERR;
    }

	strcpy(end_query, " WHERE name = ? AND type = ?");

	int param_count = 0;  /* Track number of SET clause parameters for bind offsets */
	for (int index = 0; index < tan__ModifyRules->Rule->Parameters->__sizeSimpleItem; index++) {
		/* Validate column name against whitelist */
		int allowed = 0;
		for (int w = 0; allowed_rule_columns[w] != NULL; w++) {
			if (strcmp(tan__ModifyRules->Rule->Parameters->SimpleItem[index].Name, allowed_rule_columns[w]) == 0) {
				allowed = 1;
				break;
			}
		}
		if (!allowed) {
			printf("==in func %s========Rejected invalid column name: %s==========\n", __func__,
				tan__ModifyRules->Rule->Parameters->SimpleItem[index].Name);
			continue;
		}

		if (param_count > 0) {
			strcat(sql, ", ");
		}
		if (strcmp(tan__ModifyRules->Rule->Parameters->SimpleItem[index].Name, "active") == 0) {
			if (strcmp(tan__ModifyRules->Rule->Parameters->SimpleItem[index].Value, "false") == 0) {
				active = 0;
			}
			else if (strcmp(tan__ModifyRules->Rule->Parameters->SimpleItem[index].Value, "true") == 0) {
				active = 1;
			}
		}

		//sql = UPDATE analytics_rules SET
		strcat(sql, tan__ModifyRules->Rule->Parameters->SimpleItem[index].Name);
		strcat(sql, placeholder); //sql = UPDATE analytics_rules SET active = ?
		param_count++;
	}

	if (param_count == 0) {
		printf("==in func %s========No valid columns to update==========\n", __func__);
		free(sql);
		free(placeholder);
		free(end_query);
		sqlite3_close(db);
		return SOAP_OK;
	}

	strcat(sql, end_query);
	printf("\n===sql query: %s =======\n",sql);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, NULL);
    if (rc != SQLITE_OK) {
        printf("==in func %s========Failed to prepare statement: %s==========\n", __func__, sqlite3_errmsg(db));
        sqlite3_close(db);
		free(sql);
		free(placeholder);
		free(end_query);
        return SOAP_ERR;
    }

	// Bind the values
	int bind_idx = 1;
	for (int index = 0; index < tan__ModifyRules->Rule->Parameters->__sizeSimpleItem; index++) {
		/* Skip columns that weren't whitelisted */
		int allowed = 0;
		for (int w = 0; allowed_rule_columns[w] != NULL; w++) {
			if (strcmp(tan__ModifyRules->Rule->Parameters->SimpleItem[index].Name, allowed_rule_columns[w]) == 0) {
				allowed = 1;
				break;
			}
		}
		if (!allowed) continue;

		if (strcmp(tan__ModifyRules->Rule->Parameters->SimpleItem[index].Name, "active") == 0) {
			sqlite3_bind_int(res, bind_idx++, active);
			continue;
		}
		sqlite3_bind_text(res, bind_idx++, tan__ModifyRules->Rule->Parameters->SimpleItem[index].Value, -1, SQLITE_STATIC);
	}

	/* Bind WHERE clause parameters */
	sqlite3_bind_text(res, bind_idx++, tan__ModifyRules->Rule->Name, -1, SQLITE_STATIC);
	sqlite3_bind_text(res, bind_idx++, tan__ModifyRules->Rule->Type, -1, SQLITE_STATIC);

	rc = sqlite3_step(res);
    if (rc != SQLITE_DONE) {
        printf("==in func %s========Failed to execute statement: %s==========\n", __func__, sqlite3_errmsg(db));
        // sqlite3_finalize(res);
        sqlite3_close(db);
		free(sql);
		free(placeholder);
		free(end_query);
        return SOAP_ERR;
    }

	    // Finalize and close
    sqlite3_finalize(res);
    sqlite3_close(db);
	free(sql);
	free(placeholder);
	free(end_query);
	printf("\n===sql query update success =======\n");
	model_update();

	return SOAP_OK;
}

/** Web service operation '__tan__DeleteRules' implementation, should return SOAP_OK or error code */
SOAP_FMAC5 int SOAP_FMAC6 __tan__DeleteRules(struct soap * soap, struct _tan__DeleteRules *tan__DeleteRules, struct _tan__DeleteRulesResponse *tan__DeleteRulesResponse)
{
printf("\n---------------------__tan__DeleteRules-----------------\n");
// TODO: Implement rule deletion for Novatek platform
return SOAP_OK;
}
