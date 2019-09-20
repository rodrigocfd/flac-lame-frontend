# FLAC and LAME front end

An easy GUI to work with [FLAC](https://en.wikipedia.org/wiki/FLAC) and [LAME](https://en.wikipedia.org/wiki/LAME) command line tools, allowing conversions from/to MP3, FLAC and WAV audio formats.

* Download [FLAC](https://ftp.osuosl.org/pub/xiph/releases/flac/)
* Download [LAME](http://www.rarewares.org/mp3-lame-bundle.php)

Once dowloaded, write the paths in `flac-lame-frontend.ini` file.

![Screenshot](screenshot-75.png)

## WinLamb library

This project uses [WinLamb](https://github.com/rodrigocfd/winlamb) library in a [submodule](http://blog.joncairns.com/2011/10/how-to-use-git-submodules).

After cloning the repository, pull the submodule files by running:

    git submodule init
    git submodule update

### Showcase

The project showcases the following WinLamb features:

* main and modal dialogs;
* parallel multi-threaded background operations;
* listview control;
* combobox control;
* group of radio controls;
* progress indicator on taskbar;
* read from INI files;
* enumerate files in directories;
* execute external command-line programs.
