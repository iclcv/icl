# Changelog

## 10.0.2

* added file globbing for Windows
* created tests for file globbing and dynmatrix
* enabled default testing for all CI plattforms
* updated CI targets to bionic
* limit OpenCV support to versions < 4
* updated libAV related code
* updated doc and extensions to work with recent sphinx versions
* fixed jQuery issues in documentation
* add clang-tidy modernizer to scripts

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
