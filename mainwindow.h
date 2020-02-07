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


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent, const std::string &config_file);
    ~MainWindow();
    
private slots:
    // config file dialog ops (from main menu)
    void load_config_dialog();
    void save_config_dialog();

    // start the ActiChamp connection
    void link();

    // close event (potentially disabled)
    void closeEvent(QCloseEvent *ev);

    void setSamplingRate();

	// set the min chunk size according sampling rate
	void setMinChunk();

private:

    // background data reader thread
	void read_thread(const std::vector<std::string>& channelLabels);

    // raw config file IO
    void load_config(const std::string &filename);
    void save_config(const std::string &filename);

	bool m_bUnsampledMarkers;
	bool m_bSampledMarkersEEG;
    int m_nDeviceNumber;
    int m_nEEGChannelCount;
    int m_nChunkSize;
    int m_nBaseSamplingRate;
    int m_nSubSampleDivisor;
    int m_nSamplingRate;
    bool m_bUseAUX;
    bool m_bUseFDA;
    bool m_bUseActiveShield;
    bool m_bUseSampleCtr;

    struct t_AppVersion {
        int32_t Major;
        int32_t Minor;
    };
    t_AppVersion m_AppVersion;

    LibTalker m_LibTalker;
	
	bool m_bStop_;											// whether the reader thread is supposed to stop
    std::unique_ptr<std::thread>  m_ptReadThread;   	// our reader thread

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
