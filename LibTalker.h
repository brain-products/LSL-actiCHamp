
#pragma once

#include "Amplifier_LIB.h"
#include <vector>
#include <string>

class LibTalker
{
private:
	// data get from device (defined in construction)
	// these are initialized in the constructor
	int m_nConnectedDeviceCnt;
	HANDLE m_Handle;                 // device handle
	std::string m_sSerialNumber;     // serial number
	std::vector<std::pair<HANDLE, std::string>> m_vphsHandleSerialNumMap;
	int m_nAvailableChannelCnt;        
	int m_nEEGChannelCnt;
	int m_nAvailableModules;   
	int  m_nRequestedEEGChannelCnt;
	int m_nTriggerIdx;
	int m_nAuxChannelCnt;
	int m_nRequestedAuxChannelCnt;
	bool m_bUseFDA;
	bool m_bUseSim;
	bool m_bUseActiveShield;
	bool m_bUseTriggers;
	bool m_bUseSampleCtr;
	bool m_bExcludeTriggers;
	float m_fLastTriggerVal;
	// these are set later on by the user
	float m_fBaseSamplingRate;
	float m_fSubSampleDivisor;
	int m_nRecordingMode;               // defaults to record mode

	// set during configuration
	typedef struct _channelInfo {
		int nDataType;
		float fResolution;
		int nChannelType;
		float fGain;
		std::string sFunction;
		std::string sUnit;
	}t_channelInfo;
	// int dataTypeArray[100];
	t_channelInfo m_ChannelInfoArray[500];
	int m_nSampleSize;
	void EnableChannels();
	int m_nEnabledChannelCnt;
	bool m_bIsClosed;
	int m_nDevNo;

public:

	LibTalker::LibTalker();
	~LibTalker();
	void Error(const std::string& sError, int nErrorNum);
	void Connect(const std::string& sSerialNumber);
	void enumerate(std::vector<std::pair<std::string, int>>& ampData);
	void Setup();
	void Close();
	void StopAcquisition(void);
	bool CheckFDA(void); // for debugging
	//int64_t PullAmpData(BYTE* buffer, int nBufferSize, std::vector<float>& vfDataMultiplexed);
	int64_t PullAmpData(BYTE* buffer, int nBufferSize, std::vector<float>& vfDataMultiplexed, std::vector<int>& vnTriggers);

	// public data access methods 	
	inline float             getBaseSamplingRate(void) { return m_fBaseSamplingRate; }
	inline float             getSubSampleDivisor(void) { return m_fSubSampleDivisor; }
	inline void              setBaseSamplingRate(float fVal) { m_fBaseSamplingRate = fVal; }
	inline void              setSubSampleDivisor(float fVal) { m_fSubSampleDivisor = fVal; }
	inline HANDLE            getHandle(void) { if (m_Handle == NULL)return NULL; else return m_Handle; }
	inline std::string&      getSerialNumber(void) { return m_sSerialNumber; }
	inline void              setRequestedEEGChannelCnt(int nChannelCnt) { m_nRequestedEEGChannelCnt = nChannelCnt; }
	inline void              setRequestedAuxChannelCnt(int nChannelCnt) { m_nRequestedAuxChannelCnt = nChannelCnt; }
	inline void              setUseFDA(bool bUseFDA) { m_bUseFDA = bUseFDA; }
	inline void              setUseActiveShield(bool bUseActiveShield) { m_bUseActiveShield = bUseActiveShield; }
	inline int               getAvailableChannelCnt(void) { return m_nAvailableChannelCnt; }
	inline int				 getEEGChannelCnt(void) { return m_nEEGChannelCnt; }
	inline int               getAuxChannelCnt(void) { return m_nAuxChannelCnt; }
	inline int               getRecordingMode(void) { return m_nRecordingMode; }
	inline int               getEnabledChannelCnt(void) { return m_nEnabledChannelCnt; }
	inline int               getSampleSize(void) { return m_nSampleSize; }
	inline bool              getUseSim(void) { return m_bUseSim; }
	inline void              setUseTriggers(bool bUseTriggers) { m_bUseTriggers = bUseTriggers; }
	inline void              setExcludeTriggersFromOutput(bool bExclude) { m_bExcludeTriggers = bExclude; }
	inline bool				 isClosed(void) { return m_bIsClosed; }
	inline void              setUseSim(bool bUseSim) { m_bUseSim = bUseSim; }
	inline void              setUseSampleCtr(bool bUseSampleCtr) { m_bUseSampleCtr = bUseSampleCtr; }

};

