#include "LibTalker.h"
#include <iostream>

LibTalker::LibTalker()
{
	m_bUseAux = false;
	m_bUseFDA = false;
	m_bUseSim = false;
	m_bUseActiveShield = false;
	m_bUseTriggers = false;
	m_bUseSampleCtr = false;
	m_bExcludeTriggers = false;
	m_nRequestedEEGChannelCnt = 0;
	m_nDevNo = 0;
	m_nSampleSize = 0;
	m_fLastTriggerVal = -1.0;
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
		break;
	}
	sFullError.append(sErrorFromAmp);
	throw std::runtime_error(sFullError);
}



void LibTalker::Connect(int nIdx)
{

	m_nDevNo = nIdx;
	char hwi[20];
	strcpy_s(hwi, "USB");
	std::string sHWDeviceAddress = "";
	int res;
	Close();
	res = ampEnumerateDevices(hwi, sizeof(hwi), (const char*)sHWDeviceAddress.data(), 0);
	if (res < AMP_OK)
	{
		Error("Error enumerating devices: ", res);
		return;
	}
	m_Handle = NULL;
	res = ampOpenDevice(nIdx, &m_Handle);
	if (res != AMP_OK)
	{
		Error("Error opening device: ", res);
		return;
	}
	res = ampGetProperty(m_Handle, PG_DEVICE, 0, DPROP_I32_AvailableChannels, &m_nAvailableChannelCnt, sizeof(m_nAvailableChannelCnt));
	if (res != AMP_OK)
	{
		Error("Error getting available channels:", res);
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
			sErr = "Error getting channel type for channel " + std::to_string(i);
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
		if (nChannType == CT_AUX && m_bUseAux)
		{
			nChannType = 1;
			res = ampSetProperty(m_Handle, PG_CHANNEL, i, CPROP_B32_RecordingEnabled, &nVal, sizeof(nVal));
			m_nEnabledChannelCnt++;
		}
		if (nChannType == CT_TRG && m_bUseTriggers)
		{
			// only record input triggers?
			//if (!bISecondsTrigger)
			//{
			res = ampGetProperty(m_Handle, PG_CHANNEL, i, CPROP_CHR_Function, &pcFunction, sizeof(pcFunction));
			if (strcmp(pcFunction, "Trigger Input")==0)
			{
				nVal = 1;
				res = ampSetProperty(m_Handle, PG_CHANNEL, i, CPROP_B32_RecordingEnabled, &nVal, sizeof(nVal));
				m_nEnabledChannelCnt++;
			}
				//bIsSecondTrigger = true;
			//}
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
	res = ampSetProperty(m_Handle, PG_DEVICE, 0, DPROP_F32_BaseSampleRate, &fVal, sizeof(fVal));
	if (res != AMP_OK)
		Error("Error setting base sampling rate during setup: ", res);
	fVal = m_fSubSampleDivisor;
	res = ampSetProperty(m_Handle, PG_DEVICE, 0, DPROP_F32_SubSampleDivisor, &fVal, sizeof(fVal));
	if (res != AMP_OK)
		Error("Error setting subsample divisor during setup: ", res);
	EnableChannels();
	if (res != AMP_OK)
		Error("Error setting recording mode: ", res);

	res = ampStartAcquisition(m_Handle);
	if (res != AMP_OK)
		Error("Error starting acquistion during setup: ", res);
	
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
		Error("Error stopping acquistion during setup: ", res);
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
		//vfTmpData.resize(m_nEnabledChannelCnt);

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



