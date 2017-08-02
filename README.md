# FLAC and LAME front end

An easy GUI to work with [FLAC](https://ftp.osuosl.org/pub/xiph/releases/flac/) and [LAME](http://www.rarewares.org/mp3-lame-bundle.php) command line tools, allowing conversions from/to MP3, FLAC and WAV audio formats.

![Screenshot](screenshot-75.png)

## WinLamb libraries

This project uses [WinLamb](https://github.com/rodrigocfd/winlamb) and [Winlamb More](https://github.com/rodrigocfd/winlamb-more) libraries in [subtrees](http://bluedesk.blogspot.com.br/2017/06/trying-out-git-subtree.html). To add the subtrees, run:

```
git remote add winlamb --no-tags https://github.com/rodrigocfd/winlamb.git
git subtree add --prefix winlamb winlamb master --squash

git remote add winlamb-more --no-tags https://github.com/rodrigocfd/winlamb-more.git
git subtree add --prefix winlamb-more winlamb-more master --squash
```

Then, when you want to pull the code from the external library repos, run:

```
git subtree pull --prefix winlamb winlamb master --squash
git subtree pull --prefix winlamb-more winlamb-more master --squash
```
