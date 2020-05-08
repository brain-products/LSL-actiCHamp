#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <thread>
#include <string>
#include <vector>

// LSL API
//#define LSL_DEBUG_BINDINGS
#include "lsl_cpp.h"

// BrainAmp API
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinIoCtl.h>

#include "LibTalker.h"


struct t_AppVersion 
{
    int32_t Major;
    int32_t Minor;
    int32_t Bugfix;
};

struct t_AmpConfiguration
{
    std::string m_sSerialNumber;
    bool m_bUseActiveShield;
    bool m_bFDA;
    bool m_bUseSampleCtr;
    int m_nBaseSamplingRate;
    int m_nSubSampleDivisor;
    int m_nChunkSize;
    std::vector<std::string> m_psEegChannelLabels;
    std::vector<std::string> m_psAuxChannelLabels;
    bool m_bUnsampledMarkers;
    bool m_bSampledMarkersEEG;
};

namespace Ui 
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent, const char* config_file);
    ~MainWindow();
    
private slots:

    void LoadConfigDialog();
    void SaveConfigDialog();
    void Link();
    void closeEvent(QCloseEvent *ev);
    void SetSamplingRate();
	//void SetMinChunk();
    void VersionsDialog();
    void UpdateChannelLabels();
    void RefreshDevices();
    void UpdateChannelLabelsEEG(int);
    void UpdateChannelLabelsGUI(int);
    void ChooseDevice(int which);


private:

	void ReadThread(t_AmpConfiguration ampConfiguration);
    void AmpSetup(t_AmpConfiguration ampConfiguration);
    void LoadConfig(const QString &filename);
    void SaveConfig(const QString &filename);
    void ToggleGUIEnabled(bool bEnabled);

    t_AppVersion m_AppVersion;
    LibTalker m_LibTalker;
    std::vector<std::string> m_psAmpSns;
    std::vector<int> m_pnUsableChannelsByDevice;
    bool m_bUseActiveShield;
    int m_nSamplingRate;
    bool m_bOverrideAutoUpdate;
	bool m_bStop;										
    std::unique_ptr<std::thread>  m_ptReadThread;   	
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
