#include "LibTalker.h"
#include <iostream>

LibTalker::LibTalker()
{
	m_bUseFDA = false;
	m_bUseSim = false;
	m_bUseActiveShield = false;
	m_bUseTriggers = false;
	m_bUseSampleCtr = false;
	m_bExcludeTriggers = false;
	m_nRequestedEEGChannelCnt = 0;
	m_nRequestedAuxChannelCnt = 0;
	m_nDevNo = 0;
	m_nSampleSize = 0;
	m_fLastTriggerVal = -1.0;
	m_nConnectedDeviceCnt = -1;
}

LibTalker::~LibTalker()
{
	Close();

}

void LibTalker::Error(const std::string& sError, int nErrorNum)
{
	std::string sErrorFromAmp;
	std::string sFullError = sError;
	switch (nErrorNum)
	{
	case -1:
		sErrorFromAmp = "AMP_ERR_FAIL";
		break;
		// TODO:
	case -2:
		sErrorFromAmp = "AMP_ERR_PARAM";
		break;
	case -3:
		sErrorFromAmp = "AMP_ERR_VERSION";
		break;
	case -4:
		sErrorFromAmp = "AMP_ERR_MEMORY";
		break;
	case -5:
		sErrorFromAmp = "AMP_ERR_BUSY";
		break;
	case -6:
		sErrorFromAmp = "AMP_ERR_NODEVICE";
		break;
	case -7:
		sErrorFromAmp = "AMP_ERR_NOSUPPORT";
		break;
	case -8:
		sErrorFromAmp = "AMP_ERR_EXCEPTION";
		break;
	case -9:
		sErrorFromAmp = "AMP_ERR_FWVERSION";
		break;
	case -10:
		sErrorFromAmp = "AMP_ERR_TIMEOUT";
		break;
	case -11:
		sErrorFromAmp = "AMP_ERR_BUFFERSIZE";
		break;

	case -101:
		sErrorFromAmp = "IF_ERR_FAIL";
		break;
	case -102:
		sErrorFromAmp = "IF_ERR_BT_SERVICE";
		break;
	case -103:
		sErrorFromAmp = "IF_ERR_MEMORY";
		break;
	case -104:
		sErrorFromAmp = "IF_ERR_NODEVICE";
		break;
	case -105:
		sErrorFromAmp = "IF_ERR_CONNECT";
		break;
	case -106:
		sErrorFromAmp = "IF_ERR_DISCONNECTED";
		break;
	case -107:
		sErrorFromAmp = "IF_ERR_TIMEOUT";
		break;
	case -108:
		sErrorFromAmp = "IF_ERR_ALREADYOPEN";
		break;
	case -109:
		sErrorFromAmp = "IF_ERR_PARAMETER";
		break;
	case -110:
		sErrorFromAmp = "IF_ERR_ATCOMMAND";
		break;

	case -200:
		sErrorFromAmp = "DEVICE_ERR_BASE";
		break;
	case -201:
		sErrorFromAmp = "DEVICE_ERR_FAIL";
		break;
	case -202:
		sErrorFromAmp = "DEVICE_ERR_PARAM";
		break;
	case -203:
		sErrorFromAmp = "DEVICE_ERR_VERSION";
		break;
	case -204:
		sErrorFromAmp = "DEVICE_ERR_MEMORY";
		break;
	case -205:
		sErrorFromAmp = "DEVICE_ERR_BUSY";
		break;
	case -206:
		sErrorFromAmp = "DEVICE_ERR_SDWRITE";
		break;
	case -207:
		sErrorFromAmp = "DEVICE_ERR_SDREAD";
		break;
	case -208:
		sErrorFromAmp = "DEVICE_ERR_NOSD";
		break;
	}
	sFullError.append(sErrorFromAmp);
	throw std::runtime_error(sFullError);
}

void LibTalker::enumerate(std::vector<std::pair<std::string, int>>& ampData) {

	int nRes;
	char HWI[20];


	if (!ampData.empty()) {
		throw std::runtime_error("Input ampData vector isn't empty");
		return;
	}

	strcpy_s(HWI, "USB");

	nRes = ampEnumerateDevices(HWI, sizeof(HWI), "actiCHamp", 0);

	if (nRes <= 0)
	{
		m_nConnectedDeviceCnt = -1;
		Error("Enumeration error", nRes);
	}
	else
	{
		m_nConnectedDeviceCnt = nRes;
		for (int i = 0; i < nRes; i++)
		{
			int nResult;
			HANDLE handle = NULL;

			nResult = ampOpenDevice(i, &handle);
			if (nResult != AMP_OK)
				Error("Enumeration error openning device: ", nResult);


			char sVar[20];
			nResult = ampGetProperty(handle, PG_DEVICE, i, DPROP_CHR_SerialNumber, sVar, sizeof(sVar));
			if (nResult != AMP_OK)
				Error("Enumeration error getting device serial number: ", nResult);

			else
			{
				//int32_t nAvailableModules;
				int32_t nAvailableChannels;
				nResult = ampGetProperty(handle, PG_DEVICE, 0, DPROP_I32_AvailableChannels, &nAvailableChannels, sizeof(nAvailableChannels));
				if (nResult != AMP_OK)
					Error("Enumeration error getting available channel count: ", nResult);
				/*int32_t nVar;
				char sModName[100];
				int nTotalAvailableChannels = 0;
				for (int j = 0; j < nAvailableModules; j++)
				{
					nResult = ampGetProperty(handle, PG_MODULE, j, MPROP_I32_UseableChannels, &nVar, sizeof(nVar));
					if (nResult != AMP_OK) 
						Error("Enumeration error getting device channel: ", nResult);
				}*/
				int nEEGChannels = 0;
				t_ChannelType ct;
				for (int i = 0; i < nAvailableChannels; i++)
				{
					nResult = ampGetProperty(handle, PG_CHANNEL, i, CPROP_I32_Type, &ct, sizeof(ct));
					if (nResult != AMP_OK)
						Error("Enumeration error scanning channel types: ", nResult);
					else
						if (ct == CT_EEG)
							nEEGChannels++;
				}
				ampData.push_back(std::make_pair(std::string(sVar), nEEGChannels));
			}
			nResult = ampCloseDevice(handle);
			if (nResult != AMP_OK) 
				Error("Enumeration error closing device: ", nResult);
		}
	}
}


void LibTalker::Connect(const std::string& sSerialNumber)
{

	char hwi[20];
	strcpy_s(hwi, "USB");
	std::string sHWDeviceAddress = "";
	int res;
	Close();
	if (m_nConnectedDeviceCnt == -1)
	{
		res = ampEnumerateDevices(hwi, sizeof(hwi), (const char*)sHWDeviceAddress.data(), 0);
		if (res < AMP_OK)
		{
			Error("Connection error enumerating devices: ", res);
			return;
		}
		m_nConnectedDeviceCnt = res;
	}

	char pcSerialNumber[100];
	m_Handle = NULL;
	HANDLE handle;
	for (int i = 0; i < m_nConnectedDeviceCnt; i++)
	{
		res = ampOpenDevice(i, &handle);
		if (res != AMP_OK)
		{
			Error("Connection error openning device: ", res);
			return;
		}
		res = ampGetProperty(handle, PG_DEVICE, i, DPROP_CHR_SerialNumber, pcSerialNumber, sizeof(pcSerialNumber));
		if (res != AMP_OK)
		{
			Error("Connection error getting serial number: ", res);
			return;
		}
		if (sSerialNumber.compare(pcSerialNumber) == 0)
		{
			m_nDevNo = i;
			m_Handle = handle;
			break;
		}
		res = ampCloseDevice(handle);
		
	}
	if (m_Handle == NULL)
		throw std::exception("Could not connect to specified amplifier. Please recheck connections and re-scan for devices.");

	res = ampGetProperty(m_Handle, PG_DEVICE, 0, DPROP_I32_AvailableChannels, &m_nAvailableChannelCnt, sizeof(m_nAvailableChannelCnt));
	if (res != AMP_OK)
	{
		Error("Connection error getting available channels: ", res);
		return;
	}
	int32_t nChannelType;
	m_nEEGChannelCnt = 0;
	std::string sErr;
	for (int i = 0; i < m_nAvailableChannelCnt; i++)
	{
		res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_I32_Type, &nChannelType, sizeof(nChannelType));
		if (res != AMP_OK)
		{
			sErr = "Connection error getting channel type for channel " + std::to_string(i);
			Error(sErr, res);
			return;
		}
		if (nChannelType == CT_EEG)
			m_nEEGChannelCnt++;
		if (nChannelType == CT_TRG)
			m_nTriggerIdx = i;
	}
}

void LibTalker::EnableChannels()
{
	int res;
	int32_t nVal, nChannType;
	m_nEnabledChannelCnt = 0;

	if (m_nRequestedEEGChannelCnt > m_nAvailableChannelCnt)
		throw std::exception(std::string("Requested Number of EEG channels (" 
			+std::to_string(m_nRequestedEEGChannelCnt) 
			+ ") is greater than the number of channels (" 
			+ std::to_string(m_nAvailableChannelCnt) 
			+ "available on this device.").c_str());

	char pcFunction[100];
	nVal = 0;
	for (int i = 0; i < m_nAvailableChannelCnt; i++)
		res = ampSetProperty(m_Handle, PG_CHANNEL, i, CPROP_B32_RecordingEnabled, &nVal, sizeof(nVal));

	for (int i = 0; i < m_nAvailableChannelCnt; i++)
	{
		res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_I32_Type, &nChannType, sizeof(nChannType));

		if (nChannType == CT_EEG && i < m_nRequestedEEGChannelCnt)
		{
			nVal = 1;
			res = ampSetProperty(m_Handle, PG_CHANNEL, i, CPROP_B32_RecordingEnabled, &nVal, sizeof(nVal));
			m_nEnabledChannelCnt++;
		}
		if (nChannType == CT_AUX && i < m_nRequestedAuxChannelCnt + m_nEEGChannelCnt)
		{
			nChannType = 1;
			res = ampSetProperty(m_Handle, PG_CHANNEL, i, CPROP_B32_RecordingEnabled, &nVal, sizeof(nVal));
			m_nEnabledChannelCnt++;
		}
		if (nChannType == CT_TRG && m_bUseTriggers)
		{
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_CHR_Function, &pcFunction, sizeof(pcFunction));
			if (strcmp(pcFunction, "Trigger Input")==0)
			{
				nVal = 1;
				res = ampSetProperty(m_Handle, PG_CHANNEL, i, CPROP_B32_RecordingEnabled, &nVal, sizeof(nVal));
				m_nEnabledChannelCnt++;
			}
		}
	}
}

void LibTalker::Setup()
{
	int res;
	int32_t nDataType;
	float fResolution;
	int32_t nChannelType;
	float fGain;
	int cnt = 0;
	m_nSampleSize = 0;
	int nEnabled;
	char pcSerialNumber[100];
	char pcFunction[100];
	char pcUnit[100];
	t_RecordingMode rmNormal = RM_NORMAL;
	res = ampSetProperty(m_Handle, PG_DEVICE, 0, DPROP_I32_RecordingMode, &rmNormal, sizeof(rmNormal));
	float fVal = m_fBaseSamplingRate;
	int32_t nUseFDA = (m_bUseFDA) ? 1 : 0;
	res = ampSetProperty(m_Handle, PG_DEVICE, 0, DPROP_B32_FastDataAccess, &nUseFDA, sizeof(nUseFDA));
	if (res != AMP_OK)
		Error("Setup error setting FDA option: ", res);
	res = ampSetProperty(m_Handle, PG_DEVICE, 0, DPROP_F32_BaseSampleRate, &fVal, sizeof(fVal));
	if (res != AMP_OK)
		Error("Setup error setting base sampling rate: ", res);
	fVal = m_fSubSampleDivisor;
	res = ampSetProperty(m_Handle, PG_DEVICE, 0, DPROP_F32_SubSampleDivisor, &fVal, sizeof(fVal));
	if (res != AMP_OK)
		Error("Setup error setting subsample divisor: ", res);
	EnableChannels();
	if (res != AMP_OK)
		Error("Setup error setting recording mode: ", res);

	res = ampStartAcquisition(m_Handle);
	if (res != AMP_OK)
		Error("Setup error starting acquistion: ", res);
	
	res = ampGetProperty(m_Handle, PG_DEVICE, 0, DPROP_CHR_SerialNumber, &pcSerialNumber, sizeof(pcSerialNumber));
	m_sSerialNumber = std::string(pcSerialNumber);

	for (int i = 0; i < m_nAvailableChannelCnt; i++) {
		
		res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_B32_RecordingEnabled, &nEnabled, sizeof(nEnabled));

		if (nEnabled) {
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_I32_DataType, &nDataType, sizeof(nDataType));
			m_ChannelInfoArray[cnt].nDataType = nDataType;
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_F32_Resolution, &fResolution, sizeof(fResolution));
			m_ChannelInfoArray[cnt].fResolution = fResolution;
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_I32_Type, &nChannelType, sizeof(nChannelType));
			m_ChannelInfoArray[cnt].nChannelType = nChannelType;
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_F32_Gain, &fGain, sizeof(fGain));
			m_ChannelInfoArray[cnt].fGain = fGain;
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_CHR_Function, &pcFunction, sizeof(pcFunction));
			m_ChannelInfoArray[cnt].sFunction = std::string(pcFunction);
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_CHR_Unit, &pcUnit, sizeof(pcUnit));
			m_ChannelInfoArray[cnt].sUnit = std::string(pcUnit);
			cnt++;
			switch (nDataType)
			{
			case DT_INT16:
			case DT_UINT16:
			{
				m_nSampleSize += 2;
			}
			break;
			case DT_INT32:
			case DT_UINT32:
			case DT_FLOAT32:
			{
				m_nSampleSize += 4;

			}
			break;
			case DT_INT64:
			case DT_UINT64:
			case DT_FLOAT64:
			{
				m_nSampleSize += 8;
			}
			break;
			default:
				break;
			}
		}

	}
	// add the sample counter size
	m_nSampleSize += 8;

	if (res != AMP_OK)
		Error("Setup error stopping acquistion: ", res);
}

void LibTalker::Close(void) {

	int res;
	if (m_Handle != NULL)
	{
		res = ampCloseDevice(m_Handle);
		m_Handle = NULL;
	}

}

void LibTalker::StopAcquisition(void) 
{
	int res = ampStopAcquisition(m_Handle);
}

int64_t LibTalker::PullAmpData(BYTE* buffer, int nBufferSize, std::vector<float>& vfOutDataMultiplexed, std::vector<int>& vnTriggers)
{
	int64_t nSamplesRead = ampGetData(m_Handle, buffer, nBufferSize, 0);
	
	if (nSamplesRead == 0)return 0;
	if (nSamplesRead < 0)
		Error("Error pulling samples from device: ", nSamplesRead);

	int nOffset;
	std::vector<float> vfTmpData;
	int64_t nSampleCnt = nSamplesRead / m_nSampleSize;
	float fSample;
	float fCounterVal;
	float fTriggerVal;
	bool bHasFullTrigger;
	int nTriggerBitCnt;
	for (int s = 0; s < nSampleCnt; s++)
	{
		if (m_bUseSampleCtr)
			fCounterVal = (float)*(int64_t*)&buffer[s * m_nSampleSize];
		nOffset = 8;

		for (int i = 0; i < m_nEnabledChannelCnt; i++)
		{
			switch (m_ChannelInfoArray[i].nDataType)
			{
			case DT_INT16:
			{
				int16_t tmp = *(int16_t*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)tmp * m_ChannelInfoArray[i].fResolution;
				nOffset += 2;
				break;
			}
			case DT_UINT16:
			{
				uint16_t tmp = *(uint16_t*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)tmp * m_ChannelInfoArray[i].fResolution;
				nOffset += 2;
				break;
			}
			case DT_INT32:
			{
				int32_t tmp = *(int32_t*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)(tmp)*m_ChannelInfoArray[i].fResolution;
				nOffset += 4;
				break;
			}
			case DT_UINT32:
			{
				uint32_t tmp = *(uint32_t*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)tmp * m_ChannelInfoArray[i].fResolution;
				nOffset += 4;
				break;
			}
			case DT_FLOAT32:
			{
				float tmp = *(float*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)tmp * m_ChannelInfoArray[i].fResolution;
				nOffset += 4;
				break;
			}
			case DT_INT64:
			{
				int64_t tmp = *(int64_t*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)tmp * m_ChannelInfoArray[i].fResolution;
				nOffset += 8;
				break;
			}
			case DT_UINT64:
			{
				uint64_t tmp = *(uint64_t*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)tmp * m_ChannelInfoArray[i].fResolution;
				nOffset += 8;
				break;
			}
			case DT_FLOAT64:
			{

				float tmp = *(float*)&buffer[s * m_nSampleSize + nOffset];
				fSample = (float)tmp * m_ChannelInfoArray[i].fResolution;
				nOffset += 8;
				break;
			}
			default:
				break;

			}

			if (m_ChannelInfoArray[i].nChannelType == CT_TRG)
			{
				fTriggerVal = ((fSample == m_fLastTriggerVal) ? -1.0 : (float)fSample);
				if (!m_bExcludeTriggers)
					vfOutDataMultiplexed.push_back(fTriggerVal);
				vnTriggers.push_back((UINT32)fSample);
				m_fLastTriggerVal = fSample;
			}
			else
				vfOutDataMultiplexed.push_back(fSample);

		}
		if (m_bUseSampleCtr)
			vfOutDataMultiplexed.push_back(fCounterVal);
	}

	return nSampleCnt;
}

bool LibTalker::CheckFDA(void)
{
	int32_t nFDA = 0;
	int res = 0;
	res = ampGetProperty(m_Handle, PG_DEVICE, 0, DPROP_B32_FastDataAccess, &nFDA, sizeof(nFDA));
	if (res != AMP_OK)
		Error("Setup error getting FDA option: ", res);
	return (nFDA == 1) ? true : false;
	
}

