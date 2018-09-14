# Changelog

## 10.1.0

* recent versions of IPP/MKL are now supported
* changed travis CI structure to use docker images 
* added file globbing for Windows
* enabled default testing for all CI plattforms

## 10.0.1

* setup infrastructure for testing and coverage analysis
* Bugfix: building ICL with RST failed
* Bugfix: Improved computation of pseudoinverse camera Qi
* simplified create_view_ray
* Bugfix: Optris IR detection failed
* Bugfix: Prefere custom Bullet versions (with BULLET_ROOT); use system path afterwards

## 10.0.0

* moved ICL to github
* introduced CI generously provided by travis and appveyor
* packaging workflow for Ubuntu packages
* ICL now builds reliably on all three major desktop operating systems
    - supported and tested versions include Win 10, OSX (High) Sierra and Ubuntu LTS (trusty/xenial)