# NIDAQ_Monitor
Data acquiring software for NI-DAQ device using OpenFrameworks and FFTW


## Requirement

- NI-Daqmx
- OpenFrameworks 0.10.1
- ofxUI (addon)
- Visual Studio 2017
- (for FFT analyzer version) fftw-3.3.5


## Install

### Step 1
- If you don't have NI-Daqmx, install it from NI web page.
- If you don't have ofxUI, get it as follows:
```
cd (OF_HOME)\addons
git clone https://github.com/rezaali/ofxUI.git
```

### Step 2
Download NIDAQ_Monitor source codes to your openframeworks directory
```
cd (OF_HOME)\apps\myApps
git clone https://github.com/03araki/NIDAQ_Monitor.git
```

If you require FFT analyzer version, change branch as follows:
```
cd NIDAQ_Monitor
git checkout implement_fft_analyzer
```

Then, compile the project using Visual Studio 2017
