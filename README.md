# LSL-actiCHamp
LSL connector for the actiCHamp (plus) device from Brain Products.

To download, please click the 'Latest release' link to the right for the latest versions.

Please note that you may need to install the Microsoft C++ redistributable packages ([here](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)) in order to run the executables. Please note that for this app to function properly, you may need to download and install not only the package for Visual Studio 2015, 2017, and 2019, but also the Visual Studio 2012 (VC++ 11.0) redistributable package. Both packages can be found by following the link above.

# Getting Started
If you are new to using LSL, you might want to read the [Quick Start guide](https://labstreaminglayer.readthedocs.io/info/getting_started.html) on the LabStreamingLayer main repository. 

For measuring impedances, ensuring good signal quality, and troubleshooting basic amplifier functionality, it is always recommended to use [BrainVision Recorder](https://www.brainproducts.com/downloads.php?kid=2) and to consult the amplifier manual.

You can also find a useful (free) LSL viewer on the Brian Products website: [BrainVision LSL Viewer](https://www.brainproducts.com/downloads.php?kid=40&tab=3).

# Usage

Make sure that you have correctly installed the drivers for your amplifier, and that the amplifier is connected to the computer. Also make sure that the battery is fully charged and connected to the amplifier. It is always a good idea to be familiar with the amplifier manual before operation.

  * Start the actiCHamp Connector app. You should see a window like the following.
> ![actiCHamp.png](actiCHamp.png)

## Device Settings

### Device Serial Number

If you know the serial number of the amplifier you want to connect with, you may enter it manually in the 'Device Serial Number' field. Otherwise, you can scan for amplifiers using the 'Scan' button at the bottom of the GUI. After scanning, the'Available Devices' combo box will be populated with all available devices. The 'Device Serial Number' field will automatically be filled with the number of the first device found. Whichever device is referred to here will be the device that is connected with at the 'Link' action. 

### Number of EEG Channels

You can adjust the number of EEG channels that the device enables during Link action. Note that if the 'Overwrite on Channel Change' box is checked, changing these values will change the labels in the Channel Labels text box on the left side of the GUI. See the section on Channel Labels below for more information. The maximum number of available channels depends on the number of modules on the amplifier.

### Number of Aux Channels

The actiCHamp/actiCHamp Plus amplifiers have 8 available auxilary channels. Any number of these can be enabled (starting from channel 1 and moving up).

### Include Sample Ctr

Checking this box will add an additional channel to the EEG/AUX channel stream that counts the incoming samples coming from the device.

### Fast Data Access

The actiCHamp/actiCHamp Plus device is capable of very low-latency data delivery. However, in default mode (Fast Data Access unchecked) the driver will periodically probe the USB connection to ensure the cable hasn't been disconnected during data acquisition. If this safety check is disabled, it greatly reduces the amount of time between the moment a signal is acquired by the sensors and the time the digitally sample corresponding to the value at the sensor is available to the Connector app.

This is useful for low-latency applications such as closed-loop and BCI. It is recommended to use a relatively high sampling rate, e.g. 10 or 20kHz or else the length of the data buffer will be so large as to render this speedup negligible.

### Chunk Size

For most EEG experiments you can ignore the Chunk Size setting, but if you are developing a latency-critical real-time application (e.g., a P300 speller BCI), you can lower this setting to reduce the latency of your system. 

### Base Sampling Rate and Sub Sample Divisor

The actiCHamp/actiCHamp Plus has 3 native sampling frequencies: 100, 50, and 10kHz. The device driver can sub-sample in an extremely optimized manner by a factor of 1, 2, 5, 10, 20, 50, or 100. These values are the 'Base Sampling Rate' and 'Sub Sample Divisor' respectively. The Base Sampling Rate divided by the Sub Sample Divisor yields the the Nominal Sampling Rate.

### Nominal Sampling Rate

See above. Note that this is a read-only field. It will update when the Base Sampling Rate or Sub Sample Divisor is changed.

## LSL Trigger Output Style

These check boxes determine the way that the app handles device triggers in LSL.

Note that it is possible to enable neither, one of, or both trigger output styles simultaneously. They are not mutually exclusive.

### Unsampled String Markers

If 'Unsampled String Markers' is checked, the app will create a Marker stream on 'Link' that will forward the triggers received at the 1-bit trigger input on the hardware device. This stream will have the name of 'LiveAmpSN-xxxxxx-xxxx-DeviceTrigger' and its sampling rate is 0, i.e. it is a Marker stream 

If the STE box is connected, there will be two Marker streams created. One for the 1-bit amplifier trigger and one for the STE box trigger input. The STE Marker stream will have the name 'LiveAmpSN-xxxxxx-xxxx-STETriggerIn'.

If the STE box is connected and the STE Out Mode is set to 'Sync' there will be a third Marker outlet that corresponds to the hardware triggers sent from the trigger out of the STE box. The name of this Marker stream will be 'LiveAmpSN-xxxxxx-xxxx-STESync'. Note that this is not recommended for 'Unsampled String Markers' as it creates a large number of string markers (one per sample) which is not the intended use of LSL Marker outlets.

### EEG Channel

If this box is checked, extra channels will be added to the EEG/Bipolar/AUX/ACC stream corresponding to the requested trigger outputs. Rather than unsampled markers, these channels will output -1 if no trigger is available, else the value corresponding to the triggers in; and, in the case of the STE box in 'Sync' mode, the sync stream presented at the DSub output of the STE box. 

### Further Information

Please see the file [explanation_of_trigger_marker_types.pdf]( https://github.com/brain-products/LSL-LiveAmp/blob/master/explanation_of_trigger_marker_types.pdf) for more details.

 ## Link

 When pressed, the Link button will attempt to connect to the device specified in the 'Device Serial Number' field. When complete, the button text will change to 'Unlink' and all other GUI widgets will be disabled. An error message will pop up if no devices are available.

 When the Connector is linked to the actiCHamp, LSL streams will be created according to the settings set in the GUI. Please note that settings can be saved in a configuration file (see below) for ease of repetition.

## Scan For Devices

When 'Scan' is pressed, the app will search for available LiveAmps. When 1 or more actiCHamps is found, the Serial Numbers of these devices will populate the 'Available Devices' combo box. The first device found will automatically be placed in the 'Device Serial Number' field. When devices are available in the 'Available Devices' box, the user may choose from the available devices in the combo box by interacting with the combo box itself. The chosen device will be forwarded to the 'Device Number' field. If the selected device is unavailable at 'Link', the app will notify the user with an error message.

The LiveAmp device has a simulator mode which can be activated with the 'Use Simulator' checkbox. This will create 

## Configuration file

The configuration settings can be saved to a .cfg file (see File / Save Configuration) and subsequently loaded from such a file (via File / Load Configuration). Importantly, the program can be started with a command-line argument of the form "actiCHamp.exe -c myconfig.cfg", which allows to load the config automatically at start-up. The recommended procedure to use the app in experiments is to make a shortcut on the experimenter's desktop which points to a previously saved configuration customized to the study being recorded to minimize the chance of operator error.

## Active Shielding

There is no GUI setting to activate the Active Shielding option on the actiCHamp Connector. However, for those who know how to use this option (please read the amplifier manual before using this option!) you may do so by editing a configuration file in a text editor and including the following lines under the 'Settings' section:

`activeShield=true`

It is very important that you understand the consequences of activating this option as when used incorrectly, it can result in very unexpected results. Please consult the manual.

## Channel Labels

If the `Overwrite Channel Labels` box is checked, the channel label field will automatically update when you change the number of channels. The chosen channel label is simply an integer number corresponding to the channel number. You can change the channel labels by editing the `Channel Labels` text field directly.

The latest version of the actiCHamp Connector uses [INI](https://en.wikipedia.org/wiki/INI_file) style configuration files (see above) to store preferred settings between sessions. Easier and less error prone than adjusting channel labels in the App's GUI, is to set the channel labels by editing a config file in a text editor and then loading the updated config file. To do so, in the chosen config file simply create a [section](https://en.wikipedia.org/wiki/INI_file#Sections) called `channels` then create a [key](https://en.wikipedia.org/wiki/INI_file#Keys_(properties)) called `labels` with the corresponding labels for each channel separated by commas. For example, a 32 channel 10-20 layout may look like this:

`[channels]`
`labels=Fp1, Fp2, F7, F3, Fz, F4, F8, FC5, FC1, FC2, FC6, T7, C3, Cz, C4, T8, TP9, CP5, CP1, CP2, CP6, TP10, P7, P3, Pz, P4, P8, PO9, O1, Oz, O2, PO10`

The LSL channel meta-data corresponds to the conventions of the XDF file format. These are described [here](https://github.com/sccn/xdf/wiki/EEG-Meta-Data).

## Loading Channel Label Files

Python users may automatically insert channel labels from a .bvef file into an LSL config file. To do so, please use the free utility [BVEF2lslconfig](https://github.com/brain-products/BVEF2lslconfig). You can find many electrode position files with channel labels for common cap configurations on the Brain Products website [here](https://www.brainproducts.com/downloads.php?kid=44). 
