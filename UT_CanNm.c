/** ==================================================================================================================*\
  @file UT_CanNm.c

  @brief Unit tests for Can Network Management Module
\*====================================================================================================================*/
#define UNIT_TEST

/*====================================================================================================================*\
    Include headers
\*====================================================================================================================*/
#include "Std_Types.h"
#include "acutest.h"
#include "fff.h"
#include "CanNm.h"
#include "CanNm.c"

/*====================================================================================================================*\
    Local macros
\*====================================================================================================================*/
#define CANNM_CHANNEL_COUNT 1
#define CANNM_RXPDU_COUNT   1
#define CANNM_SDU_LENGTH    8

/*====================================================================================================================*\
    Local types
\*====================================================================================================================*/

/*====================================================================================================================*\
    Global variables
\*====================================================================================================================*/

/*====================================================================================================================*\
    Local variables (static)
\*====================================================================================================================*/

static uint8 TestRxMessageSdu[CANNM_SDU_LENGTH] = {1, 2, 3, 4, 5, 6, 7, 8};
static uint8 TestTxMessageSdu[CANNM_SDU_LENGTH] = {1, 2, 3, 4, 5, 6, 7, 8};

static PduInfoType canNmRxPduInfo = {
  .SduDataPtr = TestRxMessageSdu,
  .SduLength = CANNM_SDU_LENGTH
};

static PduInfoType canNmTxPduInfo = {
  .SduDataPtr = TestTxMessageSdu,
  .SduLength = CANNM_SDU_LENGTH
};

/**
 * @brief RX PDU
 *
 * RX PDU for CanNm module (10.2.6)
*/
static CanNm_RxPdu canNmRxPdu = {
  .RxPduId = 0,
  .RxPduRef = &canNmRxPduInfo
};

/**
 * @brief TX PDU
 *
 * TX PDU for CanNm module (10.2.7)
*/
static CanNm_TxPdu canNmTxPdu = {
  .TxConfirmationPduId = 0,
  .TxPduRef = &canNmTxPduInfo
};

/**
 * @brief User Data TX PDU
 *
 * User Data TX PDU for CanNm module (10.2.8)
*/
static CanNm_UserDataTxPdu canNmUserDataTxPdu = {
  .TxUserDataPduId = 0,
  .TxUserDataPduRef = &canNmTxPduInfo
};

/**
 * @brief Partial Network Filter Mask Byte
 *
 * Partial Network Filter Mask Byte for CanNm module (10.2.10)
*/
static CanNm_PnFilterMaskByte canNmPnFilterMaskByte = {
  .PnFilterMaskByteIndex = 0,
  .PnFilterMaskByteValue = 0
};

/**
 * @brief Partial Network Info
 *
 * Partial Network Info for CanNm module (10.2.9)
*/
static CanNm_PnInfo canNmPnInfo = {
  .PnInfoLength = 1,
  .PnInfoOffset = 0,
  .PnFilterMaskByte = &canNmPnFilterMaskByte
};

/**
 * @brief Test channel configuration
 *
 * Test channel configuration for CanNm (10.2.5)
*/
static CanNm_ChannelType canNmChannel[CANNM_CHANNEL_COUNT] = {
  [0] = {
	.TimeoutTime 			= 100,
	.MsgCycleOffset			= 5,
	.MsgCycleTime 			= 500,
	.RepeatMessageTime 		= 1000,
	.WaitBusSleepTime 		= 1000,
	.RemoteSleepIndTime 	= 2000,
    .PduCbvPosition  		= CANNM_PDU_BYTE_1,
    .PduNidPosition 		= CANNM_PDU_BYTE_0,
    .RxPdu[0]       		= &canNmRxPdu,
	.RxPdu[1]       		= &canNmRxPdu,
	.RxPdu[2]       		= &canNmRxPdu,
	.RxPdu[3]       		= &canNmRxPdu,
	.RxPdu[4]       		= &canNmRxPdu,
	.RxPdu[5]       		= &canNmRxPdu,
	.RxPdu[6]       		= &canNmRxPdu,
    .TxPdu          		= &canNmTxPdu,
    .UserDataTxPdu  		= &canNmUserDataTxPdu,
	.NodeDetectionEnabled 	= 1,
	.ActiveWakeupBitEnabled = 1,
	.NodeIdEnabled 			= 1,
	.RepeatMsgIndEnabled 	= 1
  }
};

/**
 * @brief Test configuration
 *
 * Test configuration for CanNm module (10.2.2)
*/
static CanNm_ConfigType canNmConfig = {
  .MainFunctionPeriod = 1.0,
  .ChannelConfig = {canNmChannel}
};

static NetworkHandleType nmChannelHandle = 0;

static PduIdType TxPduId = 0;
static PduIdType RxPduId = 0;

static uint8 SduDataPtr[CANNM_SDU_LENGTH];

static PduInfoType PduInfoPtr = {
	.SduDataPtr = SduDataPtr,
	.SduLength = CANNM_SDU_LENGTH
};

const PduInfoType cPduInfoPtr;

uint8 nmNodeIdPtr = 1;
uint8 nmPduDataPtr = 1;
Nm_StateType nmStatePtr = 1;
Nm_ModeType nmModePtr = 1;
boolean nmRemoteSleepIndPtr;
/*====================================================================================================================*\
    Local functions declarations
\*====================================================================================================================*/

/*====================================================================================================================*\
    Global inline functions and function macros code
\*====================================================================================================================*/

/*====================================================================================================================*\
    Global functions code
\*====================================================================================================================*/

/*====================================================================================================================*\
    Local functions (static) code
\*====================================================================================================================*/

/**
 * @brief Initialization test
 *
 * Function testing initialization for CanNm (7.4, 8.3.1)
*/
void Test_Of_CanNm_Init(void)
{
	/*Check if the CanNm module is correctly initialized*/
	CanNm_Init(&canNmConfig);
	TEST_CHECK(CanNm_Internal.Channels[0].State == NM_STATE_BUS_SLEEP);
	TEST_CHECK(CanNm_Internal.Channels[0].Requested == FALSE);
	TEST_CHECK(CanNm_Internal.Channels[0].Mode == NM_MODE_BUS_SLEEP);
	TEST_CHECK(CanNm_ConfigPtr == &canNmConfig);

	if (canNmConfig.GlobalPnSupport) {
        /* Check initialization for GlobalPnSupport */
		TEST_CHECK(CanNm_Internal.Channels[0].TimeoutTimer.State == CANNM_TIMER_STOPPED);
	}
	TEST_CHECK(CanNm_Internal.Channels[0].BusLoadReduction == 0);
	TEST_CHECK(CanNm_Internal.Channels[0].MessageCycleTimer.State == CANNM_TIMER_STOPPED);

	/* Check initialization of user data to 0xFF */
	uint8* destUserData = CanNm_Internal_GetUserDataPtr(canNmChannel, canNmChannel->TxPdu->TxPduRef->SduDataPtr);
	uint8 userDataLength = CanNm_Internal_GetUserDataLength(canNmChannel);
	for (uint8* ptr = destUserData; ptr < (destUserData + userDataLength); ptr++) {
		TEST_CHECK(*destUserData == 0xFF);
	}

	uint8 pduCbv = canNmChannel->TxPdu->TxPduRef->SduDataPtr[canNmChannel[0].PduCbvPosition];
	TEST_CHECK(pduCbv == 0x00);

}

void Test_Of_CanNm_DeInit(void)
{
    CanNm_Init(&canNmConfig);
    TEST_CHECK(CanNm_Internal.Channels[0].State == NM_STATE_BUS_SLEEP);

    /* Check if deinitialization sets the state to NM_STATE_UNINIT after state NM_STATE_BUS_SLEEP*/
	CanNm_DeInit();
	TEST_CHECK(CanNm_Internal.Channels[0].State == NM_STATE_UNINIT);
	TEST_CHECK(CanNm_Internal.InitStatus = CANNM_UNINIT);
}


void Test_Of_CanNm_PassiveStartUp(void)
{
	Std_ReturnType status;

	/* Check that PassiveStartUp returns E_NOT_OK when called without PassiveModeEnabled enabled*/
	canNmConfig.PassiveModeEnabled = 0;
	CanNm_Init(&canNmConfig);
	status = CanNm_PassiveStartUp(nmChannelHandle);
	TEST_CHECK(status == E_NOT_OK);
	CanNm_DeInit();

	/* Check that PassiveStartUp returns E_OK when called after initialization with PassiveModeEnabled enabled */
	canNmConfig.PassiveModeEnabled = 1;
	CanNm_Init(&canNmConfig);
	status = CanNm_PassiveStartUp(nmChannelHandle);
	TEST_CHECK(status == E_OK);

}

void Test_Of_CanNm_NetworkRequest(void)
{
    Std_ReturnType status;

	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

    // Check that NetworkRequest sets the channel to NM_MODE_NETWORK and NM_STATE_NORMAL_OPERATION
	CanNm_Init(&canNmConfig);
	status = CanNm_NetworkRequest(nmChannelHandle);
	TEST_CHECK(status == E_OK);
	TEST_CHECK(ChannelInternal->Requested == TRUE);
	TEST_CHECK(ChannelInternal->TxEnabled == TRUE);

	ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
	status = CanNm_NetworkRequest(nmChannelHandle);
	TEST_CHECK(status == E_OK);
	TEST_CHECK(ChannelInternal->Mode == NM_MODE_NETWORK);
	TEST_CHECK(ChannelInternal->State == NM_STATE_REPEAT_MESSAGE);
	TEST_CHECK(ChannelInternal->Requested == TRUE);

	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_READY_SLEEP;
	status = CanNm_NetworkRequest(nmChannelHandle);
	TEST_CHECK(status == E_OK);
	TEST_CHECK(ChannelInternal->Mode == NM_MODE_NETWORK);
	TEST_CHECK(ChannelInternal->State == NM_STATE_NORMAL_OPERATION);
	TEST_CHECK(ChannelInternal->Requested == TRUE);

	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
	status = CanNm_NetworkRequest(nmChannelHandle);
	TEST_CHECK(status == E_OK);
	TEST_CHECK(ChannelInternal->Mode == NM_MODE_NETWORK);
	TEST_CHECK(ChannelInternal->State == NM_STATE_NORMAL_OPERATION);
	TEST_CHECK(ChannelInternal->Requested == TRUE);

	ChannelInternal->State = NM_STATE_REPEAT_MESSAGE;
	status = CanNm_NetworkRequest(nmChannelHandle);
	TEST_CHECK(status == E_OK);
	TEST_CHECK(ChannelInternal->Mode == NM_MODE_NETWORK);
	TEST_CHECK(ChannelInternal->State == NM_STATE_REPEAT_MESSAGE);
	TEST_CHECK(ChannelInternal->Requested == TRUE);
}

void Test_Of_CanNm_NetworkRelease(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

	// Check that NetworkRelease sets the channel to NM_MODE_NETWORK and NM_STATE_READY_SLEEP
	CanNm_Init(&canNmConfig);
	status = CanNm_NetworkRelease(nmChannelHandle);
	TEST_CHECK(status == NM_E_OK);
	TEST_CHECK(ChannelInternal->Requested == FALSE);

	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
	status = CanNm_NetworkRelease(nmChannelHandle);
	TEST_CHECK(ChannelInternal->Mode == NM_MODE_NETWORK);
	TEST_CHECK(ChannelInternal->State == NM_STATE_READY_SLEEP);
	TEST_CHECK(ChannelInternal->Requested == FALSE);
	TEST_CHECK(ChannelInternal->TxEnabled == FALSE);


	CanNm_DeInit();
    CanNm_Init(&canNmConfig);
    const NetworkHandleType nmChannelHandle = 0;
    CanNm_Internal.Channels[nmChannelHandle].State = NM_STATE_NORMAL_OPERATION;
    CanNm_Internal.Channels[nmChannelHandle].Mode = NM_MODE_NETWORK;
    CanNm_Internal.Channels[nmChannelHandle].Requested = TRUE;

    status = CanNm_NetworkRelease(nmChannelHandle);
    TEST_CHECK(status == E_OK);

    // Check that Requested flag is set to FALSE after NetworkRelease
    TEST_CHECK(CanNm_Internal.Channels[nmChannelHandle].Requested == FALSE);

    // Check that the function transitions to ReadySleep if the channel was in NormalOperation
    if (CanNm_Internal.Channels[nmChannelHandle].Mode == NM_MODE_NETWORK &&
        CanNm_Internal.Channels[nmChannelHandle].State == NM_STATE_NORMAL_OPERATION) {
        TEST_CHECK(CanNm_Internal.Channels[nmChannelHandle].State == NM_STATE_READY_SLEEP);
    }


}

void Test_Of_CanNm_DisableCommunication(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

	// Check if DisableCommunication work in PassiveModeEnabled off and NM_MODE_BUS_SLEEP
	canNmConfig.PassiveModeEnabled = 0;
	CanNm_Init(&canNmConfig);
	ChannelInternal->Mode = NM_MODE_NETWORK;
	status = CanNm_DisableCommunication(nmChannelHandle);
	TEST_CHECK(status == E_OK);
	TEST_CHECK(ChannelInternal->TxEnabled == 0);
	ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
	CanNm_DeInit();

	// Check if DisableCommunication work in PassiveModeEnabled on and NM_MODE_BUS_SLEEP
	canNmConfig.PassiveModeEnabled = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->Mode = NM_MODE_NETWORK;
	status = CanNm_DisableCommunication(nmChannelHandle);
	TEST_CHECK(status == E_NOT_OK);
	CanNm_DeInit();

    // Check if DisableCommunication work in PassiveModeEnabled on on and incorrect mode
    CanNm_Init(&canNmConfig);
    canNmConfig.PassiveModeEnabled = 1;
    ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
    status = CanNm_DisableCommunication(nmChannelHandle);
    TEST_CHECK(status == E_NOT_OK);
    CanNm_DeInit();

    // Check if DisableCommunication work in PassiveModeEnabled off and incorrect mode
    canNmConfig.PassiveModeEnabled = 0;
    ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
    status = CanNm_DisableCommunication(nmChannelHandle);
    TEST_CHECK(status == E_NOT_OK);
    CanNm_DeInit();


}

void Test_Of_CanNm_EnableCommunication(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

	 /* Check if EnableCommunication works in NM_MODE_NETWORK with PassiveModeEnabled off */
	canNmConfig.CoordinationSyncSupport = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->Mode = NM_MODE_NETWORK;
	status = CanNm_EnableCommunication(nmChannelHandle);
	TEST_CHECK(status == E_OK);
	ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
	CanNm_DeInit();

	/* Check if EnableCommunication works in NM_MODE_NETWORK with PassiveModeEnabled on */
	canNmConfig.PassiveModeEnabled = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->MessageCycleTimer.State = CANNM_TIMER_STARTED;
	status = CanNm_EnableCommunication(nmChannelHandle);
	TEST_CHECK(status == E_NOT_OK);
	CanNm_DeInit();

	/* Check if EnableCommunication works in incorrect mode with PassiveModeEnabled on */
	canNmConfig.PassiveModeEnabled = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
	status = CanNm_EnableCommunication(nmChannelHandle);
	TEST_CHECK(status == E_NOT_OK);
	CanNm_DeInit();

	/* Check if EnableCommunication works in incorrect mode with PassiveModeEnabled off */
	canNmConfig.PassiveModeEnabled = 0;
	CanNm_Init(&canNmConfig);
	ChannelInternal->Mode = NM_MODE_PREPARE_BUS_SLEEP;
	status = CanNm_EnableCommunication(nmChannelHandle);
	TEST_CHECK(status == E_NOT_OK);
	CanNm_DeInit();
}

void Test_Of_CanNm_SetUserData(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;
	const uint8 cnmUserDataPtr = 1;

	// Check that SetUserData returns E_OK when UserDataEnabled is set
	canNmConfig.UserDataEnabled = 1;
	CanNm_Init(&canNmConfig);
	status = CanNm_SetUserData(nmChannelHandle, &cnmUserDataPtr);
	TEST_CHECK(status == E_OK);
	CanNm_DeInit();

	canNmConfig.ComUserDataSupport = 1;
	CanNm_Init(&canNmConfig);
	status = CanNm_SetUserData(nmChannelHandle, &cnmUserDataPtr);
	TEST_CHECK(status == E_NOT_OK);
}

void Test_Of_CanNm_GetUserData(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;
	uint8 nmUserData[CANNM_SDU_LENGTH];

	// Check that GetUserData returns E_OK when UserDataEnabled is set
	canNmConfig.UserDataEnabled = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->RxLastPdu = 1;
	status = CanNm_GetUserData(nmChannelHandle, nmUserData);
	TEST_CHECK(status == E_OK);

	// Check that GetUserData returns E_NOT_OK when NO_PDU_RECEIVED
	ChannelInternal->RxLastPdu = NO_PDU_RECEIVED;
	status = CanNm_GetUserData(nmChannelHandle, nmUserData);
	TEST_CHECK(status == E_NOT_OK);
}

void Test_Of_CanNm_Transmit(void)
{
	Std_ReturnType status;

	// Check that Transmit returns E_NOT_OK before initialization
	CanNm_Init(&canNmConfig);
	status = CanNm_Transmit(TxPduId, &PduInfoPtr);
	TEST_CHECK(status == E_NOT_OK);

	// Check that Transmit returns E_OK after proper configuration
	canNmConfig.GlobalPnSupport = 1;
	canNmConfig.ComUserDataSupport = 1;
	CanNm_Init(&canNmConfig);
	status = CanNm_Transmit(TxPduId, &cPduInfoPtr);
	TEST_CHECK(status == E_OK);
}

void Test_Of_CanNm_GetNodeIdentifier(void)
{
    // Check that GetNodeIdentifier returns NM_E_NOT_OK before reception
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

	CanNm_Init(&canNmConfig);
	status = CanNm_GetNodeIdentifier(nmChannelHandle, &nmNodeIdPtr);
	TEST_CHECK(status == NM_E_NOT_OK);

	// Check that GetNodeIdentifier returns NM_E_OK after reception
	ChannelInternal->RxLastPdu = 1;
	status = CanNm_GetNodeIdentifier(nmChannelHandle, &nmNodeIdPtr);
	TEST_CHECK(status == NM_E_OK);
}

void Test_Of_CanNm_GetLocalNodeIdentifier(void)
{
	Std_ReturnType status;

	// Check that GetLocalNodeIdentifier returns NM_E_OK after initialization
	CanNm_Init(&canNmConfig);
	status = CanNm_GetLocalNodeIdentifier(nmChannelHandle, &nmNodeIdPtr);
	TEST_CHECK(status == NM_E_OK);
}

void Test_Of_CanNm_RepeatMessageRequest(void)
{
    // Check that RepeatMessageRequest returns E_OK in NM_STATE_READY_SLEEP
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

	canNmConfig.ChannelConfig[0]->NodeDetectionEnabled = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->State = NM_STATE_READY_SLEEP;
	status = CanNm_RepeatMessageRequest(nmChannelHandle);
	TEST_CHECK(status == E_OK);

	// Check that RepeatMessageRequest returns E_NOT_OK in other states
	status = CanNm_RepeatMessageRequest(nmChannelHandle);
	TEST_CHECK(status == E_NOT_OK);
}

void Test_Of_CanNm_GetPduData(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

    // Check that GetPduData returns NM_E_NOT_OK before reception
	CanNm_Init(&canNmConfig);
	status = CanNm_GetPduData(nmChannelHandle, &nmPduDataPtr);
	TEST_CHECK(status == NM_E_NOT_OK);
	CanNm_DeInit();

	// Check that GetPduData returns NM_E_NOT_OK before reception
	canNmConfig.ChannelConfig[0]->NodeDetectionEnabled = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->RxLastPdu = 1;
	status = CanNm_GetPduData(nmChannelHandle, &nmPduDataPtr);
	TEST_CHECK(status == NM_E_OK);
}

void Test_Of_CanNm_GetState(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

	// Check that GetState returns NM_E_OK after initialization
	CanNm_Init(&canNmConfig);
	status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
	TEST_CHECK(status == NM_E_OK);
}


void Test_Of_CanNm_RequestBusSynchronization(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;

	 // Check that RequestBusSynchronization returns E_NOT_OK before proper setup
	CanNm_Init(&canNmConfig);
	status = CanNm_RequestBusSynchronization(nmChannelHandle);
	TEST_CHECK(status == E_NOT_OK);

	// Check that RequestBusSynchronization returns E_OK in NM_MODE_NETWORK
	ChannelInternal->Mode = NM_MODE_NETWORK;
	ChannelInternal->TxEnabled = 1;
	status = CanNm_RequestBusSynchronization(nmChannelHandle);
	TEST_CHECK(status == E_OK);
}

void Test_Of_CanNm_CheckRemoteSleepInd(void)
{
    CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];

    // Check incorrect state (NM_STATE_NORMAL_OPERATION), the function should return E_NOT_OK
    ChannelInternal->State = NM_STATE_BUS_SLEEP;
    Std_ReturnType status = CanNm_CheckRemoteSleepInd(nmChannelHandle, &nmRemoteSleepIndPtr);
    TEST_CHECK(status == E_NOT_OK);

    // Check correct NM_STATE_NORMAL_OPERATION state, the function should return E_OK
    ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
    status = CanNm_CheckRemoteSleepInd(nmChannelHandle, &nmRemoteSleepIndPtr);
    TEST_CHECK(status == E_OK);


}

void Test_Of_CanNm_SetSleepReadyBit(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status;
	boolean nmSleepReadyBit;

	// Check that SetSleepReadyBit returns E_NOT_OK before initialization
	CanNm_Init(&canNmConfig);
	status = CanNm_SetSleepReadyBit(nmChannelHandle, nmSleepReadyBit);
	TEST_CHECK(status == E_NOT_OK);
	CanNm_DeInit();

	// Check that SetSleepReadyBit returns E_OK in NM_STATE_NORMAL_OPERATION
	canNmConfig.CoordinationSyncSupport = 1;
	CanNm_Init(&canNmConfig);
	ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
	status = CanNm_SetSleepReadyBit(nmChannelHandle, nmSleepReadyBit);
	TEST_CHECK(status == E_OK);
}

void Test_Of_CanNm_TxConfirmation(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	Std_ReturnType status = E_OK;

	CanNm_Init(&canNmConfig);
	CanNm_TxConfirmation(nmChannelHandle, status);

}

void Test_Of_CanNm_ConfirmPnAvailability(void)
{
	CanNm_Internal_ChannelType* ChannelInternal = &CanNm_Internal.Channels[nmChannelHandle];
	CanNm_Init(&canNmConfig);
	CanNm_ConfirmPnAvailability(nmChannelHandle);
	TEST_CHECK(ChannelInternal->NmPduFilterAlgorithm = TRUE);
}

void Test_Of_CanNm_TriggerTransmit(void)
{
	Std_ReturnType status;

	CanNm_Init(&canNmConfig);
	/* Check that TriggerTransmit returns E_NOT_OK for SduLength = 0x1 */
	PduInfoPtr.SduLength = 0x1;
	status = CanNm_TriggerTransmit(TxPduId, &PduInfoPtr);
	TEST_CHECK(status == E_NOT_OK);
    /* Check that TriggerTransmit returns E_OK for valid SduLength */
	PduInfoPtr.SduLength = CANNM_SDU_LENGTH;
	status = CanNm_TriggerTransmit(TxPduId, &PduInfoPtr);
	TEST_CHECK(status == E_OK);
}

void Test_Of_State_Machine(void)
{
	Std_ReturnType status;
	uint32 tick;

	for (tick = 0; tick < 10000000UL; tick++){
		if (tick ==0){
			CanNm_Init(&canNmConfig);
            // Check if initial state is NM_STATE_BUS_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_BUS_SLEEP);
		}
		else if (tick ==50000){
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_BUS_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_BUS_SLEEP);
			TEST_CHECK(status == E_OK);
		}

		else if (tick ==70000){
			status = CanNm_NetworkRequest(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_REPEAT_MESSAGE
			TEST_CHECK(nmStatePtr == NM_STATE_REPEAT_MESSAGE);
			TEST_CHECK(status == E_OK);
		}
		else if (tick ==90000){
			status = CanNm_CheckRemoteSleepInd(nmChannelHandle, &nmRemoteSleepIndPtr);
			status = CanNm_RepeatMessageRequest(nmChannelHandle);
			status = CanNm_RequestBusSynchronization(nmChannelHandle);
			status = CanNm_RepeatMessageRequest(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_REPEAT_MESSAGE
			TEST_CHECK(nmStatePtr == NM_STATE_REPEAT_MESSAGE);
			TEST_CHECK(status == E_OK);
		}

		else if (tick ==220000){
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_NORMAL_OPERATION
			TEST_CHECK(nmStatePtr == NM_STATE_NORMAL_OPERATION);
			TEST_CHECK(status == E_OK);
		}

		else if (tick ==490000){
			status = CanNm_CheckRemoteSleepInd(nmChannelHandle, &nmRemoteSleepIndPtr);
			status = CanNm_RepeatMessageRequest(nmChannelHandle);
			status = CanNm_RequestBusSynchronization(nmChannelHandle);
			status = CanNm_RepeatMessageRequest(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_REPEAT_MESSAGE
			TEST_CHECK(nmStatePtr == NM_STATE_REPEAT_MESSAGE);
			TEST_CHECK(status == E_OK);
		}

		else if (tick ==1400000){
			status = CanNm_NetworkRelease(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_READY_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_READY_SLEEP);
			status = CanNm_PassiveStartUp(nmChannelHandle);
			status = CanNm_NetworkRequest(nmChannelHandle);
			status = CanNm_NetworkRelease(nmChannelHandle);
			TEST_CHECK(status == E_OK);
		}
		else if (tick ==1400101){
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_PREPARE_BUS_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_PREPARE_BUS_SLEEP);
			status = CanNm_NetworkRelease(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			TEST_CHECK(nmStatePtr == NM_STATE_PREPARE_BUS_SLEEP);
			TEST_CHECK(status == E_OK);
		}
		else if (tick ==1401180){
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_BUS_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_BUS_SLEEP);
			status = CanNm_NetworkRelease(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			TEST_CHECK(nmStatePtr == NM_STATE_BUS_SLEEP);
			TEST_CHECK(status == E_OK);
		}
		else if (tick ==1800000){
			status = CanNm_NetworkRequest(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_REPEAT_MESSAGE
			TEST_CHECK(nmStatePtr == NM_STATE_REPEAT_MESSAGE);
			TEST_CHECK(status == E_OK);
			status = CanNm_NetworkRelease(nmChannelHandle);
		}
		else if (tick ==1900000){
			status = CanNm_DisableCommunication(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_BUS_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_BUS_SLEEP);
			TEST_CHECK(status == E_OK);
			status = CanNm_NetworkRequest(nmChannelHandle);
		}
		else if (tick ==2500000){
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_NORMAL_OPERATION
			TEST_CHECK(nmStatePtr == NM_STATE_NORMAL_OPERATION);
			TEST_CHECK(status == E_OK);
			status = CanNm_RepeatMessageRequest(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_REPEAT_MESSAGE
			TEST_CHECK(nmStatePtr == NM_STATE_REPEAT_MESSAGE);
		}
		else if (tick ==2601050){
			status = CanNm_NetworkRelease(nmChannelHandle);
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_READY_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_READY_SLEEP);
			TEST_CHECK(status == E_OK);
		}
		else if (tick ==2605050){
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_BUS_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_BUS_SLEEP);
			CanNm_DeInit();
			// Check if initial state is NM_STATE_BUS_SLEEP
			TEST_CHECK(nmStatePtr == NM_STATE_BUS_SLEEP);
			TEST_CHECK(status == E_OK);
		}
        else if (tick ==2809050){
			status = CanNm_GetState(nmChannelHandle, &nmStatePtr, &nmModePtr);
			// Check if initial state is NM_STATE_UNINIT
			TEST_CHECK(nmStatePtr == NM_STATE_UNINIT);
			TEST_CHECK(status == E_OK);
		}
		CanNm_MainFunction();
	}

	TEST_CHECK(status == E_OK);

}
/*
  Test list - write down here all functions which should be executed as tests.
*/
TEST_LIST = {
  { "Test_Of_CanNm_Init", Test_Of_CanNm_Init },
  { "Test_Of_CanNm_DeInit", Test_Of_CanNm_DeInit },
  { "Test_Of_CanNm_PassiveStartUp", Test_Of_CanNm_PassiveStartUp },
  { "Test_Of_CanNm_NetworkRequest", Test_Of_CanNm_NetworkRequest },
  { "Test_Of_CanNm_NetworkRelease", Test_Of_CanNm_NetworkRelease },
  { "Test_Of_CanNm_DisableCommunication", Test_Of_CanNm_DisableCommunication },
  { "Test_Of_CanNm_EnableCommunication", Test_Of_CanNm_EnableCommunication },
  { "Test_Of_CanNm_SetUserData", Test_Of_CanNm_SetUserData },
  { "Test_Of_CanNm_GetUserData", Test_Of_CanNm_GetUserData },
  { "Test_Of_CanNm_Transmit", Test_Of_CanNm_Transmit },
  { "Test_Of_CanNm_GetNodeIdentifier", Test_Of_CanNm_GetNodeIdentifier },
  { "Test_Of_CanNm_GetLocalNodeIdentifier", Test_Of_CanNm_GetLocalNodeIdentifier },
  { "Test_Of_CanNm_RepeatMessageRequest", Test_Of_CanNm_RepeatMessageRequest },
  { "Test_Of_CanNm_GetPduData", Test_Of_CanNm_GetPduData },
  { "Test_Of_CanNm_GetState", Test_Of_CanNm_GetState },
  { "Test_Of_CanNm_RequestBusSynchronization", Test_Of_CanNm_RequestBusSynchronization },
  { "Test_Of_CanNm_CheckRemoteSleepInd", Test_Of_CanNm_CheckRemoteSleepInd },
  { "Test_Of_CanNm_SetSleepReadyBit", Test_Of_CanNm_SetSleepReadyBit },
  { "Test_Of_CanNm_TxConfirmation", Test_Of_CanNm_TxConfirmation },
  { "Test_Of_CanNm_ConfirmPnAvailability", Test_Of_CanNm_ConfirmPnAvailability },
  { "Test_Of_CanNm_TriggerTransmit", Test_Of_CanNm_TriggerTransmit },
  { "Test_Of_State_Machine", Test_Of_State_Machine },
  { NULL, NULL }
};

