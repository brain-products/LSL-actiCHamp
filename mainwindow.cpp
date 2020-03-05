/*#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h> */ 

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Amplifier_LIB.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <iostream>

const int m_pnBaseSamplingRates[] = {10000,50000,100000};
const int m_pnSubSampleDivisors[] = { 1,2,5,10,20,50,100 };

void Transpose(const std::vector<std::vector<double> > &in, std::vector<std::vector<double> > &out);

#define LIBVERSIONSTREAM(version) version.Major << "." << version.Minor << "." << version.Build << "." << version.Revision
#define LSLVERSIONSTREAM(version) (version/100) << "." << (version%100)
#define APPVERSIONSTREAM(version) version.Major << "." << version.Minor

MainWindow::MainWindow(QWidget *parent, const std::string &config_file): QMainWindow(parent),ui(new Ui::MainWindow)
{
	m_AppVersion.Major = 1;
	m_AppVersion.Minor = 12;
	//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
	//_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG ); 

	ui->setupUi(this);
	m_bUseActiveShield = false;
	// parse startup config file
	load_config(config_file);

	// make GUI connections
	QObject::connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
	QObject::connect(ui->linkButton, SIGNAL(clicked()), this, SLOT(link()));
	QObject::connect(ui->actionLoad_Configuration, SIGNAL(triggered()), this, SLOT(load_config_dialog()));
	QObject::connect(ui->actionSave_Configuration, SIGNAL(triggered()), this, SLOT(save_config_dialog()));
	QObject::connect(ui->baseSamplingRate, SIGNAL(currentIndexChanged(int)), this, SLOT(setSamplingRate()));
	QObject::connect(ui->subSampleDivisor, SIGNAL(currentIndexChanged(int)), this, SLOT(setSamplingRate()));
	QObject::connect(ui->actionVersions, SIGNAL(triggered()), this, SLOT(versionsDialog()));

	setSamplingRate();
}

void MainWindow::versionsDialog()
{
	t_VersionNumber libVersion;
	GetLibraryVersion(&libVersion);
	int32_t lslProtocolVersion = lsl::protocol_version();
	int32_t lslLibVersion = lsl::library_version();
	std::stringstream ss;
	ss << "Amplifier_LIB: " << LIBVERSIONSTREAM(libVersion) << "\n" <<
		"lsl protocol: " << LSLVERSIONSTREAM(lslProtocolVersion) << "\n" <<
		"liblsl: " << LSLVERSIONSTREAM(lslLibVersion) << "\n" <<
		"App: " << APPVERSIONSTREAM(m_AppVersion) << "_beta";
	QMessageBox::information(this, "Versions", ss.str().c_str(), QMessageBox::Ok);
}

void MainWindow::setSamplingRate()
{
	int nNum = m_pnBaseSamplingRates[ui->baseSamplingRate->currentIndex()];
	int nDenom = m_pnSubSampleDivisors[ui->subSampleDivisor->currentIndex()];
	std::string sSR = std::to_string(nNum / nDenom);
	ui->nominalSamplingRate->setText(sSR.c_str());
	m_nSamplingRate = nNum / nDenom;
	//setMinChunk();
}

void MainWindow::setMinChunk(){


	switch (m_nSamplingRate) {
	case 100:
		if (ui->chunkSize->value() < 100)ui->chunkSize->setValue(100);
		ui->chunkSize->setMinimum(100);
		break;
	case 125:
		if(ui->chunkSize->value()<80)ui->chunkSize->setValue(80);
		ui->chunkSize->setMinimum(80);
		break;
	case 250:
		if(ui->chunkSize->value()<40)ui->chunkSize->setValue(40);
		ui->chunkSize->setMinimum(40);
		break;
	case 500:
		if(ui->chunkSize->value()<20)ui->chunkSize->setValue(20);
		ui->chunkSize->setMinimum(20);
		break;
	case 1000:
		if(ui->chunkSize->value()<10)ui->chunkSize->setValue(10);
		ui->chunkSize->setMinimum(10);
		break;
	}
}

void MainWindow::load_config_dialog() {
	QString sel = QFileDialog::getOpenFileName(this,"Load Configuration File","","Configuration Files (*.cfg)");
	if (!sel.isEmpty())
		load_config(sel.toStdString());
}

void MainWindow::save_config_dialog() {
	QString sel = QFileDialog::getSaveFileName(this,"Save Configuration File","","Configuration Files (*.cfg)");
	if (!sel.isEmpty())
		save_config(sel.toStdString());
}

void MainWindow::closeEvent(QCloseEvent *ev) {
	if (m_ptReadThread)
		ev->ignore();
	/*else
		_CrtDumpMemoryLeaks();  */
}

int getBaseSamplingRateIndex(int nBaseSamplingRate)
{
	switch (nBaseSamplingRate)
	{
	case 10000:
		return 0;
	case 50000:
		return 1;
	case 100000:
		return 2;
	}
	return 0;
}

int getSubSampleDivisorIndex(int nSubSampleDivisor)
{
	switch (nSubSampleDivisor)
	{
	case 1:
		return 0;
	case 2:
		return 1;
	case 5:
		return 2;
	case 10:
		return 3;
	case 20:
		return 4;
	case 50:
		return 5;
	case 100:
		return 6;
	}
	return 0;
}


void MainWindow::load_config(const std::string &filename) {
	using boost::property_tree::ptree;
	ptree pt;

	// parse file
	try {
		read_xml(filename, pt);
	} catch(std::exception &e) {
		QMessageBox::information(this,"Error",(std::string("Cannot read config file: ")+= e.what()).c_str(),QMessageBox::Ok);
		return;
	}

	// get config values
	try {

		ui->deviceNumber->setValue(pt.get<int>("settings.devicenumber",0));
		ui->channelCount->setValue(pt.get<int>("settings.channelcount",32));
		ui->chunkSize->setValue(pt.get<int>("settings.chunksize",10));
		// TODO: make this a switch statement based on actual values
		m_nBaseSamplingRate = pt.get<int>("settings.basesamplingrate", 10000);
		ui->baseSamplingRate->setCurrentIndex(getBaseSamplingRateIndex(m_nBaseSamplingRate));
		m_nSubSampleDivisor = pt.get<int>("settings.subsampledivisor", 20);
		ui->nominalSamplingRate->setText((std::to_string(m_nBaseSamplingRate / m_nSubSampleDivisor)).c_str());
		ui->subSampleDivisor->setCurrentIndex(getSubSampleDivisorIndex(m_nSubSampleDivisor));
		//setMinChunk();
		ui->useAUX->setCheckState(pt.get<bool>("settings.useaux",false) ? Qt::Checked : Qt::Unchecked);
		ui->fastDataAccess->setCheckState(pt.get<bool>("settings.usefda", false) ? Qt::Checked : Qt::Unchecked);
		ui->useSampleCtr->setCheckState(pt.get<bool>("settings.usesamplectr", false) ? Qt::Checked : Qt::Unchecked);
		m_bUseActiveShield = pt.get<bool>("settings.activeshield",false);
		ui->sampledMarkersEEG->setCheckState(pt.get<bool>("settings.sampledmarkersEEG",false) ? Qt::Checked : Qt::Unchecked);
		ui->unsampledMarkers->setCheckState(pt.get<bool>("settings.unsampledmarkers",false) ? Qt::Checked : Qt::Unchecked);	
		ui->channelLabels->clear();
		BOOST_FOREACH(ptree::value_type &v, pt.get_child("channels.labels"))
			ui->channelLabels->appendPlainText(v.second.data().c_str());
		setSamplingRate();
	} catch(std::exception &) {
		QMessageBox::information(this,"Error in Config File","Could not read out config parameters.",QMessageBox::Ok);
		return;
	}
}

void MainWindow::save_config(const std::string &filename) {
	using boost::property_tree::ptree;
	ptree pt;

	// transfer UI content into property tree
	try {
		pt.put("settings.devicenumber",ui->deviceNumber->value());
		pt.put("settings.channelcount",ui->channelCount->value());
		pt.put("settings.chunksize",ui->chunkSize->value());
		pt.put("settings.basesamplingrate", m_pnBaseSamplingRates[ui->baseSamplingRate->currentIndex()]);
		pt.put("settings.subsampledivisor", m_pnSubSampleDivisors[ui->subSampleDivisor->currentIndex()]);
		pt.put("settings.useaux",ui->useAUX->checkState()==Qt::Checked);
		pt.put("settings.usefda", ui->fastDataAccess->checkState() == Qt::Checked);
		pt.put("settings.usesamplectr", ui->useSampleCtr->checkState() == Qt::Checked);
		pt.put("settings.unsampledmarkers",ui->unsampledMarkers->checkState()==Qt::Checked);
		pt.put("settings.sampledmarkersEEG",ui->sampledMarkersEEG->checkState()==Qt::Checked);
		std::vector<std::string> channelLabels;
		boost::algorithm::split(channelLabels,ui->channelLabels->toPlainText().toStdString(),boost::algorithm::is_any_of("\n"));
		BOOST_FOREACH(std::string &v, channelLabels)
			pt.add("channels.labels.label", v);
	} catch(std::exception &e) {
		QMessageBox::critical(this,"Error",(std::string("Could not prepare settings for saving: ")+=e.what()).c_str(),QMessageBox::Ok);
	}

	// write to disk
	try {
		write_xml(filename, pt);
	} catch(std::exception &e) {
		QMessageBox::critical(this,"Error",(std::string("Could not write to config file: ")+=e.what()).c_str(),QMessageBox::Ok);
	}
}


// start/stop the ActiChamp connection
void MainWindow::link() 
{
	bool bSuccess = true;
	if (m_ptReadThread) 
	{
		// === perform unlink action ===
		try {
			m_bStop_ = true;
			m_ptReadThread->join();
			m_ptReadThread.reset();
			//m_LibTalker.Close();
		} catch(std::exception &e) {
			QMessageBox::critical(this,"Error",(std::string("Could not stop the background processing: ")+=e.what()).c_str(),QMessageBox::Ok);
			return;
		}

		// indicate that we are now successfully unlinked
		ui->linkButton->setText("Link");
	} else {
		// === perform link action ===

		try {
			std::vector<std::string> channelLabels;
			boost::algorithm::split(channelLabels, ui->channelLabels->toPlainText().toStdString(), boost::algorithm::is_any_of("\n"));
			// get the UI parameters...
			m_nDeviceNumber = ui->deviceNumber->value();

			this->setWindowTitle(QString("Attempting to connect"));
			this->setCursor(Qt::WaitCursor);
			m_LibTalker.Connect(m_nDeviceNumber);

			m_nEEGChannelCount = ui->channelCount->value();
			int nEEGChannelCntFromAmp = m_LibTalker.getEEGChannelCnt();
			if (m_nEEGChannelCount > nEEGChannelCntFromAmp)
			{
				QMessageBox::warning(this, "Cannot proceed", "You have selected more channels than are available on this device");
				this->setCursor(Qt::ArrowCursor);
				return;
			}
			if (m_nEEGChannelCount != channelLabels.size())
			{
				QMessageBox::warning(this, "Cannot proceed", "EEG channel labels list length does not equal number of selected channels");
				this->setCursor(Qt::ArrowCursor);
				return;
			}

			m_LibTalker.setUseTriggers(ui->sampledMarkersEEG->isChecked() || ui->unsampledMarkers->isChecked());
			m_LibTalker.setExcludeTriggersFromOutput(!ui->sampledMarkersEEG->isChecked());
			m_LibTalker.setRequestedEEGChannelCnt(m_nEEGChannelCount);
			m_nBaseSamplingRate = m_pnBaseSamplingRates[ui->baseSamplingRate->currentIndex()];
			m_LibTalker.setBaseSamplingRate((float)m_nBaseSamplingRate);
			m_nSubSampleDivisor = m_pnSubSampleDivisors[ui->subSampleDivisor->currentIndex()];			
			m_LibTalker.setSubSampleDivisor((float)m_nSubSampleDivisor);
			m_nSamplingRate = m_nBaseSamplingRate / m_nSubSampleDivisor;
			m_bUseAUX = ui->useAUX->checkState() == Qt::Checked;
			m_LibTalker.setUseAux(m_bUseAUX);
			m_bUseFDA = ui->fastDataAccess->checkState() == Qt::Checked;
			m_LibTalker.setUseFDA(m_bUseFDA);
			m_bUseSampleCtr = ui->useSampleCtr->checkState() == Qt::Checked;
			m_LibTalker.setUseSampleCtr(m_bUseSampleCtr);
			m_bSampledMarkersEEG = ui->sampledMarkersEEG->checkState() == Qt::Checked;
			m_bUnsampledMarkers = ui->unsampledMarkers->checkState() == Qt::Checked;
			m_LibTalker.setUseActiveShield(false);			
			m_LibTalker.Setup();
			m_nChunkSize = ui->chunkSize->value();

			this->setCursor(Qt::ArrowCursor);
			this->setWindowTitle("actiCHamp Connector");
			// start reader thread
			m_bStop_ = false;
			m_ptReadThread.reset(new std::thread(&MainWindow::read_thread,this, channelLabels));
		}

		catch(std::exception &e) {

			// generate error message
			std::string msg = e.what();
			QMessageBox::critical(this,"Error",("Could not initialize the actiCHamp: " + msg).c_str(),QMessageBox::Ok);	
			this->setCursor(Qt::ArrowCursor);
			return;
		}

		// done, all successful
		ui->linkButton->setText("Unlink");
	}
}



// background data reader thread
void MainWindow::read_thread(const std::vector<std::string>& channelLabels) 
{
	//int res = SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	int nEnabledChannelCnt = m_nEEGChannelCount + ((m_bUseAUX) ? 8 : 0);
	int nExtraEEGChannelCnt = 0;
	if (m_bSampledMarkersEEG) nExtraEEGChannelCnt+=1;
	if (m_bUseSampleCtr) nExtraEEGChannelCnt += 1;

	try
	{
		// setup LSL
		lsl::stream_outlet* poutMarkerOutlet = NULL;
		std::cout << std::string("actiCHamp" + m_LibTalker.getSerialNumber());
		std::cout << "EEG";
		std::cout << nEnabledChannelCnt + nEnabledChannelCnt;
		std::cout << (double)m_nSamplingRate;
		std::cout << lsl::cf_float32;
		std::cout << m_LibTalker.getSerialNumber();
		lsl::stream_info infoData("actiCHamp-" + m_LibTalker.getSerialNumber(),
			"EEG",
			nEnabledChannelCnt + nExtraEEGChannelCnt,
			(double)m_nSamplingRate,
			lsl::cf_float32,
			m_LibTalker.getSerialNumber());


		lsl::xml_element channels = infoData.desc().append_child("channels");


		for (int i = 0; i < channelLabels.size(); i++)
		{
			channels.append_child("channel")
				.append_child_value("label", channelLabels[i].c_str())
				.append_child_value("type", "EEG")
				.append_child_value("unit", "microvolts");
		}
		if (m_bUseAUX)
			for (int i = 0; i < 8; i++)
			{
				channels.append_child("channel")
					.append_child_value("label", ("AUX_" + std::to_string(i + 1)).c_str())
					.append_child_value("type", "AUX")
					.append_child_value("unit", "microvolts");
			}
		if (m_bSampledMarkersEEG)
			channels.append_child("channel")
			.append_child_value("label", "Markers")
			.append_child_value("type", "Markers")
			.append_child_value("unit", "");

		if (m_bUseSampleCtr)
			channels.append_child("channel")
			.append_child_value("label", "SampleCounter")
			.append_child_value("type", "SampleCounter")
			.append_child_value("unit", "");

		infoData.desc().append_child("acquisition")
			.append_child_value("manufacturer", "Brain Products");

		t_VersionNumber libVersion;
		GetLibraryVersion(&libVersion);
		int32_t lslProtocolVersion = lsl::protocol_version();
		int32_t lslLibVersion = lsl::library_version();
		std::stringstream ssLib;
		ssLib << LIBVERSIONSTREAM(libVersion);
		std::stringstream ssProt;
		ssProt << LSLVERSIONSTREAM(lslProtocolVersion);
		std::stringstream ssLSL;
		ssLSL << LSLVERSIONSTREAM(lslLibVersion);
		std::stringstream ssApp;
		ssApp << APPVERSIONSTREAM(m_AppVersion);

		infoData.desc().append_child("versions")
			.append_child_value("Amplifier_LIB", ssLib.str())
			.append_child_value("lsl_protocol", ssProt.str())
			.append_child_value("liblsl", ssLSL.str())
			.append_child_value("App", ssApp.str());

		lsl::stream_outlet outData(infoData);

		lsl::stream_info infoMarkers("actiCHampMarkers-" + m_LibTalker.getSerialNumber(),
			"Markers",
			1,
			0,
			lsl::cf_string,
			m_LibTalker.getSerialNumber());

		if (m_bUnsampledMarkers)
		{
			infoMarkers.desc().append_child("channels")
				.append_child("channel")
				.append_child_value("label", "Markers")
				.append_child_value("type", "Markers")
				.append_child_value("unit", "");

			infoMarkers.desc().append_child("acquisition")
				.append_child_value("manufacturer", "Brain Products");

			infoMarkers.desc().append_child("versions")
				.append_child_value("Amplifier_LIB", ssLib.str() + "_beta")
				.append_child_value("lsl_protocol", ssProt.str())
				.append_child_value("liblsl", ssLSL.str())
				.append_child_value("App", ssApp.str());

			poutMarkerOutlet = new lsl::stream_outlet(infoMarkers);

		}

		// do data acquistion
		std::vector<float> vfDataBufferMultiplexed;
		std::vector<int> vnTriggers;
		int nMarker;
		int nPreviousMarker = -1;
		int32_t nBufferSize = m_nChunkSize * m_LibTalker.getSampleSize();
		BYTE* buffer = NULL;
		buffer = new BYTE[nBufferSize];
		int64_t nSampleCnt;
		double dNow;
		int nSamplesPulled = 0;
		dNow = lsl::local_clock();
		//vfDataBufferMultiplexed.clear();
		vfDataBufferMultiplexed.resize(m_nChunkSize * (nEnabledChannelCnt + nExtraEEGChannelCnt));
		//vnTriggers.clear();
		vnTriggers.resize(m_nChunkSize);
		while (!m_bStop_)
		{

			nSampleCnt = m_LibTalker.PullAmpData(buffer, nBufferSize, vfDataBufferMultiplexed, vnTriggers);
			if (nSampleCnt <= 0) {
				// CPU saver, this is ok even at higher sampling rates
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			nSamplesPulled += nSampleCnt;

			if (m_bUnsampledMarkers)
			{
				for (int s = 0; s < nSampleCnt; s++)
				{
					nMarker = (int)vnTriggers[s];
					if (nMarker != nPreviousMarker)
					{
						std::string sMarker = std::to_string(nMarker);
						poutMarkerOutlet->push_sample(&sMarker, dNow + (s + 1 - nSampleCnt) / m_nSamplingRate);
					}
					nPreviousMarker = nMarker;
				}
			}
			outData.push_chunk_multiplexed(vfDataBufferMultiplexed, dNow);
			vfDataBufferMultiplexed.clear();
			vnTriggers.clear();
			dNow = lsl::local_clock();
			nSamplesPulled = 0;
		}
		if (poutMarkerOutlet != NULL)
			delete(poutMarkerOutlet);
	}
	catch (std::exception & e)
	{
		std::string msg = e.what();
		//QMessageBox::critical(this, "Error", ("Error in acquisition loop: " + msg).c_str(), QMessageBox::Ok);
	}
	// exit gracefully
	//if(poutMarkerOutlet !=NULL)

}

MainWindow::~MainWindow() {
	delete ui;
}

