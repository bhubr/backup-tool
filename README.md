# backup-tool

A tool for this: https://benoithubert.net/indexer-manuellement-ses-fichiers-sur-un-nas-synology/

## License

[GNU General Public License (Version 3)](http://www.fsf.org/licensing/licenses/gpl-3.0.html)

## Credits

* Alexander Peslyak, better known as Solar Designer <solar at openwall.com> for [md5 source code in C](http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5)
* [Michael Kerrisk](http://man7.org/mtk/index.html) for providing base code for inotify use (some of his code, found [here](http://man7.org/tlpi/code/index.html), is in folder `inotify/` as of 
first commits).
* mloydm for providing [this code on SO](http://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux)
* Graham Poulter for providing [this code](http://snipplr.com/view/4025/mp3-checksum-in-id3-tag/) that I translated into C
* Jerry Jeremiah for his comprehensive answer to [this SO question](http://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response)

## Todo

* Don't add if same md5 in same dir (in same volume, but since each dir id is different anyway...)
  * Detect moved files (in the future)
* First scan the whole tree so as to know how many files are to be processed
* Detect duplicates
* "touch" files that already existed
* URL-ENCODE AMPERSANDS (&) in file names!!
* Srv: Create a "session" when the client launches, finish it at the end (then check not created files)
* Srv: CHECK HOST FROM IP first then take volume from this host
* Restrict to file type...

## Done

* Generate full md5 anyway in addition to audio hash for mp3 files
* Change response from server: add success flag in all cases, put output file in "file" obj for example
